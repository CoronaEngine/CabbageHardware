#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <atomic>
#include <thread>
#include <vector>
#include <memory>

#include "Config.h"
#include "RenderThread.h"

class Application
{
public:
    Application()
        : running(true)
    {
        LOG_INFO("Application: Constructor");
    }

    ~Application()
    {
        LOG_INFO("Application: Destructor");
        cleanup();
    }

    bool initialize()
    {
        LOG_INFO("Application: Initializing");

        if (glfwInit() < 0)
        {
            LOG_ERROR("Failed to initialize GLFW");
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        windows.resize(RenderConfig::WINDOW_COUNT);
        for (int i = 0; i < RenderConfig::WINDOW_COUNT; i++)
        {
            GLFWwindow *window = glfwCreateWindow(RenderConfig::WINDOW_WIDTH,
                                                  RenderConfig::WINDOW_HEIGHT,
                                                  ("Cabbage Engine - Window " + std::to_string(i)).c_str(),
                                                  nullptr, nullptr);

            if (!window)
            {
                LOG_ERROR("Failed to create window " << i);
                return false;
            }

            windows[i] = window;
            LOG_INFO("Created window " << i);
        }

        LOG_INFO("Application: Initialized successfully");
        return true;
    }

    void run()
    {
        LOG_INFO("Application: Starting");
        startRenderThreads();
        mainLoop();
        LOG_INFO("Application: Exiting");
    }

private:
    void startRenderThreads()
    {
        LOG_INFO("Starting " << windows.size() << " render threads");
        threads.reserve(windows.size());

        for (size_t i = 0; i < windows.size(); i++)
        {
            threads.emplace_back([this, i]()
            {
                try
                {
                    RenderThread renderer(glfwGetWin32Window(windows[i]),
                                          running,
                                          static_cast<int>(i));

                    renderer.run();
                }
                catch (const std::exception &e)
                {
                    LOG_ERROR("Thread " << i << " crashed: " << e.what());
                    running.store(false);
                }
            });
        }
    }

    void mainLoop()
    {
        LOG_INFO("Entering main loop");

        while (running.load())
        {
            glfwPollEvents();

            for (auto* window : windows)
            {
                if (glfwWindowShouldClose(window))
                {
                    LOG_INFO("Window close requested");
                    running.store(false);
                    break;
                }
            }
        }

        LOG_INFO("Main loop ended");
    }

    void cleanup()
    {
        if (cleaned)
            return;

        cleaned = true;

        LOG_INFO("Application: Cleaning up");

        // 停止所有线程
        running.store(false);

        // 等待所有渲染线程结束
        LOG_INFO("Waiting for render threads to finish");
        for (size_t i = 0; i < threads.size(); i++)
        {
            if (threads[i].joinable())
            {
                LOG_INFO("Joining thread " << i);
                threads[i].join();
            }
        }
        threads.clear();
        LOG_INFO("All render threads joined");

        // 销毁窗口
        LOG_INFO("Destroying windows");
        for (size_t i = 0; i < windows.size(); i++)
        {
            if (windows[i])
            {
                LOG_INFO("Destroying window " << i);
                glfwDestroyWindow(windows[i]);
            }
        }
        windows.clear();

        // 终止 GLFW
        LOG_INFO("Terminating GLFW");
        glfwTerminate();

        LOG_INFO("Application: Cleanup complete");
    }

    std::atomic_bool running;
    std::vector<GLFWwindow *> windows;
    std::vector<std::thread> threads;
    bool cleaned = false;
};

int main()
{
    LOG_INFO("=== Program Started ===");

    try
    {
        Application app;

        if (!app.initialize())
        {
            LOG_ERROR("Failed to initialize application");
            return -1;
        }

        app.run();
        LOG_INFO("=== Program Ended Normally ===");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Fatal error: " << e.what());
        LOG_INFO("=== Program Crashed ===");
        return -1;
    }

    return 0;
}