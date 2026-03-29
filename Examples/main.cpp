#include <ktm/ktm.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "CabbageHardware.h"
#include "Config.h"
#include "CubeData.h"
#include "TextureTest.h"
#include "corona/kernel/core/i_logger.h"

#include "Codegen/BuiltinVariate.h"
#include "Codegen/ControlFlows.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"

// 通过 CMake helicon_compile_shaders 自动编译生成的 shader 反射头文件
// eDSL 路径不再依赖 GLSL 反射头文件，render target 通过 bindRenderTarget 自动绑定
#include GLSL(vert.glsl)
#include GLSL(frag.glsl)
#include GLSL(compute.glsl)

// uniform buffer中
struct GlobalUniformParam
{
    float globalTime;
    float globalScale;
    uint32_t frameCount;
    uint32_t padding; // 为了对齐
};

// storage buffer
struct RasterizerStorageBufferObject
{
    uint32_t textureIndex;
    ktm::fmat4x4 model = ktm::rotate3d_axis(ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    ktm::fmat4x4 view = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f), ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    ktm::fmat4x4 proj = ktm::perspective_lh(ktm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 10.0f);
    ktm::fvec3 viewPos = ktm::fvec3(2.0f, 2.0f, 2.0f);
    ktm::fvec3 lightColor = ktm::fvec3(10.0f, 10.0f, 10.0f);
    ktm::fvec3 lightPos = ktm::fvec3(1.0f, 1.0f, 1.0f);
};

// Vertex attribute proxy: 统一定义顶点属性布局（字段顺序与 CPU 端 Vertex struct 一致）
// VS 使用 Aggregate<VertexAttributeProxy> 作为输入参数，
// 内部自动展开为独立的 LOCATION 属性，与传统 vertex input 等价
struct VertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 normal;
    EmbeddedShader::Float2 texCoord;
    EmbeddedShader::Float3 color;
};

int main()
{
    // Corona::Kernel::CoronaLogger::get_logger()->set_log_level(quill::LogLevel::TraceL3);
    //  setupSignalHandlers();

    // 运行压缩纹理测试（可选）
    // testCompressedTextures();

    //CFW_LOG_INFO("Starting main application...");

    if (glfwInit() < 0)
    {
        return -1;
    }

    //CFW_LOG_INFO("Main thread started...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // 每个 pair 创建 2 个窗口: 偶数索引 = EDSL, 奇数索引 = GLSL
    constexpr std::size_t DEMO_PAIR_COUNT = 1;
    constexpr std::size_t TOTAL_WINDOWS = DEMO_PAIR_COUNT * 2;
    std::vector<GLFWwindow *> windows(TOTAL_WINDOWS);
    for (size_t i = 0; i < windows.size(); i++)
    {
        std::string title = (i % 2 == 0) ? "Cabbage Engine [EDSL]" : "Cabbage Engine [GLSL]";
        windows[i] = glfwCreateWindow(1920, 1080, title.c_str(), nullptr, nullptr);
    }

    {
        std::vector<HardwareImage> finalOutputImages(windows.size());
        std::vector<HardwareExecutor> executors(windows.size());
        for (size_t i = 0; i < finalOutputImages.size(); i++)
        {
            HardwareImageCreateInfo createInfo;
            createInfo.width = 1920;
            createInfo.height = 1080;
            createInfo.format = ImageFormat::RGBA16_FLOAT;
            createInfo.usage = ImageUsage::StorageImage;
            createInfo.arrayLayers = 1;
            createInfo.mipLevels = 1;

            finalOutputImages[i] = HardwareImage(createInfo);
        }

        // HardwareBuffer normalBuffer = HardwareBuffer(normals, BufferUsage::VertexBuffer);
        // HardwareBuffer uvBuffer = HardwareBuffer(uvs, BufferUsage::VertexBuffer);
        // HardwareBuffer colorBuffer = HardwareBuffer(colors, BufferUsage::VertexBuffer);

        // 纹理加载 - 选择以下任一方式
        // 方式1: 加载普通纹理
        auto textureResult = loadTexture(shaderPath + "/awesomeface.png");

        // 方式2: 加载BC1压缩纹理
        // auto textureResult = loadCompressedTexture(shaderPath + "/awesomeface.png", true);

        // 方式3: 加载带有 mipmap 和 array layers 的纹理
        // auto textureResult = loadTextureWithMipmapAndLayers(shaderPath + "/awesomeface.png", 2, 5, 1, 0);

        if (!textureResult.success)
        {
            //CFW_LOG_ERROR("Failed to load texture, exiting...");
            for (size_t i = 0; i < windows.size(); i++)
            {
                glfwDestroyWindow(windows[i]);
            }
            glfwTerminate();
            return -1;
        }

        uint32_t textureID = textureResult.descriptorID;
        HardwareImage &texture = textureResult.texture;

        std::vector<std::vector<HardwareBuffer>> rasterizerStorageBuffers(windows.size());
        //std::vector<HardwareBuffer> computeStorageBuffers(windows.size());

        std::atomic_bool running = true;

        auto meshThread = [&](uint32_t threadIndex) {
            //CFW_LOG_INFO("Mesh thread {} started...", threadIndex);

            //ComputeStorageBufferObject computeUniformData(windows.size());
            //computeStorageBuffers[threadIndex] = HardwareBuffer(sizeof(ComputeStorageBufferObject), BufferUsage::StorageBuffer);

            std::vector<ktm::fmat4x4> modelMat(20);
            std::vector<RasterizerStorageBufferObject> rasterizerStorageBufferObjects(modelMat.size());
            for (size_t i = 0; i < modelMat.size(); i++)
            {
                modelMat[i] = (ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) * ktm::scale3d(ktm::fvec3(0.1, 0.1, 0.1)) * ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f)));
                rasterizerStorageBuffers[threadIndex].push_back(HardwareBuffer(sizeof(RasterizerStorageBufferObject), BufferUsage::StorageBuffer, &(modelMat[i])));
            }

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load())
            {
                // 等待上一帧显示完成（或初始状态）
                /*meshSemaphores[threadIndex]->acquire();
                if (!running.load()) break;*/

                float currentTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

                for (size_t i = 0; i < rasterizerStorageBuffers[threadIndex].size(); i++)
                {
                    // rasterizerUniformBufferObject[i].textureIndex = texture[0][0].storeDescriptor();
                    rasterizerStorageBufferObjects[i].textureIndex = textureID;
                    rasterizerStorageBufferObjects[i].model = modelMat[i] * ktm::rotate3d_axis(currentTime * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                    rasterizerStorageBuffers[threadIndex][i].copyFromData(&(rasterizerStorageBufferObjects[i]), sizeof(rasterizerStorageBufferObjects[i]));
                }

                //computeUniformData.imageID = finalOutputImages[threadIndex].storeDescriptor();
                //computeStorageBuffers[threadIndex].copyFromData(&computeUniformData, sizeof(computeUniformData));

                ++frameCount;

                // 通知渲染线程可以开始
                // renderSemaphores[threadIndex]->release();
            }
            // 退出时释放后续信号量，防止死锁
            // renderSemaphores[threadIndex]->release();

            //CFW_LOG_INFO("Mesh thread {} ended.", threadIndex);
        };

        // =====================================================================
        // renderThreadEDSL: EDSL 路径 — C++ lambda 定义 shader，自动绑定资源
        // =====================================================================
        auto renderThreadEDSL = [&](uint32_t threadIndex) {
            using namespace EmbeddedShader;
            using namespace EmbeddedShader::Ast;
            using namespace ktm;

            // Texture2D proxy 声明时直接绑定已有 HardwareImage
            Texture2D<fvec4> inputImageRGBA16 = finalOutputImages[threadIndex];

            // EDSL compute shader: ACES filmic tone mapping
            auto acesFilmicToneMapCurve = [&](Float3 x) 
            {
                Float a = 2.51f;
                Float b = 0.03f;
                Float c = 2.43f;
                Float d = 0.59f;
                Float e = 0.14f;

                return clamp((x * (a * x + b)) / (x * (c * x + d) + e), fvec3(0.0f), fvec3(1.0f));
            };

            auto compute = [&] 
            {
                Float4 color = inputImageRGBA16[dispatchThreadID()->xy()];
                inputImageRGBA16[dispatchThreadID()->xy()] = Float4(acesFilmicToneMapCurve(color->xyz()), 1.f);
            };

            // EDSL vertex shader: Aggregate<VertexAttributeProxy> 自动展开为顶点属性
            auto vsLambda = [&](Aggregate<VertexAttributeProxy> vertex) -> Float4
            {
                position() = Float4(vertex->position, 1.0f);
                return Float4(vertex->color, 1.0f);
            };

            // EDSL fragment shader: return Float4 → 自动映射到 SV_TARGET0
            auto fsLambda = [&](Float4 interpolatedColor) -> Float4
            {
                return Float4(0.18f, 0.72f, 0.35f, 1.0f);
            };

            // 从 lambda 创建管线，bindOutputTargets 自动绑定 render target
            RasterizerPipeline rasterizer(vsLambda, fsLambda);
            rasterizer.bindOutputTargets(inputImageRGBA16);

            // 从 lambda 创建 compute 管线，auto-bind 资源
            ComputePipeline computer(compute, uvec3(8, 8, 1));

            uint64_t frameCount = 0;

            while (running.load())
            {
                HardwareBuffer vertexBuffer = HardwareBuffer(vertices, BufferUsage::VertexBuffer);
                HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

                for (size_t i = 0; i < rasterizerStorageBuffers[threadIndex].size(); i++)
                {
                    // EDSL 路径: 无需手动绑定，render target 已通过 bindOutputTargets 注册
                    rasterizer.record(indexBuffer, vertexBuffer);
                }

                // auto-bind: proxy 的 boundResource_ 自动传递给 pipeline
                executors[threadIndex] << rasterizer(1920, 1080)
                                       << computer(1920 / 8, 1080 / 8, 1)
                                       << executors[threadIndex].commit();
                ++frameCount;
            }
        };

        // =====================================================================
        // renderThreadGLSL: 手写 GLSL 路径 — 预编译 SPIR-V + 手动 BindingKey 绑定
        // =====================================================================
        auto renderThreadGLSL = [&](uint32_t threadIndex) {
            // 从预编译 SPIR-V 二进制创建光栅化管线（vert.glsl + frag.glsl）
            // TypedRasterizerPipeline 通过模板参数自动构造，并暴露 binding block 为直接成员
            TypedRasterizerPipeline<vert_glsl, frag_glsl> rasterizer;

            // 直接成员访问绑定 render target: frag shader 的 outColor (stageOutputs)
            // stageOutputs 存入 renderTargets[]，不会被 record() 重置，只需绑定一次
            rasterizer.outColor = finalOutputImages[threadIndex];

            // 从预编译 SPIR-V 创建 compute 管线（TypedComputePipeline 自动构造）
            TypedComputePipeline<compute_glsl> computer;

            // compute shader 的 storage image 描述符索引在帧间不变，仅需获取一次
            uint32_t computeImageDescriptorID = finalOutputImages[threadIndex].storeDescriptor();
            computer.GlobalUniformParam.imageID = computeImageDescriptorID;

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load())
            {
                HardwareBuffer vertexBuffer = HardwareBuffer(vertices, BufferUsage::VertexBuffer);
                HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);
                float currentTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

                // UBO 字段在整帧内共享，不随 draw call 变化
                // record() 只快照 tempPushConstant（并重置），不重置 tempUBO
                // 因此 UBO 只需在 for 循环外设置一次
                rasterizer.GlobalUniformParam.globalTime = currentTime;
                rasterizer.GlobalUniformParam.globalScale = 2.0f + sin(currentTime) * 2.0f;
                rasterizer.GlobalUniformParam.frameCount = static_cast<uint32_t>(frameCount);
                rasterizer.GlobalUniformParam.padding = 0u;

                for (size_t i = 0; i < rasterizerStorageBuffers[threadIndex].size(); i++)
                {
                    // push constant 每次 record() 后被重置，必须在每次 record() 前重新设置
                    rasterizer.pushConsts.storageBufferIndex = rasterizerStorageBuffers[threadIndex][i].storeDescriptor();
                    rasterizer.record(indexBuffer, vertexBuffer);
                }

                executors[threadIndex] << rasterizer(1920, 1080)
                                       << computer(1920 / 8, 1080 / 8, 1)
                                       << executors[threadIndex].commit();
                ++frameCount;
            }
        };

        auto displayThread = [&](uint32_t threadIndex) {
            //CFW_LOG_INFO("Display thread {} started...", threadIndex);

            HardwareDisplayer displayManager = HardwareDisplayer(glfwGetWin32Window(windows[threadIndex]));

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load())
            {
                // 等待渲染提交完成
                // displaySemaphores[threadIndex]->acquire();
                // if (!running.load()) break;

                float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
                // CFW_LOG_INFO("Display thread {} frame {} at {:.3f}s", threadIndex, frameCount, time);

                displayManager.wait(executors[threadIndex]) << finalOutputImages[threadIndex];
                ++frameCount;

                // 通知 Mesh 线程开始下一帧
                // meshSemaphores[threadIndex]->release();
            }
            // meshSemaphores[threadIndex]->release();

            //CFW_LOG_INFO("Display thread {} ended.", threadIndex);
        };

        std::vector<std::thread> meshThreads;
        std::vector<std::thread> renderThreads;
        std::vector<std::thread> displayThreads;

        for (size_t i = 0; i < windows.size(); i++)
        {
            meshThreads.emplace_back(meshThread, i);
            if (i % 2 == 0)
                renderThreads.emplace_back(renderThreadEDSL, i);
            else
                renderThreads.emplace_back(renderThreadGLSL, i);
            displayThreads.emplace_back(displayThread, i);
        }

        while (running.load())
        {
            glfwPollEvents();
            for (size_t i = 0; i < windows.size(); i++)
            {
                if (glfwWindowShouldClose(windows[i]))
                {
                    running.store(false);
                    break;
                }
            }
        }

        for (size_t i = 0; i < windows.size(); i++)
        {
            if (meshThreads[i].joinable())
                meshThreads[i].join();
            if (renderThreads[i].joinable())
                renderThreads[i].join();
            if (displayThreads[i].joinable())
                displayThreads[i].join();
        }
    }

    for (size_t i = 0; i < windows.size(); i++)
    {
        glfwDestroyWindow(windows[i]);
    }

    glfwTerminate();

    return 0;
}
