#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Cube.h"
#include "CabbageHardware.h"
#include "Pipeline/ComputePipeline.h"
#include "Pipeline/RasterizerPipeline.h"
#include <Hardware/GlobalContext.h>

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

            HardwareBuffer postionBuffer = HardwareBuffer(pos, BufferUsage::VertexBuffer);
            HardwareBuffer normalBuffer = HardwareBuffer(normal, BufferUsage::VertexBuffer);
            HardwareBuffer uvBuffer = HardwareBuffer(textureUV, BufferUsage::VertexBuffer);
            HardwareBuffer colorBuffer = HardwareBuffer(color, BufferUsage::VertexBuffer);

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

                // 开始执行
                // 注意这里的执行顺序是从上到下，也就是先光栅化后计算
                // 每一个管线前面的<<操作符都会把管线内记录的命令添加到executor中
                // 最后通过commit提交执行
                // executor会自动处理管线间的资源同步问题
                // 比如这里的finalOutputImage在光栅化管线中作为输出，在计算管线中作为输入
                // executor会自动在两条管线间插入合适的内存屏障，确保数据正确性
                // 当然你也可以手动添加内存屏障，覆盖executor的自动处理
                // 具体用法请参考HardwareExecutor和CommandRecord的实现
                // 注意这里的图像格式和用法必须匹配，否则会导致未定义行为
                // 比如finalOutputImage作为光栅化的输出，必须包含VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                // 作为计算的输入，必须包含VK_IMAGE_USAGE_STORAGE_BIT
                // 这些都会在创建HardwareImage时自动处理
                // 如果你需要更复杂的用法，可以参考HardwareImage的实现自行扩展
                // 比如添加更多的图像用法标志，或者支持多层图像等
                // 总之，HardwareExecutor和CommandRecord的设计目标是简化多管线协作的复杂性
                // 让用户专注于管线本身的逻辑，而不必过多关心底层的同步和资源管理细节
                executor
                    << rasterizer(1920, 1080)
                    << computer(1920 / 8, 1080 / 8, 1)
                    << executor.commit();

                displayManager = finalOutputImage;

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
