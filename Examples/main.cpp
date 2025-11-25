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

#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>

#include "CabbageHardware.h"
#include "Config.h"
#include "CubeData.h"
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

// TODO: 测试mipmap生成
// BC1压缩辅助函数
std::vector<uint8_t> compressToBC1(const unsigned char* data, int width, int height, int channels) {
    // BC1每4x4像素块占8字节
    uint32_t blockCountX = (width + 3) / 4;
    uint32_t blockCountY = (height + 3) / 4;
    std::vector<uint8_t> compressedData(blockCountX * blockCountY * 8);

    for (int by = 0; by < blockCountY; ++by) {
        for (int bx = 0; bx < blockCountX; ++bx) {
            uint8_t block[64];  // 4x4像素块，RGBA格式

            // 提取4x4块
            for (int py = 0; py < 4; ++py) {
                for (int px = 0; px < 4; ++px) {
                    int x = bx * 4 + px;
                    int y = by * 4 + py;

                    if (x < width && y < height) {
                        int srcIdx = (y * width + x) * channels;
                        block[(py * 4 + px) * 4 + 0] = data[srcIdx + 0];                                // R
                        block[(py * 4 + px) * 4 + 1] = channels > 1 ? data[srcIdx + 1] : data[srcIdx];  // G
                        block[(py * 4 + px) * 4 + 2] = channels > 2 ? data[srcIdx + 2] : data[srcIdx];  // B
                        block[(py * 4 + px) * 4 + 3] = 255;                                             // A
                    } else {
                        // 填充边界
                        block[(py * 4 + px) * 4 + 0] = 0;
                        block[(py * 4 + px) * 4 + 1] = 0;
                        block[(py * 4 + px) * 4 + 2] = 0;
                        block[(py * 4 + px) * 4 + 3] = 255;
                    }
                }
            }

            // 使用stb_dxt压缩块
            uint8_t* outBlock = &compressedData[(by * blockCountX + bx) * 8];
            stb_compress_dxt_block(outBlock, block, 0, STB_DXT_NORMAL);
        }
    }

    return compressedData;
}

void testCompressedTextures() {
    std::cout << "\n=== 开始测试纹理压缩格式 ===" << std::endl;

    // 加载原始图像
    int width, height, channels;
    unsigned char* data = stbi_load(std::string(shaderPath + "/awesomeface.png").c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "加载纹理失败: " << stbi_failure_reason() << std::endl;
        return;
    }

    std::cout << "原始纹理尺寸: " << width << "x" << height << ", 通道数: " << channels << std::endl;

    // 压缩为BC1格式
    auto compressedData = compressToBC1(data, width, height, 4);
    std::cout << "BC1压缩数据大小: " << compressedData.size() << " 字节" << std::endl;
    std::cout << "压缩比: " << (float)(width * height * 4) / compressedData.size() << ":1" << std::endl;

    try {
        // 测试BC1_RGB_UNORM格式
        std::cout << "\n测试 BC1_RGB_UNORM 格式..." << std::endl;
        HardwareImageCreateInfo bc1UnormCreateInfo;
        bc1UnormCreateInfo.width = width;
        bc1UnormCreateInfo.height = height;
        bc1UnormCreateInfo.format = ImageFormat::BC1_RGB_UNORM;
        bc1UnormCreateInfo.usage = ImageUsage::SampledImage;
        bc1UnormCreateInfo.arrayLayers = 1;
        bc1UnormCreateInfo.mipLevels = 1;
        bc1UnormCreateInfo.initialData = compressedData.data();

        HardwareImage textureBC1Unorm(bc1UnormCreateInfo);
        std::cout << "✓ BC1_RGB_UNORM 纹理创建成功，描述符ID: " << textureBC1Unorm.storeDescriptor() << std::endl;

        // 测试BC1_RGB_SRGB格式
        std::cout << "\n测试 BC1_RGB_SRGB 格式..." << std::endl;
        HardwareImageCreateInfo bc1SrgbCreateInfo;
        bc1SrgbCreateInfo.width = width;
        bc1SrgbCreateInfo.height = height;
        bc1SrgbCreateInfo.format = ImageFormat::BC1_RGB_SRGB;
        bc1SrgbCreateInfo.usage = ImageUsage::SampledImage;
        bc1SrgbCreateInfo.arrayLayers = 1;
        bc1SrgbCreateInfo.mipLevels = 1;
        bc1SrgbCreateInfo.initialData = compressedData.data();

        HardwareImage textureBC1Srgb(bc1SrgbCreateInfo);
        std::cout << "✓ BC1_RGB_SRGB 纹理创建成功，描述符ID: " << textureBC1Srgb.storeDescriptor() << std::endl;

        std::cout << "\n=== 所有压缩格式测试通过 ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "✗ 压缩纹理测试失败: " << e.what() << std::endl;
    }

    stbi_image_free(data);
}

int main() {
    // 首先运行压缩纹理测试
    // testCompressedTextures();
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm now_tm;
    localtime_s(&now_tm, &now_c);
    char time_buffer[64];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", &now_tm);

    auto const file_sink = Corona::Kernel::create_file_sink(std::string(time_buffer) + "_log.log");
    Corona::Kernel::CoronaLogger::get_default()->add_sink(file_sink);
    Corona::Kernel::CoronaLogger::get_default()->set_level(Corona::Kernel::LogLevel::debug);
    Corona::Kernel::CoronaLogger::info("Starting main application...");

    if (glfwInit() >= 0) {
        Corona::Kernel::CoronaLogger::info("Main thread started...");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        std::vector<GLFWwindow*> windows(6);
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

        // 创建BC1压缩纹理用于实际渲染测试
        auto compressedData = compressToBC1(data, width, height, channels);

        /*HardwareImageCreateInfo textureCreateInfo;
        textureCreateInfo.width = width;
        textureCreateInfo.height = height;
        textureCreateInfo.format = ImageFormat::RGBA8_SRGB;
        textureCreateInfo.usage = ImageUsage::SampledImage;
        textureCreateInfo.arrayLayers = 1;
        textureCreateInfo.mipLevels = 1;
        textureCreateInfo.initialData = data;*/

        HardwareImageCreateInfo textureCreateInfo;
        textureCreateInfo.width = width;
        textureCreateInfo.height = height;
        textureCreateInfo.format = ImageFormat::BC1_RGB_SRGB;  // 使用BC1_RGB_SRGB进行渲染测试
        textureCreateInfo.usage = ImageUsage::SampledImage;
        textureCreateInfo.arrayLayers = 1;
        textureCreateInfo.mipLevels = 1;
        textureCreateInfo.initialData = compressedData.data();

        HardwareImage texture(textureCreateInfo);

        std::cout << "\n使用BC1_RGB_SRGB压缩纹理进行渲染..." << std::endl;

        std::vector<std::vector<HardwareBuffer>> rasterizerUniformBuffers(windows.size());
        std::vector<HardwareBuffer> computeUniformBuffers(windows.size());

        std::atomic_bool running = true;

        auto meshThread = [&](uint32_t threadIndex) {
            Corona::Kernel::CoronaLogger::info("Mesh thread started...");
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
                    rasterizerUniformBufferObject[i].textureIndex = texture.storeDescriptor();
                    rasterizerUniformBufferObject[i].model = modelMat[i] * ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
                    rasterizerUniformBuffers[threadIndex][i].copyFromData(&(rasterizerUniformBufferObject[i]), sizeof(rasterizerUniformBufferObject[i]));
                }

                computeUniformData.imageID = finalOutputImages[threadIndex].storeDescriptor();
                computeUniformBuffers[threadIndex].copyFromData(&computeUniformData, sizeof(computeUniformData));
            }
        };

        auto renderThread = [&](uint32_t threadIndex) {
            Corona::Kernel::CoronaLogger::info("Render thread started...");
            RasterizerPipeline rasterizer(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));
            ComputePipeline computer(readStringFile(shaderPath + "/compute.glsl"));
            while (running.load()) {
                for (size_t i = 0; i < rasterizerUniformBuffers[threadIndex].size(); i++) {
                    rasterizer["pushConsts.uniformBufferIndex"] = rasterizerUniformBuffers[threadIndex][i].storeDescriptor();
                    rasterizer["inPosition"] = postionBuffer;
                    rasterizer["inColor"] = colorBuffer;
                    rasterizer["inTexCoord"] = uvBuffer;
                    rasterizer["inNormal"] = normalBuffer;
                    rasterizer["outColor"] = finalOutputImages[threadIndex];

                    executors[threadIndex] << rasterizer.record(indexBuffer);
                }

                computer["pushConsts.uniformBufferIndex"] = computeUniformBuffers[threadIndex].storeDescriptor();

                executors[threadIndex] << rasterizer(1920, 1080)
                                       << computer(1920 / 8, 1080 / 8, 1)
                                       << executors[threadIndex].commit();
            }
        };

        // auto displayThread = [&](uint32_t threadIndex) {
        //     Corona::Kernel::CoronaLogger::info("Display thread started...");
        //     HardwareDisplayer displayManager = HardwareDisplayer(glfwGetWin32Window(windows[threadIndex]));
        //     while (running.load()) {
        //         displayManager.wait(executors[threadIndex]) << finalOutputImages[threadIndex];
        //     }
        // };

        for (size_t i = 0; i < windows.size(); i++) {
            std::thread(meshThread, i).detach();
            std::thread(renderThread, i).detach();
            // std::thread(displayThread, i).detach();
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
