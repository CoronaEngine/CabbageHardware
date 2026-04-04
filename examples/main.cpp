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
#include <utility>
#include <vector>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "CabbageHardware.h"
#include "drop_oldest_queue.h"
#include "runtime_config.h"
#include "scenario.h"
#include "scenario_registry.h"

#include "default_scenario.h"
#include "hello_triangle/hello_triangle_scenario.h"

struct RuntimeStats
{
    // Mesh 线程统计：成功生成并投递的总帧数
    std::atomic<uint64_t> mesh_frames_produced{0};

    // Render 线程统计：EDSL / GLSL 各自完成提交的总帧数
    std::atomic<uint64_t> edsl_render_frames{0};
    std::atomic<uint64_t> glsl_render_frames{0};

    // Display 线程统计：显示循环执行次数与两路实际显示帧数
    std::atomic<uint64_t> display_loop_ticks{0};
    std::atomic<uint64_t> edsl_display_frames{0};
    std::atomic<uint64_t> glsl_display_frames{0};

    // 延迟统计：render 提交到 display 呈现的累计延迟（微秒）
    std::atomic<uint64_t> edsl_latency_us_total{0};
    std::atomic<uint64_t> glsl_latency_us_total{0};

    // 对齐统计：两路最近一次显示的 frame_id（用于计算帧差）
    std::atomic<uint64_t> latest_edsl_frame_id{0};
    std::atomic<uint64_t> latest_glsl_frame_id{0};
};

// 基础自检：验证 drop-oldest 队列的核心行为是否符合预期。
bool run_queue_self_test()
{
    DropOldestQueue<int> queue(3);
    for (int i = 0; i < 10; ++i)
    {
        if (!queue.push(i))
        {
            return false;
        }
    }

    if (queue.dropped_count() != 7)
    {
        return false;
    }

    auto latest = queue.try_pop_all_latest();

    if (!latest.has_value() || latest.value() != 9)
    {
        return false;
    }

    queue.close();
    auto maybe_value = queue.pop_wait();
    return !maybe_value.has_value();
}

// 打印当前已注册场景列表，便于命令行选择。
void print_available_scenarios()
{
    auto names = list_scenarios();
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

int main(int argc, char **argv)
{
    // 1 解析运行参数（场景、数据集、队列深度、窗口尺寸等）。
    RuntimeConfig config;

    try
    {
        config = parse_runtime_config(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to parse runtime config: " << e.what() << '\n';
        std::cerr << runtime_config_usage(argc > 0 ? argv[0] : "CabbageHardwareExamples");
        return -1;
    }

    if (config.show_help)
    {
        std::cout << runtime_config_usage(argc > 0 ? argv[0] : "CabbageHardwareExamples");
        return 0;
    }

    if (!run_queue_self_test())
    {
        std::cerr << "DropOldestQueue self-test failed, aborting.\n";
        return -1;
    }

    register_default_scenario();
    register_hello_triangle_scenario();

    // 2 初始化窗口与输出图像。
    if (glfwInit() < 0)
    {
        std::cerr << "glfwInit failed.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    std::array<GLFWwindow *, 2> windows{nullptr, nullptr};
    auto destroy_windows_and_terminate = [&] {
        for (GLFWwindow *window : windows)
        {
            if (window != nullptr)
            {
                glfwDestroyWindow(window);
            }
        }
        glfwTerminate();
    };

    windows[0] = glfwCreateWindow(static_cast<int>(config.window_width),
                                  static_cast<int>(config.window_height),
                                  "Cabbage Engine [EDSL]",
                                  nullptr,
                                  nullptr);

    windows[1] = glfwCreateWindow(static_cast<int>(config.window_width),
                                  static_cast<int>(config.window_height),
                                  "Cabbage Engine [GLSL]",
                                  nullptr,
                                  nullptr);

    if (windows[0] == nullptr || windows[1] == nullptr)
    {
        std::cerr << "Failed to create GLFW windows.\n";
        destroy_windows_and_terminate();
        return -1;
    }

    std::array<HardwareImage, 2> final_output_images;
    std::array<HardwareExecutor, 2> executors;
    for (std::size_t i = 0; i < final_output_images.size(); ++i)
    {
        HardwareImageCreateInfo create_Info;
        create_Info.width = config.window_width;
        create_Info.height = config.window_height;
        create_Info.format = ImageFormat::RGBA16_FLOAT;
        create_Info.usage = ImageUsage::StorageImage;
        create_Info.arrayLayers = 1;
        create_Info.mipLevels = 1;
        final_output_images[i] = HardwareImage(create_Info);
    }

    // 3 根据场景名创建并初始化场景实现。
    auto scenario = create_scenario(config.scenario, config);
    if (!scenario)
    {
        std::cerr << "Unknown scenario: " << config.scenario << '\n';
        print_available_scenarios();
        destroy_windows_and_terminate();
        return -1;
    }

    std::string scenario_error;
    if (!scenario->init(config, final_output_images, scenario_error))
    {
        std::cerr << "Scenario init failed: " << scenario_error << '\n';
        destroy_windows_and_terminate();
        return -1;
    }

    DropOldestQueue<MeshFrame> mesh_to_edsl(config.queue_depth);
    DropOldestQueue<MeshFrame> mesh_to_glsl(config.queue_depth);
    DropOldestQueue<RenderFrame> edsl_to_display(config.queue_depth);
    DropOldestQueue<RenderFrame> glsl_to_display(config.queue_depth);

    RuntimeStats stats;
    std::atomic_bool running{true};
    std::atomic_bool has_error{false};
    std::mutex error_mutex;
    std::string error_message;

    auto request_stop = [&] {
        // 统一停止入口：关闭运行标记并唤醒所有队列等待点。
        running.store(false);
        mesh_to_edsl.close();
        mesh_to_glsl.close();
        edsl_to_display.close();
        glsl_to_display.close();
    };

    auto set_error_and_stop = [&](const std::string &thread_name, const std::string &message) {
        // 仅记录首个错误，避免多线程并发覆盖关键信息。
        bool expected = false;
        if (has_error.compare_exchange_strong(expected, true))
        {
            std::lock_guard<std::mutex> lock(error_mutex);
            error_message = thread_name + ": " + message;
        }
        request_stop();
    };

    auto mesh_thread = [&] {
        // meshThread：生成单调 frameId 和共享 payload，同时投喂两个 render 输入队列。
        try
        {
            uint64_t frame_id = 0;
            const auto frame_interval = (config.max_fps > 0) ? std::chrono::microseconds(1'000'000 / config.max_fps)
                                                             : std::chrono::microseconds(0);

            while (running.load())
            {
                auto frame_begin = Clock::now();
                std::string mesh_error;

                MeshFrame frame;
                frame.frame_id = ++frame_id;
                frame.timestamp = frame_begin;
                frame.payload = scenario->mesh_tick(frame.frame_id, frame.timestamp, mesh_error);

                if (!frame.payload)
                {
                    set_error_and_stop("MeshThread", mesh_error.empty() ? "MeshTick returned empty payload" : mesh_error);
                    break;
                }

                MeshFrame edsl_frame = frame;
                if (!mesh_to_edsl.push(std::move(edsl_frame)) || !mesh_to_glsl.push(std::move(frame)))
                {
                    break;
                }

                stats.mesh_frames_produced.fetch_add(1, std::memory_order_relaxed);

                if (frame_interval.count() > 0)
                {
                    auto elapsed = Clock::now() - frame_begin;
                    if (elapsed < frame_interval)
                    {
                        std::this_thread::sleep_for(frame_interval - elapsed);
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            set_error_and_stop("MeshThread", e.what());
        }
        request_stop();
    };

    auto render_thread_edsl = [&] {
        // renderThreadEDSL：消费 mesh 帧，执行 EDSL 渲染并把提交结果发给 display 队列。
        try
        {
            while (running.load() || !mesh_to_edsl.is_closed_and_empty())
            {
                auto mesh_frame = mesh_to_edsl.pop_wait();
                if (!mesh_frame.has_value())
                {
                    break;
                }

                std::string render_error;
                if (!scenario->render_edsl_tick(mesh_frame.value(), executors[0], final_output_images[0], render_error))
                {
                    set_error_and_stop("RenderThreadEDSL",
                                       render_error.empty() ? "render_edsl_tick failed" : render_error);
                    break;
                }

                RenderFrame render_frame;
                render_frame.frame_id = mesh_frame->frame_id;
                render_frame.backend = Backend::EDSL;
                render_frame.output_image = &final_output_images[0];
                render_frame.executor_ref = &executors[0];
                render_frame.submit_timestamp = Clock::now();
                if (!edsl_to_display.push(std::move(render_frame)))
                {
                    break;
                }

                stats.edsl_render_frames.fetch_add(1, std::memory_order_relaxed);
            }
        }
        catch (const std::exception &e)
        {
            set_error_and_stop("RenderThreadEDSL", e.what());
        }
        edsl_to_display.close();
    };

    auto render_thread_glsl = [&] {
        // renderThreadGLSL：消费 mesh 帧，执行 GLSL 渲染并把提交结果发给 display 队列。
        try
        {
            while (running.load() || !mesh_to_glsl.is_closed_and_empty())
            {
                auto mesh_frame = mesh_to_glsl.pop_wait();
                if (!mesh_frame.has_value())
                {
                    break;
                }

                std::string render_error;
                if (!scenario->render_glsl_tick(mesh_frame.value(), executors[1], final_output_images[1], render_error))
                {
                    set_error_and_stop("RenderThreadGLSL",
                                       render_error.empty() ? "render_glsl_tick failed" : render_error);
                    break;
                }

                RenderFrame render_frame;
                render_frame.frame_id = mesh_frame->frame_id;
                render_frame.backend = Backend::GLSL;
                render_frame.output_image = &final_output_images[1];
                render_frame.executor_ref = &executors[1];
                render_frame.submit_timestamp = Clock::now();
                if (!glsl_to_display.push(std::move(render_frame)))
                {
                    break;
                }

                stats.glsl_render_frames.fetch_add(1, std::memory_order_relaxed);
            }
        }
        catch (const std::exception &e)
        {
            set_error_and_stop("RenderThreadGLSL", e.what());
        }
        glsl_to_display.close();
    };

    auto display_thread = [&] {
        // displayThread：每轮取两条渲染输出队列的“最新帧”，避免被旧帧积压拖慢。
        try
        {
            HardwareDisplayer edsl_displayer(glfwGetWin32Window(windows[0]));
            HardwareDisplayer glsl_displayer(glfwGetWin32Window(windows[1]));

            std::optional<RenderFrame> latest_edsl_frame;
            std::optional<RenderFrame> latest_glsl_frame;

            while (running.load() || !edsl_to_display.is_closed_and_empty() || !glsl_to_display.is_closed_and_empty())
            {
                bool got_new_frame = false;

                auto latest_edsl = edsl_to_display.try_pop_all_latest();
                if (latest_edsl.has_value())
                {
                    latest_edsl_frame = std::move(latest_edsl.value());
                    got_new_frame = true;
                }

                auto latest_glsl = glsl_to_display.try_pop_all_latest();
                if (latest_glsl.has_value())
                {
                    latest_glsl_frame = std::move(latest_glsl.value());
                    got_new_frame = true;
                }

                auto now = Clock::now();

                if (latest_edsl_frame.has_value() &&
                    latest_edsl_frame->output_image != nullptr &&
                    latest_edsl_frame->executor_ref != nullptr)
                {
                    // 对 EDSL 最新可用帧进行 wait + present。
                    edsl_displayer.wait(*latest_edsl_frame->executor_ref) << *latest_edsl_frame->output_image;
                    scenario->display_tick(latest_edsl_frame.value());
                    if (latest_edsl.has_value())
                    {
                        stats.latest_edsl_frame_id.store(latest_edsl_frame->frame_id, std::memory_order_relaxed);
                        auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                              now - latest_edsl_frame->submit_timestamp)
                                             .count();
                        stats.edsl_latency_us_total.fetch_add(static_cast<uint64_t>(latency_us), std::memory_order_relaxed);
                        stats.edsl_display_frames.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                if (latest_glsl_frame.has_value() &&
                    latest_glsl_frame->output_image != nullptr &&
                    latest_glsl_frame->executor_ref != nullptr)
                {
                    // 对 GLSL 最新可用帧进行 wait + present。
                    glsl_displayer.wait(*latest_glsl_frame->executor_ref) << *latest_glsl_frame->output_image;
                    scenario->display_tick(latest_glsl_frame.value());
                    if (latest_glsl.has_value())
                    {
                        stats.latest_glsl_frame_id.store(latest_glsl_frame->frame_id, std::memory_order_relaxed);
                        auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                              now - latest_glsl_frame->submit_timestamp)
                                             .count();
                        stats.glsl_latency_us_total.fetch_add(static_cast<uint64_t>(latency_us), std::memory_order_relaxed);
                        stats.glsl_display_frames.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                stats.display_loop_ticks.fetch_add(1, std::memory_order_relaxed);
                if (!got_new_frame)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
        catch (const std::exception &e)
        {
            set_error_and_stop("DisplayThread", e.what());
        }
    };

    std::thread render_edsl_worker(render_thread_edsl);
    std::thread render_glsl_worker(render_thread_glsl);
    std::thread display_worker(display_thread);
    std::thread mesh_worker(mesh_thread);

    // 4 主线程负责窗口事件与周期统计输出。
    auto last_stat_tick = Clock::now();
    uint64_t prev_mesh_frames = 0;
    uint64_t prev_edsl_frames = 0;
    uint64_t prev_glsl_frames = 0;
    uint64_t prev_display_ticks = 0;

    while (running.load())
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(windows[0]) || glfwWindowShouldClose(windows[1]))
        {
            request_stop();
            break;
        }

        if (config.enable_compare_stats)
        {
            auto now = Clock::now();
            if (now - last_stat_tick >= std::chrono::seconds(1))
            {
                uint64_t mesh_frames = stats.mesh_frames_produced.load(std::memory_order_relaxed);
                uint64_t edsl_frames = stats.edsl_render_frames.load(std::memory_order_relaxed);
                uint64_t glsl_frames = stats.glsl_render_frames.load(std::memory_order_relaxed);
                uint64_t display_ticks = stats.display_loop_ticks.load(std::memory_order_relaxed);

                uint64_t mesh_fps = mesh_frames - prev_mesh_frames;
                uint64_t edsl_fps = edsl_frames - prev_edsl_frames;
                uint64_t glsl_fps = glsl_frames - prev_glsl_frames;
                uint64_t display_fps = display_ticks - prev_display_ticks;

                prev_mesh_frames = mesh_frames;
                prev_edsl_frames = edsl_frames;
                prev_glsl_frames = glsl_frames;
                prev_display_ticks = display_ticks;

                uint64_t latest_edsl = stats.latest_edsl_frame_id.load(std::memory_order_relaxed);
                uint64_t latest_glsl = stats.latest_glsl_frame_id.load(std::memory_order_relaxed);
                uint64_t frame_gap = (latest_edsl >= latest_glsl) ? (latest_edsl - latest_glsl) : (latest_glsl - latest_edsl);

                uint64_t edsl_displayed = stats.edsl_display_frames.load(std::memory_order_relaxed);
                uint64_t glsl_displayed = stats.glsl_display_frames.load(std::memory_order_relaxed);
                double avg_edsl_latency_ms = (edsl_displayed == 0)
                                                 ? 0.0
                                                 : static_cast<double>(stats.edsl_latency_us_total.load(std::memory_order_relaxed)) / 1000.0 /
                                                       static_cast<double>(edsl_displayed);
                double avg_glsl_latency_ms = (glsl_displayed == 0)
                                                 ? 0.0
                                                 : static_cast<double>(stats.glsl_latency_us_total.load(std::memory_order_relaxed)) / 1000.0 /
                                                       static_cast<double>(glsl_displayed);

                std::cout << "[Stats] meshFPS=" << mesh_fps
                          << " edslRenderFPS=" << edsl_fps
                          << " glslRenderFPS=" << glsl_fps
                          << " displayLoopFPS=" << display_fps
                          << " frameGap=" << frame_gap
                          << " drops(mesh->edsl=" << mesh_to_edsl.dropped_count()
                          << ", mesh->glsl=" << mesh_to_glsl.dropped_count()
                          << ", edsl->display=" << edsl_to_display.dropped_count()
                          << ", glsl->display=" << glsl_to_display.dropped_count() << ")"
                          << " avgLatencyMs(edsl=" << avg_edsl_latency_ms
                          << ", glsl=" << avg_glsl_latency_ms << ")\n";

                last_stat_tick = now;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    request_stop();

    // 5 线程收敛与场景清理。
    if (mesh_worker.joinable())
    {
        mesh_worker.join();
    }
    if (render_edsl_worker.joinable())
    {
        render_edsl_worker.join();
    }
    if (render_glsl_worker.joinable())
    {
        render_glsl_worker.join();
    }
    if (display_worker.joinable())
    {
        display_worker.join();
    }

    scenario->shutdown();
    destroy_windows_and_terminate();

    if (has_error.load())
    {
        std::lock_guard<std::mutex> lock(error_mutex);
        std::cerr << "Fatal error: " << error_message << '\n';
        return -1;
    }

    return 0;
}
