#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>

#include <ktm/ktm.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "CabbageHardware.h"

#include "CubeData.h"
#include "Config.h"

struct RasterizerUniformBufferObject
{
    uint32_t textureIndex;
    ktm::fmat4x4 model = ktm::rotate3d_axis(ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    ktm::fmat4x4 view = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f), ktm::fvec3(0.0f, 0.0f, 0.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    ktm::fmat4x4 proj = ktm::perspective_lh(ktm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 10.0f);
    ktm::fvec3 viewPos = ktm::fvec3(2.0f, 2.0f, 2.0f);
    ktm::fvec3 lightColor = ktm::fvec3(10.0f, 10.0f, 10.0f);
    ktm::fvec3 lightPos = ktm::fvec3(1.0f, 1.0f, 1.0f);
};

struct ComputeUniformBufferObject
{
    uint32_t imageID;
};

int main()
{
    if (glfwInit() >= 0)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        std::vector<GLFWwindow *> windows(8);
        for (size_t i = 0; i < windows.size(); i++)
        {
            windows[i] = glfwCreateWindow(1920, 1080, "Cabbage Engine ", nullptr, nullptr);
        }

        std::atomic_bool running = true;

        auto oneWindowThread = [&](void *surface) {
            HardwareDisplayer displayManager = HardwareDisplayer(surface);

            RasterizerUniformBufferObject rasterizerUniformBufferObject;
            ComputeUniformBufferObject computeUniformData;

            HardwareBuffer postionBuffer = HardwareBuffer(positions, BufferUsage::VertexBuffer);
            HardwareBuffer normalBuffer = HardwareBuffer(normals, BufferUsage::VertexBuffer);
            HardwareBuffer uvBuffer = HardwareBuffer(uvs, BufferUsage::VertexBuffer);
            HardwareBuffer colorBuffer = HardwareBuffer(colors, BufferUsage::VertexBuffer);

            HardwareBuffer indexBuffer = HardwareBuffer(indices, BufferUsage::IndexBuffer);

            HardwareBuffer computeUniformBuffer = HardwareBuffer(sizeof(ComputeUniformBufferObject), BufferUsage::UniformBuffer);

            int width, height, channels;
            unsigned char *data = stbi_load(std::string(shaderPath + "/awesomeface.png").c_str(), &width, &height, &channels, 0);
            if (!data)
            {
                // if the image fails to load, maby image encryption.
                std::cerr << "stbi_load failed: " << stbi_failure_reason() << std::endl;
            }

            HardwareImage texture(width, height, ImageFormat::RGBA8_SRGB, ImageUsage::SampledImage, 1, data);

            HardwareImage finalOutputImage(1920, 1080, ImageFormat::RGBA16_FLOAT, ImageUsage::StorageImage);

            RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));

            ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));

            auto startTime = std::chrono::high_resolution_clock::now();

            double totalTimeMs = 0.0;
            int frameCount = 0;

            HardwareExecutor executor;

            std::vector<HardwareBuffer> rasterizerUniformBuffers;
            std::vector<ktm::fmat4x4> modelMat;
            for (size_t i = 0; i < 20; i++)
            {
                ktm::fmat4x4 tempModelMat = ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) * ktm::scale3d(ktm::fvec3(0.1, 0.1, 0.1)) * ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                HardwareBuffer tempRasterizerUniformBuffers = HardwareBuffer(sizeof(RasterizerUniformBufferObject), BufferUsage::UniformBuffer);

                modelMat.push_back(tempModelMat);
                rasterizerUniformBuffers.push_back(tempRasterizerUniformBuffers);
            }

            while (running.load())
            {
                auto start = std::chrono::high_resolution_clock::now();

                float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

                for (size_t i = 0; i < modelMat.size(); i++)
                {
                    rasterizerUniformBufferObject.textureIndex = texture.storeDescriptor();
                    rasterizerUniformBufferObject.model = modelMat[i] * ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                    rasterizerUniformBuffers[i].copyFromData(&rasterizerUniformBufferObject, sizeof(rasterizerUniformBufferObject));
                    rasterizer["pushConsts.uniformBufferIndex"] = rasterizerUniformBuffers[i].storeDescriptor();
                    rasterizer["inPosition"] = postionBuffer;
                    rasterizer["inColor"] = colorBuffer;
                    rasterizer["inTexCoord"] = uvBuffer;
                    rasterizer["inNormal"] = normalBuffer;
                    rasterizer["outColor"] = finalOutputImage;

                    executor << rasterizer.record(indexBuffer);
                }

                computeUniformData.imageID = finalOutputImage.storeDescriptor();
                computeUniformBuffer.copyFromData(&computeUniformData, sizeof(computeUniformData));
                computer["pushConsts.uniformBufferIndex"] = computeUniformBuffer.storeDescriptor();

                executor << rasterizer(1920, 1080)
                         << executor.commit();

                executor << computer(1920 / 8, 1080 / 8, 1)
                         << executor.commit();

                displayManager.wait(executor) << finalOutputImage;

                auto timeD = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - start);
                totalTimeMs += timeD.count();
                frameCount++;
                if (frameCount >= 100)
                {
                    std::cout << "Average time over " << frameCount << " frames: " << totalTimeMs / frameCount << " ms" << std::endl;
                    totalTimeMs = 0.0;
                    frameCount = 0;
                }
            }
        };

        for (size_t i = 0; i < windows.size(); i++)
        {
            std::thread(oneWindowThread, glfwGetWin32Window(windows[i])).detach();
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
            glfwDestroyWindow(windows[i]);
        }
        glfwTerminate();
    }

    return 0;
}
