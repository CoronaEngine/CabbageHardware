#include <ktm/ktm.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "CabbageHardware.h"
#include "Config.h"
#include "CubeData.h"
#include "SignalHandler.h"
#include "TextureTest.h"
#include "corona/kernel/core/i_logger.h"

struct RasterizerUniformBufferObject {
    uint32_t textureIndex;
    ktm::fmat4x4 model = ktm::rotate3d_axis(ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    ktm::fmat4x4 view = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f), ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    ktm::fmat4x4 proj = ktm::perspective_lh(ktm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 10.0f);
    ktm::fvec3 viewPos = ktm::fvec3(2.0f, 2.0f, 2.0f);
    ktm::fvec3 lightColor = ktm::fvec3(10.0f, 10.0f, 10.0f);
    ktm::fvec3 lightPos = ktm::fvec3(1.0f, 1.0f, 1.0f);
};

struct ComputeUniformBufferObject {
    uint32_t imageID;
};

int main() {
    Corona::Kernel::CoronaLogger::get_logger()->set_log_level(quill::LogLevel::TraceL3);
    setupSignalHandlers();

    // 运行压缩纹理测试（可选）
    // testCompressedTextures();

    CFW_LOG_INFO("Starting main application...");

    if (glfwInit() < 0) {
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
    std::vector<GLFWwindow*> windows(WINDOW_COUNT);
    for (size_t i = 0; i < windows.size(); i++) {
        windows[i] = glfwCreateWindow(1920, 1080, "Cabbage Engine ", nullptr, nullptr);
    }

    {
        std::vector<HardwareImage> finalOutputImages(windows.size());
        std::vector<HardwareExecutor> executors(windows.size());
        for (size_t i = 0; i < finalOutputImages.size(); i++) {
            HardwareImageCreateInfo createInfo;
            createInfo.width = 1920;
            createInfo.height = 1080;
            createInfo.format = ImageFormat::RGBA16_FLOAT;
            createInfo.usage = ImageUsage::StorageImage;
            createInfo.arrayLayers = 1;
            createInfo.mipLevels = 1;

            finalOutputImages[i] = HardwareImage(createInfo);
        }

        HardwareBuffer vertexBuffer = HardwareBuffer(vertices, BufferUsage::VertexBuffer);
        // HardwareBuffer normalBuffer = HardwareBuffer(normals, BufferUsage::VertexBuffer);
        // HardwareBuffer uvBuffer = HardwareBuffer(uvs, BufferUsage::VertexBuffer);
        // HardwareBuffer colorBuffer = HardwareBuffer(colors, BufferUsage::VertexBuffer);
        HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

        // 纹理加载 - 选择以下任一方式
        // 方式1: 加载普通纹理
        auto textureResult = loadTexture(shaderPath + "/awesomeface.png");

        // 方式2: 加载BC1压缩纹理
        // auto textureResult = loadCompressedTexture(shaderPath + "/awesomeface.png", true);

        // 方式3: 加载带有 mipmap 和 array layers 的纹理
        // auto textureResult = loadTextureWithMipmapAndLayers(shaderPath + "/awesomeface.png", 2, 5, 1, 0);

        if (!textureResult.success) {
            CFW_LOG_ERROR("Failed to load texture, exiting...");
            for (size_t i = 0; i < windows.size(); i++) {
                glfwDestroyWindow(windows[i]);
            }
            glfwTerminate();
            return -1;
        }

        uint32_t textureID = textureResult.descriptorID;
        HardwareImage& texture = textureResult.texture;

        std::vector<std::vector<HardwareBuffer>> rasterizerUniformBuffers(windows.size());
        std::vector<HardwareBuffer> computeUniformBuffers(windows.size());

        std::atomic_bool running = true;

        auto meshThread = [&](uint32_t threadIndex) {
            CFW_LOG_INFO("Mesh thread {} started...", threadIndex);
            ComputeUniformBufferObject computeUniformData(windows.size());
            computeUniformBuffers[threadIndex] = HardwareBuffer(sizeof(ComputeUniformBufferObject), BufferUsage::UniformBuffer);

            std::vector<ktm::fmat4x4> modelMat(20);
            std::vector<RasterizerUniformBufferObject> rasterizerUniformBufferObject(modelMat.size());
            for (size_t i = 0; i < modelMat.size(); i++) {
                modelMat[i] = (ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) * ktm::scale3d(ktm::fvec3(0.1, 0.1, 0.1)) * ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f)));
                rasterizerUniformBuffers[threadIndex].push_back(HardwareBuffer(sizeof(RasterizerUniformBufferObject), BufferUsage::UniformBuffer, &(modelMat[i])));
            }

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load()) {
                float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
                // CFW_LOG_INFO("Mesh thread {} frame {} at {:.3f}s", threadIndex, frameCount, time);

                for (size_t i = 0; i < rasterizerUniformBuffers[threadIndex].size(); i++) {
                    // rasterizerUniformBufferObject[i].textureIndex = texture[0][0].storeDescriptor();
                    rasterizerUniformBufferObject[i].textureIndex = textureID;
                    rasterizerUniformBufferObject[i].model = modelMat[i] * ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                    rasterizerUniformBuffers[threadIndex][i].copyFromData(&(rasterizerUniformBufferObject[i]), sizeof(rasterizerUniformBufferObject[i]));
                }

                computeUniformData.imageID = finalOutputImages[threadIndex].storeDescriptor();
                computeUniformBuffers[threadIndex].copyFromData(&computeUniformData, sizeof(computeUniformData));
                ++frameCount;
            }
            CFW_LOG_INFO("Mesh thread {} ended.", threadIndex);
        };

        auto renderThread = [&](uint32_t threadIndex) {
            CFW_LOG_INFO("Render thread {} started...", threadIndex);
            RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));
            ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load()) {
                float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
                // CFW_LOG_INFO("Render thread {} frame {} at {:.3f}s", threadIndex, frameCount, time);

                for (size_t i = 0; i < rasterizerUniformBuffers[threadIndex].size(); i++) {
                    rasterizer["pushConsts.uniformBufferIndex"] = rasterizerUniformBuffers[threadIndex][i].storeDescriptor();
                    // rasterizer["inPosition"] = postionBuffer;
                    // rasterizer["inColor"] = colorBuffer;
                    // rasterizer["inTexCoord"] = uvBuffer;
                    // rasterizer["inNormal"] = normalBuffer;
                    rasterizer["outColor"] = finalOutputImages[threadIndex];

                    executors[threadIndex] << rasterizer.record(indexBuffer, vertexBuffer);
                }

                computer["pushConsts.uniformBufferIndex"] = computeUniformBuffers[threadIndex].storeDescriptor();
                executors[threadIndex] << rasterizer(1920, 1080)
                                       << computer(1920 / 8, 1080 / 8, 1)
                                       << executors[threadIndex].commit();
                ++frameCount;
            }
            CFW_LOG_INFO("Render thread {} ended.", threadIndex);
        };

        auto displayThread = [&](uint32_t threadIndex) {
            CFW_LOG_INFO("Display thread {} started...", threadIndex);
            HardwareDisplayer displayManager = HardwareDisplayer(glfwGetWin32Window(windows[threadIndex]));

            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;

            while (running.load()) {
                float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
                // CFW_LOG_INFO("Display thread {} frame {} at {:.3f}s", threadIndex, frameCount, time);

                displayManager.wait(executors[threadIndex]) << finalOutputImages[threadIndex];
                ++frameCount;
            }
            CFW_LOG_INFO("Display thread {} ended.", threadIndex);
        };

        std::vector<std::thread> meshThreads;
        std::vector<std::thread> renderThreads;
        std::vector<std::thread> displayThreads;

        for (size_t i = 0; i < windows.size(); i++) {
            meshThreads.emplace_back(meshThread, i);
            renderThreads.emplace_back(renderThread, i);
            displayThreads.emplace_back(displayThread, i);
        }

        while (running.load()) {
            glfwPollEvents();
            for (size_t i = 0; i < windows.size(); i++) {
                if (glfwWindowShouldClose(windows[i])) {
                    running.store(false);
                    break;
                }
            }
        }

        for (size_t i = 0; i < windows.size(); i++) {
            if (meshThreads[i].joinable()) meshThreads[i].join();
            if (renderThreads[i].joinable()) renderThreads[i].join();
            if (displayThreads[i].joinable()) displayThreads[i].join();
        }
    }

    for (size_t i = 0; i < windows.size(); i++) {
        glfwDestroyWindow(windows[i]);
    }
    glfwTerminate();

    return 0;
}