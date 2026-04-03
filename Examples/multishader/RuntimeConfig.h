#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace multishader
{
// 运行时配置：用于选择场景/数据集，并控制队列与线程节奏。
struct RuntimeConfig
{
    std::string scenario = "default"; // 场景注册名。
    std::string dataset = "cube"; // 数据集名，由场景自行解释。
    std::size_t queueDepth = 3; // 四条有界队列的容量。
    uint32_t windowWidth = 1920; // EDSL/GLSL 输出窗口宽度。
    uint32_t windowHeight = 1080; // EDSL/GLSL 输出窗口高度。
    uint32_t maxFps = 0; // Mesh 线程帧率上限，0 表示不限制。
    bool enableCompareStats = true; // 是否周期打印对比统计信息。
    uint32_t glslArtificialDelayMs = 0; // GLSL 线程人工延迟（用于背压测试）。
    bool showHelp = false; // 是否打印帮助信息（--help / -h）。
};

// 解析命令行参数并返回 RuntimeConfig，同时做基础合法性归一化。
RuntimeConfig parseRuntimeConfig(int argc, char **argv);

// 生成运行参数帮助文本。
std::string runtimeConfigUsage(std::string_view exeName);
} // multishader 命名空间
