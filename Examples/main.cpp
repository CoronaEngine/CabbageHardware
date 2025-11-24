#include <ktm/ktm.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_dxt.h>

#include "CabbageHardware.h"
#include "Config.h"
#include "CubeData.h"

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
    if (glfwInit() >= 0) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        std::vector<GLFWwindow*> windows(1);
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
            createInfo.arrayLayers = 1;
            createInfo.mipLevels = 1;

            finalOutputImages[i] = HardwareImage(createInfo);
        }

        HardwareBuffer postionBuffer = HardwareBuffer(positions, BufferUsage::VertexBuffer);
        HardwareBuffer normalBuffer = HardwareBuffer(normals, BufferUsage::VertexBuffer);
        HardwareBuffer uvBuffer = HardwareBuffer(uvs, BufferUsage::VertexBuffer);
        HardwareBuffer colorBuffer = HardwareBuffer(colors, BufferUsage::VertexBuffer);

        HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

        int width, height, channels;
        unsigned char* data = stbi_load(std::string(shaderPath + "/awesomeface.png").c_str(), &width, &height, &channels, 0);
        if (!data) {
            std::cerr << "stbi_load failed: " << stbi_failure_reason() << std::endl;
        }

        HardwareImageCreateInfo textureCreateInfo;
        textureCreateInfo.width = width;
        textureCreateInfo.height = height;
        textureCreateInfo.format = ImageFormat::RGBA8_SRGB;
        textureCreateInfo.usage = ImageUsage::SampledImage;
        textureCreateInfo.arrayLayers = 1;
        textureCreateInfo.mipLevels = 1;
        textureCreateInfo.initialData = data;

        HardwareImage texture(textureCreateInfo);

        std::atomic_bool running = true;

        auto renderThread = [&](uint32_t threadIndex) {
            RasterizerUniformBufferObject rasterizerUniformBufferObject;
            ComputeUniformBufferObject computeUniformData;

            std::vector<HardwareBuffer> rasterizerUniformBuffers;
            std::vector<ktm::fmat4x4> modelMat;
            for (size_t i = 0; i < 20; i++) {
                ktm::fmat4x4 tempModelMat = ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) * ktm::scale3d(ktm::fvec3(0.1, 0.1, 0.1)) * ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                HardwareBuffer tempRasterizerUniformBuffers = HardwareBuffer(sizeof(RasterizerUniformBufferObject), BufferUsage::UniformBuffer);

                modelMat.push_back(tempModelMat);
                rasterizerUniformBuffers.push_back(tempRasterizerUniformBuffers);
            }

            HardwareBuffer computeUniformBuffer = HardwareBuffer(sizeof(ComputeUniformBufferObject), BufferUsage::UniformBuffer);

            RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));

            ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));

            auto startTime = std::chrono::high_resolution_clock::now();

            while (running.load()) {
                float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

                for (size_t i = 0; i < modelMat.size(); i++) {
                    rasterizerUniformBufferObject.textureIndex = texture.storeDescriptor();
                    rasterizerUniformBufferObject.model = modelMat[i] * ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                    rasterizerUniformBuffers[i].copyFromData(&rasterizerUniformBufferObject, sizeof(rasterizerUniformBufferObject));
                    rasterizer["pushConsts.uniformBufferIndex"] = rasterizerUniformBuffers[i].storeDescriptor();
                    rasterizer["inPosition"] = postionBuffer;
                    rasterizer["inColor"] = colorBuffer;
                    rasterizer["inTexCoord"] = uvBuffer;
                    rasterizer["inNormal"] = normalBuffer;
                    rasterizer["outColor"] = finalOutputImages[threadIndex];

                    executors[threadIndex] << rasterizer.record(indexBuffer);
                }

                computeUniformData.imageID = finalOutputImages[threadIndex].storeDescriptor();
                computeUniformBuffer.copyFromData(&computeUniformData, sizeof(computeUniformData));
                computer["pushConsts.uniformBufferIndex"] = computeUniformBuffer.storeDescriptor();

                executors[threadIndex] << rasterizer(1920, 1080)
                                       << computer(1920 / 8, 1080 / 8, 1)
                                       << executors[threadIndex].commit();
            }
        };

        auto displayThread = [&](uint32_t threadIndex) {
            HardwareDisplayer displayManager = HardwareDisplayer(glfwGetWin32Window(windows[threadIndex]));
            while (running.load()) {
                //displayManager.wait(executors[threadIndex]) << finalOutputImages[threadIndex];
                displayManager << finalOutputImages[threadIndex];
            }
        };

        for (size_t i = 0; i < windows.size(); i++) {
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
    }

    return 0;
}
