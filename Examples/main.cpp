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
    }

    bool initialize()
    {
        if (glfwInit() < 0)
        {
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        windows.resize(RenderConfig::WINDOW_COUNT);
        for (auto& window : windows)
        {
            window = glfwCreateWindow(RenderConfig::WINDOW_WIDTH,
                                      RenderConfig::WINDOW_HEIGHT,
                                      "Cabbage Engine",
                                      nullptr,
                                      nullptr);

            if (!window)
            {
                return false;
            }
        }

        return true;
    }

    void run()
    {
        startRenderThreads();
        mainLoop();
        cleanup();
    }

private:
    void startRenderThreads()
    {
        threads.reserve(windows.size());

        for (auto* window : windows) 
        {
            threads.emplace_back([this, window]()
            {
                try
                {
                    RenderThread renderer(glfwGetWin32Window(window), running);
                    renderer.run();
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Render thread error: " << e.what() << std::endl;
                    running.store(false);
                }
            });
        }
    }

    void mainLoop()
    {
        while (running.load())
        {
            glfwPollEvents();

            for (auto* window : windows)
            {
                if (glfwWindowShouldClose(window))
                {
                    running.store(false);
                    break;
                }
            }
        }
    }

    void cleanup()
    {
        // 等待所有渲染线程结束
        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        // 销毁窗口
        for (auto* window : windows)
        {
            glfwDestroyWindow(window);
        }

        glfwTerminate();
    }

    std::atomic_bool running;
    std::vector<GLFWwindow*> windows;
    std::vector<std::thread> threads;
};

int main()
{
    try
    {
        Application app;

        if (!app.initialize())
        {
            std::cerr << "Failed to initialize application" << std::endl;
            return -1;
        }

        app.run();

    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}