#include <ktm/ktm.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>
// #include <semaphore>

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

//#define TEST_HELICON

// uniform buffer中
struct GlobalUniformParam
{
    float globalTime;
    float globalScale;
    uint32_t frameCount;
    uint32_t padding; // 为了对齐
};

struct GlobalUniformParamProxy
{
    EmbeddedShader::Float globalTime;
    EmbeddedShader::Float globalScale;
    EmbeddedShader::Uint frameCount;
    EmbeddedShader::Uint padding; // 为了对齐
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

struct RasterizerStorageBufferObjectProxy
{
    EmbeddedShader::Uint textureIndex;
    EmbeddedShader::Float4x4 model = ktm::rotate3d_axis(ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    EmbeddedShader::Float4x4 view = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f), ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    EmbeddedShader::Float4x4 proj = ktm::perspective_lh(ktm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 10.0f);
    EmbeddedShader::Float3 viewPos = ktm::fvec3(2.0f, 2.0f, 2.0f);
    EmbeddedShader::Float3 lightColor = ktm::fvec3(10.0f, 10.0f, 10.0f);
    EmbeddedShader::Float3 lightPos = ktm::fvec3(1.0f, 1.0f, 1.0f);
};

struct RasterizerFragmentInputProxy
{
    EmbeddedShader::Float3 fragPos;
    EmbeddedShader::Float3 fragNormal;
    EmbeddedShader::Float2 fragTexCoord;
    EmbeddedShader::Float3 fragColor;
};

struct ComputeStorageBufferObject
{
    uint32_t imageID;
};

int main()
{
    // Corona::Kernel::CoronaLogger::get_logger()->set_log_level(quill::LogLevel::TraceL3);
    //  setupSignalHandlers();

    // 运行压缩纹理测试（可选）
    // testCompressedTextures();

    CFW_LOG_INFO("Starting main application...");

    if (glfwInit() < 0)
    {
        return -1;
    }

    CFW_LOG_INFO("Main thread started...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // GLFWwindow* window = glfwCreateWindow(800, 600, "Window Title", NULL, NULL);
    // if (!window) {
    //     glfwTerminate();
    //     return -1;
    // }
    // const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // int windowedX, windowedY, windowedWidth, windowedHeight;
    // glfwGetWindowPos(window, &windowedX, &windowedY);
    // glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

    // auto lastF11Toggle = std::chrono::steady_clock::now();
    // constexpr auto f11Cooldown = std::chrono::milliseconds(100);

    // while (!glfwWindowShouldClose(window)) {
    //     glfwPollEvents();
    //     if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
    //         auto now = std::chrono::steady_clock::now();
    //         if (now - lastF11Toggle > f11Cooldown) {
    //             lastF11Toggle = now;
    //             static bool isFullscreen = false;
    //             if (isFullscreen) {
    //                 glfwSetWindowMonitor(window, NULL, windowedX, windowedY, windowedWidth, windowedHeight, 0);
    //             } else {
    //                 glfwGetWindowPos(window, &windowedX, &windowedY);
    //                 glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
    //                 glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
    //             }
    //             isFullscreen = !isFullscreen;
    //         }
    //     }
    //     glfwSwapBuffers(window);
    // }

    constexpr std::size_t WINDOW_COUNT = 1;
    std::vector<GLFWwindow *> windows(WINDOW_COUNT);
    for (size_t i = 0; i < windows.size(); i++)
    {
        windows[i] = glfwCreateWindow(1920, 1080, "Cabbage Engine ", nullptr, nullptr);
    }

    // 同步原语：每个窗口一套信号量，控制流水线 Mesh -> Render -> Display -> Mesh
    /*std::vector<std::unique_ptr<std::binary_semaphore>> meshSemaphores;
    std::vector<std::unique_ptr<std::binary_semaphore>> renderSemaphores;
    std::vector<std::unique_ptr<std::binary_semaphore>> displaySemaphores;*/

    // for (size_t i = 0; i < WINDOW_COUNT; ++i) {
    //     // 初始状态：允许 Mesh 线程开始，Render 和 Display 等待
    //     meshSemaphores.push_back(std::make_unique<std::binary_semaphore>(1));
    //     renderSemaphores.push_back(std::make_unique<std::binary_semaphore>(0));
    //     displaySemaphores.push_back(std::make_unique<std::binary_semaphore>(0));
    // }

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
            CFW_LOG_ERROR("Failed to load texture, exiting...");
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
        std::vector<HardwareBuffer> globalUniformParamBuffers(windows.size());
        std::vector<HardwareBuffer> computeStorageBuffers(windows.size());

        std::atomic_bool running = true;

        auto meshThread = [&](uint32_t threadIndex) {
            CFW_LOG_INFO("Mesh thread {} started...", threadIndex);

            ComputeStorageBufferObject computeUniformData(windows.size());
            computeStorageBuffers[threadIndex] = HardwareBuffer(sizeof(ComputeStorageBufferObject), BufferUsage::StorageBuffer);

            GlobalUniformParam globalUniformParamData(windows.size());
            globalUniformParamBuffers[threadIndex] = HardwareBuffer(sizeof(GlobalUniformParam), BufferUsage::UniformBuffer);

            std::vector<ktm::fmat4x4> modelMat(1);
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

                computeUniformData.imageID = finalOutputImages[threadIndex].storeDescriptor();
                computeStorageBuffers[threadIndex].copyFromData(&computeUniformData, sizeof(computeUniformData));

                GlobalUniformParam updatedParam{};
                updatedParam.globalTime = currentTime;
                updatedParam.globalScale = 2.0f + sin(currentTime) * 2.0f; // 轻微缩放动画
                updatedParam.frameCount = static_cast<uint32_t>(frameCount);
                updatedParam.padding = 0;
                globalUniformParamBuffers[threadIndex].copyFromData(&updatedParam, sizeof(updatedParam));

                ++frameCount;

                // 通知渲染线程可以开始
                // renderSemaphores[threadIndex]->release();
            }
            // 退出时释放后续信号量，防止死锁
            // renderSemaphores[threadIndex]->release();

            CFW_LOG_INFO("Mesh thread {} ended.", threadIndex);
        };

        auto renderThread = [&](uint32_t threadIndex) {
            CFW_LOG_INFO("Render thread {} started...", threadIndex);

            RasterizerPipeline rasterizer(readStringFile(shaderPath + "/simpleVert.glsl"), readStringFile(shaderPath + "/simpleFrag.glsl"));

#ifdef TEST_HELICON
            using namespace EmbeddedShader;
            using namespace EmbeddedShader::Ast;
            using namespace ktm;

            Texture2D<fvec4> inputImageRGBA16;
            Aggregate<GlobalUniformParamProxy> globalParams;

            auto acesFilmicToneMapCurve = [&](Float3 x) 
            {
                Float a = 2.51f;
                Float b = 0.03f;
                Float c = 2.43f;
                Float d = 0.59f;
                Float e = 0.14f;

                return clamp((x * (a * x + b)) / (x * (c * x + d) + e), fvec3(0.0f), fvec3(1.0f));
            };

            auto acesFilmicToneMapInverse = [&](const Float3 &x) 
            {
                Float3 a = fvec3(-0.59f) * x + fvec3(0.03f);
                Float3 b = sqrt(fvec3(-1.0127f) * x * x + fvec3(1.3702f) * x + fvec3(0.0009));
                Float3 c = fvec3(2) * (fvec3(2.43f) * x - fvec3(2.51f));
                return ((a - b) / c);
            };

            auto compute = [&] 
            {
                Float4 color = inputImageRGBA16[dispatchThreadID()->xy()];
                Float effectFactor = 1.f;
                //Float effectFactor = sin(globalParams->globalTime * 2.0) * 0.5 + 0.5;
                Float3 adjustedColor = color->xyz() * (1.0f + effectFactor * 0.2f);
                inputImageRGBA16[dispatchThreadID()->xy()] = Float4(acesFilmicToneMapCurve(adjustedColor), 1.f);
            };

            Aggregate<RasterizerStorageBufferObjectProxy> storageBufferObjects;

            auto vert = [&](Float3 inPosition, Float3 inNormal,Float2 inTexCoord,Float3 inColor) {
                //Float4x4 scaledModel = Float4x4(Float4(storageBufferObjects->model[0]->xyz() * globalParams->globalScale, storageBufferObjects->model[0]->w),
                //Float4(storageBufferObjects->model[1]->xyz() * globalParams->globalScale, storageBufferObjects->model[1]->w),
                //Float4(storageBufferObjects->model[2]->xyz() * globalParams->globalScale, storageBufferObjects->model[2]->w),
                //storageBufferObjects->model[3]);

                position() = mul(mul(storageBufferObjects->proj,storageBufferObjects->view),
                    //mul scaledModel
                    Float4(inPosition, 1.f));

                Aggregate<RasterizerFragmentInputProxy> fragmentInput;
                //fragmentInput->fragPos = Float3(scaledModel * Float4(inPosition, 1.0));
                //fragmentInput->fragNormal = normalize(Float3x3(transpose(/*inverse*/ scaledModel)) * inNormal) ;
                fragmentInput->fragColor = inColor;
                fragmentInput->fragTexCoord = inTexCoord;
                return fragmentInput;
            };

            Texture2D<fvec4> textures;
            Sampler sampler;

            auto DistributionGGX = [&](Float3 N, Float3 H, Float roughness)
            {
                Float a = roughness * roughness;
                Float a2 = a * a;
                Float NdotH = max(dot(N, H), 0.0f);
                Float NdotH2 = NdotH * NdotH;

                Float nom = a2;
                Float denom = (NdotH2 * (a2 - 1.0) + 1.0);
                denom = 3.14159265359f * denom * denom;

                return nom / denom;
            };

            auto GeometrySchlickGGX = [&](Float NdotV, Float roughness)
            {
                Float r = (roughness + 1.0f);
                Float k = (r * r) / 8.0f;

                Float nom = NdotV;
                Float denom = NdotV * (1.0f - k) + k;

                return nom / denom;
            };

            auto GeometrySmith = [&](Float3 N, Float3 V, Float3 L, Float roughness)
            {
                Float NdotV = max(dot(N, V), 0.0f);
                Float NdotL = max(dot(N, L), 0.0f);
                Float ggx2 = GeometrySchlickGGX(NdotV, roughness);
                Float ggx1 = GeometrySchlickGGX(NdotL, roughness);

                return ggx1 * ggx2;
            };

            auto fresnelSchlick = [&](Float cosTheta, Float3 F0)
            {
                return F0 + (fvec3(1.0f) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
            };

            auto calculateColor = [&](Float3 WorldPos, Float3 Normal, Float3 albedo, Float metallic, Float roughness)
            {
                Float3 N = normalize(Normal);
                Float3 V = normalize(storageBufferObjects->viewPos - WorldPos);

                // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
                // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
                Float3 F0 = fvec3(0.04,0.04,0.04);
                F0 = mix(F0, albedo, metallic);

                // reflectance equation
                Float3 Lo = fvec3(0.f,0.f,0.f);
    //for(int i = 0; i < 4; ++i)
                {
                    // calculate per-light radiance
                    Float3 L = normalize(storageBufferObjects->lightPos - WorldPos);
                    Float3 H = normalize(V + L);
                    //float distance = length(lightPos - WorldPos);
                    Float attenuation = 1.0;
                    Float3 radiance = storageBufferObjects->lightColor * attenuation;

                    // Cook-Torrance BRDF
                    Float NDF = DistributionGGX(N, H, roughness);
                    Float G = GeometrySmith(N, V, L, roughness);
                    Float3 F = fresnelSchlick(clamp(dot(H, V), 0.0f, 1.0f), F0);

                    Float3 numerator = NDF * G * F;
                    Float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f; // + 0.0001 to prevent divide by zero
                    Float3 specular = numerator / denominator;

                    // kS is equal to Fresnel
                    Float3 kS = F;
                    // for energy conservation, the diffuse and specular light can't
                    // be above 1.0 (unless the surface emits light); to preserve this
                    // relationship the diffuse component (kD) should equal 1.0 - kS.
                    Float3 kD = fvec3(1.f,1.f,1.f) - kS;
                    // multiply kD by the inverse metalness such that only non-metals
                    // have diffuse lighting, or a linear blend if partly metal (pure metals
                    // have no diffuse light).
                    kD *= 1.0f - metallic;

                    // scale light by NdotL
                    Float NdotL = max(dot(N, L), 0.0f);

                    // add to outgoing radiance Lo
                    Lo += (kD * albedo / Float(3.14159265359f) + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
                }

                // ambient lighting (note that the next IBL tutorial will replace
                // this ambient lighting with environment lighting).
                Float3 ambient = fvec3(0.03,0.03,0.03) * albedo;

                return ambient + Lo;
            };

            auto frag = [&](Aggregate<RasterizerFragmentInputProxy> input) {
                Float4 color = Float4(textures.sample(sampler,input->fragTexCoord/*,0.0*/));
                Float3 albedo;
                $IF(color->w > 0.01f)
                {
                    albedo = color->xyz();
                }
                $ELSE
                {
                    albedo = input->fragColor;
                }
                Float4 outColor = Float4(calculateColor(input->fragPos, input->fragNormal, albedo, 0.5f, 0.5f),1.0f);
            };

            CompilerOption compilerOption = {};
            compilerOption.compileHLSL = false;
            compilerOption.compileDXIL = true;
            compilerOption.compileDXBC = true;
            compilerOption.compileGLSL = true;
            compilerOption.enableBindless = true;

            auto computePipeline = ComputePipelineObject::compile(compute, uvec3(8, 8, 1), compilerOption);
            auto computeShaderCode = computePipeline.compute->getShaderCode(ShaderLanguage::GLSL).shaderCode;
            std::string computeShaderCodeStr = std::get<std::string>(computeShaderCode);

            ComputePipeline computer(computeShaderCodeStr);

            // 新 API：直接传入 lambda，内部完成编译
            //ComputePipeline computer(compute, uvec3(8, 8, 1));

#else
            //ComputePipeline computer(readStringFile(shaderPath + "/simpleCompute.glsl"));
#endif

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load())
            {
                // 等待数据更新完成
                // renderSemaphores[threadIndex]->acquire();
                // if (!running.load()) break;
                HardwareBuffer vertexBuffer = HardwareBuffer(vertices, BufferUsage::VertexBuffer);
                HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);
                float currentTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

                for (size_t i = 0; i < rasterizerStorageBuffers[threadIndex].size(); i++)
                {
                    //rasterizer["pushConsts.storageBufferIndex"] = rasterizerStorageBuffers[threadIndex][i].storeDescriptor();
                    //rasterizer["pushConsts.uniformBufferIndex"] = globalUniformParamBuffers[threadIndex].storeDescriptor();

                    // rasterizer["inPosition"] = postionBuffer;
                    // rasterizer["inColor"] = colorBuffer;
                    // rasterizer["inTexCoord"] = uvBuffer;
                    // rasterizer["inNormal"] = normalBuffer;
                    //rasterizer["outColor"] = finalOutputImages[threadIndex];
                    rasterizer.setResource("outColor", finalOutputImages[threadIndex]);

                    rasterizer.record(indexBuffer, vertexBuffer);
                }

                //computer["pushConsts.storageBufferIndex"] = computeStorageBuffers[threadIndex].storeDescriptor();

                executors[threadIndex] << rasterizer(1920, 1080)
                                       //<< computer(1920 / 8, 1080 / 8, 1)
                                       << executors[threadIndex].commit();
                ++frameCount;

                // 通知显示线程可以开始
                // displaySemaphores[threadIndex]->release();
            }
            // displaySemaphores[threadIndex]->release();

            CFW_LOG_INFO("Render thread {} ended.", threadIndex);
        };

        auto displayThread = [&](uint32_t threadIndex) {
            CFW_LOG_INFO("Display thread {} started...", threadIndex);

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

            CFW_LOG_INFO("Display thread {} ended.", threadIndex);
        };

        std::vector<std::thread> meshThreads;
        std::vector<std::thread> renderThreads;
        std::vector<std::thread> displayThreads;

        for (size_t i = 0; i < windows.size(); i++)
        {
            meshThreads.emplace_back(meshThread, i);
            renderThreads.emplace_back(renderThread, i);
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
                    // 释放所有信号量以唤醒等待的线程
                    /*for (size_t j = 0; j < WINDOW_COUNT; ++j)
                    {
                        meshSemaphores[j]->release();
                        renderSemaphores[j]->release();
                        displaySemaphores[j]->release();
                    }*/
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