#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <ktm/ktm.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "CabbageHardware.h"
#include "Pipeline/ComputePipeline.h"
#include "Pipeline/RasterizerPipeline.h"
#include <Hardware/GlobalContext.h>

#include "corona/kernel/utils/storage.h"

std::string readStringFile(const std::string_view file_path)
{
    std::ifstream file(file_path.data());
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open the file.");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();
    return buffer.str();
}

std::string shaderPath = [] {
    std::string resultPath = "";
    std::string runtimePath = std::filesystem::current_path().string();
    // std::replace(runtimePath.begin(), runtimePath.end(), '\\', '/');
    std::regex pattern(R"((.*)CabbageHardware\b)");
    std::smatch matches;
    if (std::regex_search(runtimePath, matches, pattern))
    {
        if (matches.size() > 1)
        {
            resultPath = matches[1].str() + "CabbageHardware";
        }
        else
        {
            throw std::runtime_error("Failed to resolve source path.");
        }
    }
    std::replace(resultPath.begin(), resultPath.end(), '\\', '/');
    return resultPath + "/Examples";
}();

std::vector<float> pos = {
    -0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,

    -0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,

    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,

    0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,

    -0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    0.5f,
    -0.5f,
    -0.5f,
    -0.5f,

    -0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    0.5f,
    -0.5f,
    0.5f,
    -0.5f,
};

std::vector<float> normal = {
    // Back face (Z-negative)
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, // Triangle 1
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, // Triangle 2

    // Front face (Z-positive)
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Triangle 1
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Triangle 2

    // Left face (X-negative)
    -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Triangle 1
    -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Triangle 2

    // Right face (X-positive)
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Triangle 1
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Triangle 2

    // Bottom face (Y-negative)
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Triangle 1
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Triangle 2

    // Top face (Y-positive)
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Triangle 1
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f  // Triangle 2
};

std::vector<float> textureUV = {
    // Back face
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,

    // Front face
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,

    // Left face
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,

    // Right face
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,

    // Bottom face
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,

    // Top face
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
};

std::vector<float> color = {
    // Back face (Red)
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    // Front face (Green)
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    // Left face (Blue)
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    // Right face (Yellow)
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    // Bottom face (Cyan)
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    // Top face (Magenta)
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
};

std::vector<uint32_t> indices =
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

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
    Corona::Kernel::Utils::Storage<RasterizerUniformBufferObject> testStronge;

    //test.

    if (glfwInit() >= 0)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        std::vector<GLFWwindow *> windows(1);
        for (size_t i = 0; i < windows.size(); i++)
        {
            windows[i] = glfwCreateWindow(1920, 1080, "Cabbage Engine ", nullptr, nullptr);
        }

        std::atomic_bool running = true;

        auto oneWindowThread = [&](void *surface) {
            HardwareDisplayer displayManager(surface);

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

            std::vector<HardwareBuffer> rasterizerUniformBuffers(10);
            std::vector<ktm::fmat4x4> modelMat(10);
            for (size_t i = 0; i < modelMat.size(); i++)
            {
                modelMat[i] = ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) * ktm::scale3d(ktm::fvec3(0.1, 0.1, 0.1)) * ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                rasterizerUniformBuffers[i] = HardwareBuffer(sizeof(RasterizerUniformBufferObject), BufferUsage::UniformBuffer);
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
        }; // 退出线程函数，临时对象会被销毁

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
