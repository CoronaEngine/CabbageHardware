#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "CabbageHardware.h"
#include "Pipeline/ComputePipeline.h"
#include "Pipeline/RasterizerPipeline.h"
#include "Config.h"
#include "CubeData.h"

class RenderThread
{
public:
    RenderThread(void *surface, std::atomic_bool &running, int threadId)
        : running(running), displayManager(surface), startTime(std::chrono::high_resolution_clock::now()), threadId(threadId)
    {
        LOG_INFO("Thread " << threadId << ": Initializing");

        try
        {
            initializeResources();
            initializePipelines();
            initializeCubes();
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Thread " << threadId << ": Initialization failed - " << e.what());
            throw;
        }

        LOG_INFO("Thread " << threadId << ": Initialized successfully");
    }

    ~RenderThread()
    {
        LOG_INFO("Thread " << threadId << ": Destroying");
        cleanup();
        LOG_INFO("Thread " << threadId << ": Destroyed");
    }

    void run()
    {
        LOG_INFO("Thread " << threadId << ": Starting render loop");

        while (running.load())
        {
            try
            {
                renderFrame();
                updateFPSCounter();
            }
            catch (const std::exception &e)
            {
                LOG_ERROR("Thread " << threadId << ": Render error - " << e.what());
                running.store(false);
                break;
            }
        }

        LOG_INFO("Thread " << threadId << ": Render loop ended");
    }

private:
    void initializeResources()
    {
        // 创建顶点缓冲区
        LOG_RESOURCE("Creating", "Vertex Buffers");
        positionBuffer = std::make_unique<HardwareBuffer>(positions, BufferUsage::VertexBuffer);
        normalBuffer = std::make_unique<HardwareBuffer>(normals, BufferUsage::VertexBuffer);
        uvBuffer = std::make_unique<HardwareBuffer>(uvs, BufferUsage::VertexBuffer);
        colorBuffer = std::make_unique<HardwareBuffer>(colors, BufferUsage::VertexBuffer);
        indexBuffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);

        // 创建 Uniform 缓冲区
        LOG_RESOURCE("Creating", "Uniform Buffer");
        computeUniformBuffer = std::make_unique<HardwareBuffer>(sizeof(ComputeUniformBufferObject),
                                                                BufferUsage::UniformBuffer);

        // 加载纹理
        LOG_RESOURCE("Loading", "Texture");
        loadTexture();

        // 创建输出图像
        LOG_RESOURCE("Creating", "Output Image");
        finalOutputImage = std::make_unique<HardwareImage>(RenderConfig::WINDOW_WIDTH,
                                                           RenderConfig::WINDOW_HEIGHT,
                                                           ImageFormat::RGBA16_FLOAT,
                                                           ImageUsage::StorageImage);
    }

    void loadTexture()
    {
        int width, height, channels;
        unsigned char* data = stbi_load(std::string(shaderPath + "/awesomeface.png").c_str(),
                                        &width,
                                        &height,
                                        &channels,
                                        0);

        if (!data)
        {
            LOG_ERROR("Failed to load texture: " << stbi_failure_reason());
            throw std::runtime_error("Texture loading failed");
        }

        texture = std::make_unique<HardwareImage>(width,
                                                  height, ImageFormat::RGBA8_SRGB,
                                                  ImageUsage::SampledImage,
                                                  1,
                                                  data);

        stbi_image_free(data);
        LOG_INFO("Texture loaded: " << width << "x" << height);
    }

    void initializePipelines()
    {
        LOG_RESOURCE("Creating", "Rasterizer Pipeline");
        rasterizer = std::make_unique<RasterizerPipelineVulkan>(readStringFile(shaderPath + "/vert.glsl"), readStringFile(shaderPath + "/frag.glsl"));
        LOG_RESOURCE("Creating", "Compute Pipeline");
        computer = std::make_unique<ComputePipelineVulkan>(readStringFile(shaderPath + "/compute.glsl"));
    }

    void initializeCubes()
    {
        LOG_RESOURCE("Creating", "Cube Data");
        cubeTransforms.reserve(RenderConfig::CUBE_COUNT);
        cubeUniformBuffers.reserve(RenderConfig::CUBE_COUNT);

        for (int i = 0; i < RenderConfig::CUBE_COUNT; ++i)
        {
            ktm::fmat4x4 modelMat = ktm::translate3d(ktm::fvec3((i % 5) - 2.0f, (i / 5) - 0.5f, 0.0f)) *
                                    ktm::scale3d(ktm::fvec3(0.1f, 0.1f, 0.1f)) *
                                    ktm::rotate3d_axis(ktm::radians(i * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));

            cubeTransforms.push_back(modelMat);
            cubeUniformBuffers.emplace_back(sizeof(RasterizerUniformBufferObject),
                                            BufferUsage::UniformBuffer);
        }
    }

    void renderFrame()
    {
        float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();

        // 渲染所有立方体
        for (size_t i = 0; i < cubeTransforms.size(); ++i)
        {
            renderCube(i, time);
        }
        // 计算着色器后处理
        runComputeShader();

        // 提交光栅化命令
        executor << (*rasterizer)(RenderConfig::WINDOW_WIDTH, RenderConfig::WINDOW_HEIGHT)
                 << (*computer)(RenderConfig::WINDOW_WIDTH / RenderConfig::COMPUTE_GROUP_SIZE,
                                RenderConfig::WINDOW_HEIGHT / RenderConfig::COMPUTE_GROUP_SIZE, 1)
                 << executor.commit();

        // 显示结果
        displayManager.wait(executor) << *finalOutputImage;
    }

    void renderCube(size_t index, float time)
    {
        RasterizerUniformBufferObject ubo;
        ubo.textureIndex = texture->storeDescriptor();
        ubo.model = cubeTransforms[index] * ktm::rotate3d_axis(time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));

        cubeUniformBuffers[index].copyFromData(&ubo, sizeof(ubo));

        (*rasterizer)["pushConsts.uniformBufferIndex"] = cubeUniformBuffers[index].storeDescriptor();
        (*rasterizer)["inPosition"] = *positionBuffer;
        (*rasterizer)["inColor"] = *colorBuffer;
        (*rasterizer)["inTexCoord"] = *uvBuffer;
        (*rasterizer)["inNormal"] = *normalBuffer;
        (*rasterizer)["outColor"] = *finalOutputImage;

        executor << rasterizer->record(*indexBuffer);
    }

    void runComputeShader()
    {
        ComputeUniformBufferObject computeUBO;
        computeUBO.imageID = finalOutputImage->storeDescriptor();
        computeUniformBuffer->copyFromData(&computeUBO, sizeof(computeUBO));

        (*computer)["pushConsts.uniformBufferIndex"] = computeUniformBuffer->storeDescriptor();
    }

    void updateFPSCounter()
    {
        auto frameEnd = std::chrono::high_resolution_clock::now();
        totalTimeMs += std::chrono::duration<float, std::chrono::milliseconds::period>(frameEnd - frameStart).count();
        frameStart = frameEnd;
        frameCount++;

        if (frameCount >= RenderConfig::FRAME_AVERAGE_COUNT)
        {
            std::cout << "Thread " << threadId << " - Average frame time: "
                      << totalTimeMs / frameCount << " ms" << std::endl;
            totalTimeMs = 0.0;
            frameCount = 0;
        }
    }

    void cleanup()
    {
        LOG_INFO("Thread " << threadId << ": Cleaning up resources");

        // 按照相反的创建顺序销毁资源
        LOG_RESOURCE("Destroying", "Cube Data");
        cubeUniformBuffers.clear();
        cubeTransforms.clear();

        LOG_RESOURCE("Destroying", "Pipelines");
        computer.reset();
        rasterizer.reset();

        LOG_RESOURCE("Destroying", "Images");
        finalOutputImage.reset();
        texture.reset();

        LOG_RESOURCE("Destroying", "Buffers");
        computeUniformBuffer.reset();
        indexBuffer.reset();
        colorBuffer.reset();
        uvBuffer.reset();
        normalBuffer.reset();
        positionBuffer.reset();

        LOG_INFO("Thread " << threadId << ": Resources cleaned up");
    }

    // 成员变量
    std::atomic_bool& running;
    HardwareDisplayer displayManager;
    HardwareExecutor executor;
    int threadId;

    // 资源 - 按销毁顺序排列

    // 管线
    std::unique_ptr<RasterizerPipelineVulkan> rasterizer;
    std::unique_ptr<ComputePipelineVulkan> computer;

    // 立方体数据
    std::vector<HardwareBuffer> cubeUniformBuffers;
    std::vector<ktm::fmat4x4> cubeTransforms;

    std::unique_ptr<HardwareImage> finalOutputImage;
    std::unique_ptr<HardwareImage> texture;

    // 资源
    std::unique_ptr<HardwareBuffer> computeUniformBuffer;
    std::unique_ptr<HardwareBuffer> indexBuffer;
    std::unique_ptr<HardwareBuffer> colorBuffer;
    std::unique_ptr<HardwareBuffer> uvBuffer;
    std::unique_ptr<HardwareBuffer> normalBuffer;
    std::unique_ptr<HardwareBuffer> positionBuffer;

    // 计时
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
    double totalTimeMs = 0.0;
    int frameCount = 0;
};