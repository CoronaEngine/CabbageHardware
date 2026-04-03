#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "CabbageHardware.h"
#include "multishader/DropOldestQueue.h"
#include "multishader/HelloTriangleScenario.h"
#include "multishader/RuntimeConfig.h"
#include "multishader/Scenario.h"
#include "multishader/ScenarioRegistry.h"

using multishader::Backend;
using multishader::Clock;
using multishader::DropOldestQueue;
using multishader::MeshFrame;
using multishader::RenderFrame;

struct RuntimeStats
{
    std::atomic<uint64_t> meshFramesProduced{0};
    std::atomic<uint64_t> edslRenderFrames{0};
    std::atomic<uint64_t> glslRenderFrames{0};
    std::atomic<uint64_t> displayLoopTicks{0};
    std::atomic<uint64_t> edslDisplayFrames{0};
    std::atomic<uint64_t> glslDisplayFrames{0};
    std::atomic<uint64_t> edslLatencyUsTotal{0};
    std::atomic<uint64_t> glslLatencyUsTotal{0};
    std::atomic<uint64_t> latestEdslFrameId{0};
    std::atomic<uint64_t> latestGlslFrameId{0};
};

// 基础自检：验证 drop-oldest 队列的核心行为是否符合预期。
bool runQueueSelfTest()
{
    DropOldestQueue<int> queue(3);
    for (int i = 0; i < 10; ++i)
    {
        if (!queue.push(i))
        {
            return false;
        }
    }
    if (queue.droppedCount() != 7)
    {
        return false;
    }
    auto latest = queue.try_pop_all_latest();
    if (!latest.has_value() || latest.value() != 9)
    {
        return false;
    }
    queue.close();
    auto maybeValue = queue.pop_wait();
    return !maybeValue.has_value();
}

// 打印当前已注册场景列表，便于命令行选择。
void printAvailableScenarios()
{
    auto names = multishader::listScenarios();
    std::ostringstream oss;
    oss << "Available scenarios: ";
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        if (i > 0)
        {
            oss << ", ";
        }
        oss << names[i];
    }
    std::cout << oss.str() << '\n';
}
} // 匿名命名空间

int main(int argc, char **argv)
{
    // 1) 解析运行参数（场景、数据集、队列深度、窗口尺寸等）。
    multishader::RuntimeConfig config;
    try
    {
        config = multishader::parseRuntimeConfig(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to parse runtime config: " << e.what() << '\n';
        std::cerr << multishader::runtimeConfigUsage(argc > 0 ? argv[0] : "CabbageHardwareExamples");
        return -1;
    }

    if (config.showHelp)
    {
        std::cout << multishader::runtimeConfigUsage(argc > 0 ? argv[0] : "CabbageHardwareExamples");
        return 0;
    }

    if (!runQueueSelfTest())
    {
        std::cerr << "DropOldestQueue self-test failed, aborting.\n";
        return -1;
    }

    multishader::registerHelloTriangleScenario();

    // 2) 初始化窗口与输出图像。
    if (glfwInit() < 0)
    {
        std::cerr << "glfwInit failed.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    std::array<GLFWwindow *, 2> windows{nullptr, nullptr};
    auto destroyWindowsAndTerminate = [&] {
        for (GLFWwindow *window : windows)
        {
            if (window != nullptr)
            {
                glfwDestroyWindow(window);
            }
        }
        glfwTerminate();
    };

    windows[0] = glfwCreateWindow(static_cast<int>(config.windowWidth),
                                  static_cast<int>(config.windowHeight),
                                  "Cabbage Engine [EDSL]",
                                  nullptr,
                                  nullptr);
    windows[1] = glfwCreateWindow(static_cast<int>(config.windowWidth),
                                  static_cast<int>(config.windowHeight),
                                  "Cabbage Engine [GLSL]",
                                  nullptr,
                                  nullptr);
    if (windows[0] == nullptr || windows[1] == nullptr)
    {
        std::cerr << "Failed to create GLFW windows.\n";
        destroyWindowsAndTerminate();
        return -1;
    }

    std::array<HardwareImage, 2> finalOutputImages;
    std::array<HardwareExecutor, 2> executors;
    for (std::size_t i = 0; i < finalOutputImages.size(); ++i)
    {
        HardwareImageCreateInfo createInfo;
        createInfo.width = config.windowWidth;
        createInfo.height = config.windowHeight;
        createInfo.format = ImageFormat::RGBA16_FLOAT;
        createInfo.usage = ImageUsage::StorageImage;
        createInfo.arrayLayers = 1;
        createInfo.mipLevels = 1;
        finalOutputImages[i] = HardwareImage(createInfo);
    }

    // 3) 根据场景名创建并初始化场景实现。
    auto scenario = multishader::createScenario(config.scenario, config);
    if (!scenario)
    {
        std::cerr << "Unknown scenario: " << config.scenario << '\n';
        printAvailableScenarios();
        destroyWindowsAndTerminate();
        return -1;
    }

    std::string scenarioError;
    if (!scenario->init(config, finalOutputImages, scenarioError))
    {
        std::cerr << "Scenario init failed: " << scenarioError << '\n';
        destroyWindowsAndTerminate();
        return -1;
    }

    DropOldestQueue<MeshFrame> meshToEdsl(config.queueDepth);
    DropOldestQueue<MeshFrame> meshToGlsl(config.queueDepth);
    DropOldestQueue<RenderFrame> edslToDisplay(config.queueDepth);
    DropOldestQueue<RenderFrame> glslToDisplay(config.queueDepth);

    RuntimeStats stats;
    std::atomic_bool running{true};
    std::atomic_bool hasError{false};
    std::mutex errorMutex;
    std::string errorMessage;

    auto requestStop = [&] {
        // 统一停止入口：关闭运行标记并唤醒所有队列等待点。
        running.store(false);
        meshToEdsl.close();
        meshToGlsl.close();
        edslToDisplay.close();
        glslToDisplay.close();
    };

    auto setErrorAndStop = [&](const std::string &threadName, const std::string &message) {
        // 仅记录首个错误，避免多线程并发覆盖关键信息。
        bool expected = false;
        if (hasError.compare_exchange_strong(expected, true))
        {
            std::lock_guard<std::mutex> lock(errorMutex);
            errorMessage = threadName + ": " + message;
        }
        requestStop();
    };

    auto meshThread = [&] {
        // meshThread：生成单调 frameId 和共享 payload，同时投喂两个 render 输入队列。
        try
        {
            uint64_t frameId = 0;
            const auto frameInterval = (config.maxFps > 0)
                                           ? std::chrono::microseconds(1'000'000 / config.maxFps)
                                           : std::chrono::microseconds(0);

            while (running.load())
            {
                auto frameBegin = Clock::now();
                std::string meshError;

                MeshFrame frame;
                frame.frameId = ++frameId;
                frame.timestamp = frameBegin;
                frame.payload = scenario->meshTick(frame.frameId, frame.timestamp, meshError);

                if (!frame.payload)
                {
                    setErrorAndStop("meshThread", meshError.empty() ? "meshTick returned empty payload" : meshError);
                    break;
                }

                MeshFrame edslFrame = frame;
                if (!meshToEdsl.push(std::move(edslFrame)) || !meshToGlsl.push(std::move(frame)))
                {
                    break;
                }

                stats.meshFramesProduced.fetch_add(1, std::memory_order_relaxed);

                if (frameInterval.count() > 0)
                {
                    auto elapsed = Clock::now() - frameBegin;
                    if (elapsed < frameInterval)
                    {
                        std::this_thread::sleep_for(frameInterval - elapsed);
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            setErrorAndStop("meshThread", e.what());
        }
        requestStop();
    };

    auto renderThreadEDSL = [&] {
        // renderThreadEDSL：消费 mesh 帧，执行 EDSL 渲染并把提交结果发给 display 队列。
        try
        {
            while (running.load() || !meshToEdsl.isClosedAndEmpty())
            {
                auto meshFrame = meshToEdsl.pop_wait();
                if (!meshFrame.has_value())
                {
                    break;
                }

                std::string renderError;
                if (!scenario->renderEDSLTick(meshFrame.value(), executors[0], finalOutputImages[0], renderError))
                {
                    setErrorAndStop("renderThreadEDSL", renderError.empty() ? "renderEDSLTick failed" : renderError);
                    break;
                }

                RenderFrame renderFrame;
                renderFrame.frameId = meshFrame->frameId;
                renderFrame.backend = Backend::EDSL;
                renderFrame.outputImage = &finalOutputImages[0];
                renderFrame.executorRef = &executors[0];
                renderFrame.submitTimestamp = Clock::now();
                if (!edslToDisplay.push(std::move(renderFrame)))
                {
                    break;
                }

                stats.edslRenderFrames.fetch_add(1, std::memory_order_relaxed);
            }
        }
        catch (const std::exception &e)
        {
            setErrorAndStop("renderThreadEDSL", e.what());
        }
        edslToDisplay.close();
    };

    auto renderThreadGLSL = [&] {
        // renderThreadGLSL：消费 mesh 帧，执行 GLSL 渲染并把提交结果发给 display 队列。
        try
        {
            while (running.load() || !meshToGlsl.isClosedAndEmpty())
            {
                auto meshFrame = meshToGlsl.pop_wait();
                if (!meshFrame.has_value())
                {
                    break;
                }

                std::string renderError;
                if (!scenario->renderGLSLTick(meshFrame.value(), executors[1], finalOutputImages[1], renderError))
                {
                    setErrorAndStop("renderThreadGLSL", renderError.empty() ? "renderGLSLTick failed" : renderError);
                    break;
                }

                RenderFrame renderFrame;
                renderFrame.frameId = meshFrame->frameId;
                renderFrame.backend = Backend::GLSL;
                renderFrame.outputImage = &finalOutputImages[1];
                renderFrame.executorRef = &executors[1];
                renderFrame.submitTimestamp = Clock::now();
                if (!glslToDisplay.push(std::move(renderFrame)))
                {
                    break;
                }

                stats.glslRenderFrames.fetch_add(1, std::memory_order_relaxed);

                if (config.glslArtificialDelayMs > 0)
                {
                    // 可选人工延迟，用于复现/观察背压和丢帧策略。
                    std::this_thread::sleep_for(std::chrono::milliseconds(config.glslArtificialDelayMs));
                }
            }
        }
        catch (const std::exception &e)
        {
            setErrorAndStop("renderThreadGLSL", e.what());
        }
        glslToDisplay.close();
    };

    auto displayThread = [&] {
        // displayThread：每轮取两条渲染输出队列的“最新帧”，避免被旧帧积压拖慢。
        try
        {
            HardwareDisplayer edslDisplayer(glfwGetWin32Window(windows[0]));
            HardwareDisplayer glslDisplayer(glfwGetWin32Window(windows[1]));

            std::optional<RenderFrame> latestEdslFrame;
            std::optional<RenderFrame> latestGlslFrame;

            while (running.load() || !edslToDisplay.isClosedAndEmpty() || !glslToDisplay.isClosedAndEmpty())
            {
                bool gotNewFrame = false;

                auto latestEdsl = edslToDisplay.try_pop_all_latest();
                if (latestEdsl.has_value())
                {
                    latestEdslFrame = std::move(latestEdsl.value());
                    gotNewFrame = true;
                }

                auto latestGlsl = glslToDisplay.try_pop_all_latest();
                if (latestGlsl.has_value())
                {
                    latestGlslFrame = std::move(latestGlsl.value());
                    gotNewFrame = true;
                }

                auto now = Clock::now();

                if (latestEdslFrame.has_value() &&
                    latestEdslFrame->outputImage != nullptr &&
                    latestEdslFrame->executorRef != nullptr)
                {
                    // 对 EDSL 最新可用帧进行 wait + present。
                    edslDisplayer.wait(*latestEdslFrame->executorRef) << *latestEdslFrame->outputImage;
                    scenario->displayTick(latestEdslFrame.value());
                    if (latestEdsl.has_value())
                    {
                        stats.latestEdslFrameId.store(latestEdslFrame->frameId, std::memory_order_relaxed);
                        auto latencyUs = std::chrono::duration_cast<std::chrono::microseconds>(
                                             now - latestEdslFrame->submitTimestamp)
                                             .count();
                        stats.edslLatencyUsTotal.fetch_add(static_cast<uint64_t>(latencyUs), std::memory_order_relaxed);
                        stats.edslDisplayFrames.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                if (latestGlslFrame.has_value() &&
                    latestGlslFrame->outputImage != nullptr &&
                    latestGlslFrame->executorRef != nullptr)
                {
                    // 对 GLSL 最新可用帧进行 wait + present。
                    glslDisplayer.wait(*latestGlslFrame->executorRef) << *latestGlslFrame->outputImage;
                    scenario->displayTick(latestGlslFrame.value());
                    if (latestGlsl.has_value())
                    {
                        stats.latestGlslFrameId.store(latestGlslFrame->frameId, std::memory_order_relaxed);
                        auto latencyUs = std::chrono::duration_cast<std::chrono::microseconds>(
                                             now - latestGlslFrame->submitTimestamp)
                                             .count();
                        stats.glslLatencyUsTotal.fetch_add(static_cast<uint64_t>(latencyUs), std::memory_order_relaxed);
                        stats.glslDisplayFrames.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                stats.displayLoopTicks.fetch_add(1, std::memory_order_relaxed);
                if (!gotNewFrame)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
        catch (const std::exception &e)
        {
            setErrorAndStop("displayThread", e.what());
        }
    };

    std::thread renderEDSLWorker(renderThreadEDSL);
    std::thread renderGLSLWorker(renderThreadGLSL);
    std::thread displayWorker(displayThread);
    std::thread meshWorker(meshThread);

    // 4) 主线程负责窗口事件与周期统计输出。
    auto lastStatTick = Clock::now();
    uint64_t prevMeshFrames = 0;
    uint64_t prevEdslFrames = 0;
    uint64_t prevGlslFrames = 0;
    uint64_t prevDisplayTicks = 0;

    while (running.load())
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(windows[0]) || glfwWindowShouldClose(windows[1]))
        {
            requestStop();
            break;
        }

        if (config.enableCompareStats)
        {
            auto now = Clock::now();
            if (now - lastStatTick >= std::chrono::seconds(1))
            {
                uint64_t meshFrames = stats.meshFramesProduced.load(std::memory_order_relaxed);
                uint64_t edslFrames = stats.edslRenderFrames.load(std::memory_order_relaxed);
                uint64_t glslFrames = stats.glslRenderFrames.load(std::memory_order_relaxed);
                uint64_t displayTicks = stats.displayLoopTicks.load(std::memory_order_relaxed);

                uint64_t meshFps = meshFrames - prevMeshFrames;
                uint64_t edslFps = edslFrames - prevEdslFrames;
                uint64_t glslFps = glslFrames - prevGlslFrames;
                uint64_t displayFps = displayTicks - prevDisplayTicks;

                prevMeshFrames = meshFrames;
                prevEdslFrames = edslFrames;
                prevGlslFrames = glslFrames;
                prevDisplayTicks = displayTicks;

                uint64_t latestEdsl = stats.latestEdslFrameId.load(std::memory_order_relaxed);
                uint64_t latestGlsl = stats.latestGlslFrameId.load(std::memory_order_relaxed);
                uint64_t frameGap = (latestEdsl >= latestGlsl) ? (latestEdsl - latestGlsl) : (latestGlsl - latestEdsl);

                uint64_t edslDisplayed = stats.edslDisplayFrames.load(std::memory_order_relaxed);
                uint64_t glslDisplayed = stats.glslDisplayFrames.load(std::memory_order_relaxed);
                double avgEdslLatencyMs = (edslDisplayed == 0)
                                              ? 0.0
                                              : static_cast<double>(stats.edslLatencyUsTotal.load(std::memory_order_relaxed)) / 1000.0 / static_cast<double>(edslDisplayed);
                double avgGlslLatencyMs = (glslDisplayed == 0)
                                              ? 0.0
                                              : static_cast<double>(stats.glslLatencyUsTotal.load(std::memory_order_relaxed)) / 1000.0 / static_cast<double>(glslDisplayed);

                std::cout << "[Stats] meshFPS=" << meshFps
                          << " edslRenderFPS=" << edslFps
                          << " glslRenderFPS=" << glslFps
                          << " displayLoopFPS=" << displayFps
                          << " frameGap=" << frameGap
                          << " drops(mesh->edsl=" << meshToEdsl.droppedCount()
                          << ", mesh->glsl=" << meshToGlsl.droppedCount()
                          << ", edsl->display=" << edslToDisplay.droppedCount()
                          << ", glsl->display=" << glslToDisplay.droppedCount() << ")"
                          << " avgLatencyMs(edsl=" << avgEdslLatencyMs
                          << ", glsl=" << avgGlslLatencyMs << ")\n";

                lastStatTick = now;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    requestStop();

    // 5) 线程收敛与场景清理。
    if (meshWorker.joinable())
    {
        meshWorker.join();
    }
    if (renderEDSLWorker.joinable())
    {
        renderEDSLWorker.join();
    }
    if (renderGLSLWorker.joinable())
    {
        renderGLSLWorker.join();
    }
    if (displayWorker.joinable())
    {
        displayWorker.join();
    }

    scenario->shutdown();
    destroyWindowsAndTerminate();

    if (hasError.load())
    {
        std::lock_guard<std::mutex> lock(errorMutex);
        std::cerr << "Fatal error: " << errorMessage << '\n';
        return -1;
    }

    return 0;
}
