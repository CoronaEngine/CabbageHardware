#include "HelloTriangleScenario.h"

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <ktm/ktm.h>

#include "CubeData.h"
#include "Codegen/BuiltinVariate.h"
#include "Codegen/ControlFlows.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"
#include "hello_triangle/CubeData.h"
#include "ScenarioRegistry.h"

#include GLSL(vert.glsl)
#include GLSL(frag.glsl)

namespace multishader
{
namespace
{
struct VertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
};

struct HelloTriangleMeshPayload
{
    std::vector<std::vector<hello_triangle::SimpleVertex>> transformedPerObject;
};
} // 匿名命名空间

struct HelloTriangleScenario::Impl
{
    explicit Impl(RuntimeConfig cfg)
        : config(std::move(cfg))
    {
    }

    void ensureEdslPipeline(const HardwareImage &outputImage)
    {
        // 懒初始化 EDSL 管线：只在首次渲染时创建一次。
        std::lock_guard<std::mutex> lock(edslMutex);
        if (edslRasterizer)
        {
            return;
        }

        using namespace EmbeddedShader;
        using namespace EmbeddedShader::Ast;

        edslOutput = outputImage;

        auto vsLambda = [&](Aggregate<VertexAttributeProxy> vertex) -> Float4 {
            position() = Float4(vertex->position, 1.0f);
            return Float4(vertex->color, 1.0f);
        };

        auto fsLambda = [&](Float4 interpolatedColor) -> Float4 {
            return interpolatedColor;
        };

        edslRasterizer = std::make_unique<RasterizerPipeline<>>(vsLambda, fsLambda);
        edslRasterizer->bindOutputTargets(edslOutput);
        // 索引缓冲固定不变，创建一次即可复用。
        edslIndexBuffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    void ensureGlslPipeline(const HardwareImage &outputImage)
    {
        // 懒初始化 GLSL 管线：只在首次渲染时创建一次。
        std::lock_guard<std::mutex> lock(glslMutex);
        if (glslRasterizer)
        {
            return;
        }

        glslRasterizer = std::make_unique<RasterizerPipeline<vert_glsl, frag_glsl>>();
        glslRasterizer->outColor = outputImage;
        // 索引缓冲固定不变，创建一次即可复用。
        glslIndexBuffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;

    std::vector<hello_triangle::SimpleVertex> simpleVertices;
    std::vector<ktm::fmat4x4> baseModelMatrices;
    ktm::fmat4x4 vpMatrix{};
    Clock::time_point startTime{};
    bool initialized{false};

    std::mutex edslMutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edslOutput;
    std::unique_ptr<RasterizerPipeline<>> edslRasterizer;
    std::unique_ptr<HardwareBuffer> edslIndexBuffer;

    std::mutex glslMutex;
    std::unique_ptr<RasterizerPipeline<vert_glsl, frag_glsl>> glslRasterizer;
    std::unique_ptr<HardwareBuffer> glslIndexBuffer;
};

HelloTriangleScenario::HelloTriangleScenario(const RuntimeConfig &config)
    : impl_(std::make_unique<Impl>(config))
{
}

HelloTriangleScenario::~HelloTriangleScenario() = default;

std::string HelloTriangleScenario::name() const
{
    return "hello_triangle";
}

bool HelloTriangleScenario::init(const RuntimeConfig &config,
                                 const std::array<HardwareImage, 2> &outputs,
                                 std::string &errorMessage)
{
    // v1 仅支持 cube 数据集，其他数据集在这里显式拒绝。
    if (config.dataset != "cube")
    {
        errorMessage = "Unsupported dataset for hello_triangle scenario: " + config.dataset;
        return false;
    }

    impl_->config = config;
    impl_->simpleVertices = hello_triangle::makeSimpleVerticesFromCubeData();
    impl_->baseModelMatrices = hello_triangle::makeBaseModelMatrices(20);

    auto viewMat = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f),
                                   ktm::fvec3(0.0f, 0.0f, 0.0f),
                                   ktm::fvec3(0.0f, 0.0f, 1.0f));
    auto aspect = static_cast<float>(config.windowWidth) / static_cast<float>(config.windowHeight);
    auto projMat = ktm::perspective_lh(ktm::radians(45.0f), aspect, 0.1f, 10.0f);
    impl_->vpMatrix = projMat * viewMat;
    impl_->startTime = Clock::now();

    // 预热两条管线，避免首帧编译/创建开销抖动。
    impl_->ensureEdslPipeline(outputs[0]);
    impl_->ensureGlslPipeline(outputs[1]);
    impl_->initialized = true;
    return true;
}

std::shared_ptr<const void> HelloTriangleScenario::meshTick(uint64_t frameId,
                                                            Clock::time_point now,
                                                            std::string &errorMessage)
{
    (void)frameId;
    if (!impl_->initialized)
    {
        errorMessage = "Scenario has not been initialized before meshTick.";
        return nullptr;
    }

    // 构建只读共享载荷，供 EDSL/GLSL 两个 render 线程并行消费。
    auto payload = std::make_shared<HelloTriangleMeshPayload>();
    payload->transformedPerObject.reserve(impl_->baseModelMatrices.size());

    float currentTime = std::chrono::duration<float>(now - impl_->startTime).count();
    for (const auto &base : impl_->baseModelMatrices)
    {
        // 每个物体都在 CPU 侧完成 MVP 与透视除法，渲染线程仅做提交。
        auto animatedModel = hello_triangle::makeAnimatedModelMatrix(base, currentTime);
        payload->transformedPerObject.emplace_back(
            hello_triangle::transformVerticesForObject(impl_->simpleVertices, impl_->vpMatrix * animatedModel));
    }

    return payload;
}

bool HelloTriangleScenario::renderEDSLTick(const MeshFrame &meshFrame,
                                           HardwareExecutor &executor,
                                           const HardwareImage &outputImage,
                                           std::string &errorMessage)
{
    if (!meshFrame.payload)
    {
        errorMessage = "renderEDSLTick received an empty mesh payload.";
        return false;
    }
    if (!impl_->edslRasterizer || !impl_->edslIndexBuffer)
    {
        impl_->ensureEdslPipeline(outputImage);
    }

    // mesh 线程和 render 线程遵循同一 payload 契约，此处做静态类型恢复。
    auto payload = std::static_pointer_cast<const HelloTriangleMeshPayload>(meshFrame.payload);
    for (const auto &transformedVertices : payload->transformedPerObject)
    {
        // 顶点数据按物体粒度录制 draw，便于后续替换不同 shader/data 路径。
        HardwareBuffer vertexBuffer(transformedVertices, BufferUsage::VertexBuffer);
        impl_->edslRasterizer->record(*impl_->edslIndexBuffer, vertexBuffer);
    }

    executor << (*impl_->edslRasterizer)(static_cast<uint16_t>(impl_->config.windowWidth),
                                         static_cast<uint16_t>(impl_->config.windowHeight))
             << executor.commit();
    return true;
}

bool HelloTriangleScenario::renderGLSLTick(const MeshFrame &meshFrame,
                                           HardwareExecutor &executor,
                                           const HardwareImage &outputImage,
                                           std::string &errorMessage)
{
    if (!meshFrame.payload)
    {
        errorMessage = "renderGLSLTick received an empty mesh payload.";
        return false;
    }
    if (!impl_->glslRasterizer || !impl_->glslIndexBuffer)
    {
        impl_->ensureGlslPipeline(outputImage);
    }

    // mesh 线程和 render 线程遵循同一 payload 契约，此处做静态类型恢复。
    auto payload = std::static_pointer_cast<const HelloTriangleMeshPayload>(meshFrame.payload);
    for (const auto &transformedVertices : payload->transformedPerObject)
    {
        HardwareBuffer vertexBuffer(transformedVertices, BufferUsage::VertexBuffer);
        impl_->glslRasterizer->record(*impl_->glslIndexBuffer, vertexBuffer);
    }

    executor << (*impl_->glslRasterizer)(static_cast<uint16_t>(impl_->config.windowWidth),
                                         static_cast<uint16_t>(impl_->config.windowHeight))
             << executor.commit();
    return true;
}

void HelloTriangleScenario::displayTick(const RenderFrame &renderFrame)
{
    (void)renderFrame;
}

void HelloTriangleScenario::shutdown()
{
    // 主动释放管线与缓冲，避免把回收压力留到进程退出阶段。
    impl_->edslIndexBuffer.reset();
    impl_->glslIndexBuffer.reset();
    impl_->edslRasterizer.reset();
    impl_->glslRasterizer.reset();
    impl_->initialized = false;
}

std::unique_ptr<ScenarioHooks> createHelloTriangleScenario(const RuntimeConfig &config)
{
    return std::make_unique<HelloTriangleScenario>(config);
}

void registerHelloTriangleScenario()
{
    static const bool registered = [] {
        registerScenario("default", createHelloTriangleScenario);
        registerScenario("hello_triangle", createHelloTriangleScenario);
        return true;
    }();
    (void)registered;
}
} // multishader 命名空间
