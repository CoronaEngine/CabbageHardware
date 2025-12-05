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
    //Corona::Kernel::CoronaLogger::get_logger()->set_log_level(quill::LogLevel::TraceL3);
    setupSignalHandlers();

    // 运行压缩纹理测试（可选）
    // testCompressedTextures();
    
    CFW_LOG_INFO("Starting main application...");

    if (glfwInit() < 0) {
        return -1;
    }

    CFW_LOG_INFO("Main thread started...");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    constexpr std::size_t WINDOD_COUNT = 1;
    std::vector<GLFWwindow*> windows(WINDOD_COUNT);
    for (size_t i = 0; i < windows.size(); i++) {
        windows[i] = glfwCreateWindow(1920, 1080, "Cabbage Engine ", nullptr, nullptr);
    }

    std::vector<HardwareImage> finalOutputImages(windows.size());
    std::vector<HardwareExecutor> executors(windows.size());
    for (size_t i = 0; i < finalOutputImages.size(); i++) {
        HardwareImageCreateInfo createInfo;
        createInfo.width = 1920;
        createInfo.height = 1080;
        createInfo.format = ImageFormat::RGBA16_FLOAT;
        createInfo.usage = ImageUsage::StorageImage;
        createInfo.arrayLayers = 5;
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
    // auto textureResult = loadTexture(shaderPath + "/awesomeface.png");

    // 方式2: 加载BC1压缩纹理
    // auto textureResult = loadCompressedTexture(shaderPath + "/awesomeface.png", true);

    // 方式3: 加载带有 mipmap 和 array layers 的纹理
    auto textureResult = loadTextureWithMipmapAndLayers(shaderPath + "/awesomeface.png", 2, 5, 1, 0);

    if (!textureResult.success) {
        CFW_LOG_ERROR("Failed to load texture, exiting...");
        glfwTerminate();
        return -1;
    }

    uint32_t textureID = textureResult.descriptorID;
    HardwareImage& texture = textureResult.texture;

    std::vector<std::vector<HardwareBuffer>> rasterizerUniformBuffers(windows.size());
    std::vector<HardwareBuffer> computeUniformBuffers(windows.size());

    std::atomic_bool running = true;

    auto meshThread = [&](uint32_t threadIndex) {
        CFW_LOG_INFO("Mesh thread started...");
        ComputeUniformBufferObject computeUniformData(windows.size());
        computeUniformBuffers[threadIndex] = HardwareBuffer(sizeof(ComputeUniformBufferObject), BufferUsage::UniformBuffer);

        std::vector<ktm::fmat4x4> modelMat(20);
        std::vector<RasterizerUniformBufferObject> rasterizerUniformBufferObject(modelMat.size());
        for (size_t i = 0; i < modelMat.size(); i++) {
            modelMat[i] = (ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) * ktm::scale3d(ktm::fvec3(0.1, 0.1, 0.1)) * ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f)));
            rasterizerUniformBuffers[threadIndex].push_back(HardwareBuffer(sizeof(RasterizerUniformBufferObject), BufferUsage::UniformBuffer, &(modelMat[i])));
            }

        auto startTime = std::chrono::high_resolution_clock::now();

        while (running.load()) {
            float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

            for (size_t i = 0; i < rasterizerUniformBuffers[threadIndex].size(); i++) {
                    //rasterizerUniformBufferObject[i].textureIndex = texture[0][0].storeDescriptor();
                    rasterizerUniformBufferObject[i].textureIndex = textureID;
                    rasterizerUniformBufferObject[i].model = modelMat[i] * ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                    rasterizerUniformBuffers[threadIndex][i].copyFromData(&(rasterizerUniformBufferObject[i]), sizeof(rasterizerUniformBufferObject[i]));
                }

                computeUniformData.imageID = finalOutputImages[threadIndex].storeDescriptor();
                computeUniformBuffers[threadIndex].copyFromData(&computeUniformData, sizeof(computeUniformData));
        }
    };

    auto renderThread = [&](uint32_t threadIndex) {
        CFW_LOG_INFO("Render thread started...");
        RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));
        ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));
        while (running.load()) {
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
                                       << executors[threadIndex].submit();
        }
    };

    auto displayThread = [&](uint32_t threadIndex) {
        CFW_LOG_INFO("Display thread started...");
        HardwareDisplayer displayManager = HardwareDisplayer(glfwGetWin32Window(windows[threadIndex]));
        while (running.load()) {
            displayManager.wait(executors[threadIndex]) << finalOutputImages[threadIndex];
        }
    };

    for (size_t i = 0; i < windows.size(); i++) {
        std::thread(meshThread, i).detach();
        std::thread(renderThread, i).detach();
        std::thread(displayThread, i).detach();
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
        glfwDestroyWindow(windows[i]);
    }
    glfwTerminate();

    return 0;
}