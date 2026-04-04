#include "default_scenario.h"

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <ktm/ktm.h>

#include "Codegen/BuiltinVariate.h"
#include "Codegen/ControlFlows.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"
#include "cube_data.h"
#include "scenario_registry.h"

#include GLSL(vert.glsl)
#include GLSL(frag.glsl)

struct VertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
};

//struct HelloTriangleMeshPayload
//{
//    std::vector<std::vector<hello_triangle::SimpleVertex>> transformedPerObject;
//};

struct DefaultScenario::Impl
{
    explicit Impl(RuntimeConfig cfg) : config(std::move(cfg)) {}

    void ensure_edsl_pipeline(const HardwareImage &output_image)
    {
        // 懒初始化 EDSL 管线：只在首次渲染时创建一次。
        std::lock_guard<std::mutex> lock(edsl_mutex);
        if (edsl_rasterizer)
        {
            return;
        }

        using namespace EmbeddedShader;
        using namespace EmbeddedShader::Ast;

        edsl_output = output_image;

        auto vs_lambda = [&](Aggregate<VertexAttributeProxy> vertex) -> Float4 
        {
            position() = Float4(vertex->position, 1.0f);
            return Float4(vertex->color, 1.0f);
        };

        auto fs_lambda = [&](Float4 interpolated_color) -> Float4 
        {
            return interpolated_color;
        };

        edsl_rasterizer = std::make_unique<RasterizerPipeline<>>(vs_lambda, fs_lambda);
        edsl_rasterizer->bindOutputTargets(edsl_output);
        // 索引缓冲固定不变，创建一次即可复用。
        edsl_index_buffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    void ensure_glsl_pipeline(const HardwareImage &output_image)
    {
        // 懒初始化 GLSL 管线：只在首次渲染时创建一次。
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (glsl_rasterizer)
        {
            return;
        }

        glsl_rasterizer = std::make_unique<RasterizerPipeline<vert_glsl, frag_glsl>>();
        glsl_rasterizer->outColor = output_image;
        // 索引缓冲固定不变，创建一次即可复用。
        glslIndexBuffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;

    std::vector<SimpleVertex> simple_vertices;
    std::vector<ktm::fmat4x4> base_model_matrices;
    ktm::fmat4x4 vp_matrix{};
    Clock::time_point star_ttime{};
    bool initialized{false};

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<vert_glsl, frag_glsl>> glsl_rasterizer;
    std::unique_ptr<HardwareBuffer> glsl_index_buffer;
};

DefaultScenario::DefaultScenario(const RuntimeConfig &config) : impl_(std::make_unique<Impl>(config)){}

DefaultScenario::~DefaultScenario() = default;

std::string DefaultScenario::name() const
{
    return "default";
}

bool DefaultScenario::init(const RuntimeConfig &config,
                           const std::array<HardwareImage, 2> &outputs,
                           std::string &error_message)
{
    // v1 仅支持 cube 数据集，其他数据集在这里显式拒绝。
    if (config.data_set != "cube")
    {
        error_message = "Unsupported dataset for hello_triangle scenario: " + config.data_set;
        return false;
    }

    impl_->config = config;
    impl_->simple_vertices = hello_triangle::makeSimpleVerticesFromCubeData();
    impl_->base_model_matrices = hello_triangle::makeBaseModelMatrices(20);

    auto view_Mat = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f),
                                   ktm::fvec3(0.0f, 0.0f, 0.0f),
                                   ktm::fvec3(0.0f, 0.0f, 1.0f));

    auto aspect = static_cast<float>(config.window_Width) / static_cast<float>(config.window_Height);
    auto proj_Mat = ktm::perspective_lh(ktm::radians(45.0f), aspect, 0.1f, 10.0f);
    impl_->vp_Matrix = proj_Mat * view_Mat;
    impl_->start_Time = Clock::now();

    // 预热两条管线，避免首帧编译/创建开销抖动。
    impl_->ensure_Edsl_Pipeline(outputs[0]);
    impl_->ensure_Glsl_Pipeline(outputs[1]);
    impl_->initialized = true;
    return true;
}

std::shared_ptr<const void> DefaultScenario::mesh_tick(uint64_t frameId,
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

bool DefaultScenario::renderEDSLTick(const MeshFrame &meshFrame,
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

bool DefaultScenario::renderGLSLTick(const MeshFrame &meshFrame,
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

void DefaultScenario::displayTick(const RenderFrame &renderFrame)
{
    (void)renderFrame;
}

void DefaultScenario::shutdown()
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