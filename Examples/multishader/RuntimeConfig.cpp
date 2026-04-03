#include "RuntimeConfig.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>

namespace multishader
{
namespace
{
template <typename UIntType>
UIntType parseUnsignedValue(const std::string &value, const char *fieldName)
{
    // 严格解析无符号整数：只要有尾随字符就判定为非法输入。
    UIntType parsed = 0;
    const char *begin = value.data();
    const char *end = value.data() + value.size();
    auto [ptr, ec] = std::from_chars(begin, end, parsed);
    if (ec != std::errc() || ptr != end)
    {
        throw std::runtime_error("Invalid value for " + std::string(fieldName) + ": " + value);
    }
    return parsed;
}

std::string readValue(int &index, int argc, char **argv, const std::string &argument)
{
    // 同时支持 --key=value 和 --key value 两种写法。
    auto equalPos = argument.find('=');
    if (equalPos != std::string::npos)
    {
        return argument.substr(equalPos + 1);
    }

    if (index + 1 >= argc)
    {
        throw std::runtime_error("Missing value after argument: " + argument);
    }
    ++index;
    return argv[index];
}

bool parseBoolValue(const std::string &value)
{
    // 兼容常见布尔写法，降低命令行使用成本。
    std::string lowered = value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (lowered == "1" || lowered == "true" || lowered == "yes" || lowered == "on")
    {
        return true;
    }
    if (lowered == "0" || lowered == "false" || lowered == "no" || lowered == "off")
    {
        return false;
    }
    throw std::runtime_error("Invalid boolean value: " + value);
}
} // 匿名命名空间

RuntimeConfig parseRuntimeConfig(int argc, char **argv)
{
    RuntimeConfig config;

    // 只接受已定义的参数，遇到未知参数立即报错。
    for (int i = 1; i < argc; ++i)
    {
        std::string argument = argv[i];
        if (argument == "--help" || argument == "-h")
        {
            config.showHelp = true;
            continue;
        }
        if (argument.rfind("--scenario", 0) == 0)
        {
            config.scenario = readValue(i, argc, argv, argument);
            continue;
        }
        if (argument.rfind("--dataset", 0) == 0)
        {
            config.dataset = readValue(i, argc, argv, argument);
            continue;
        }
        if (argument.rfind("--queue-depth", 0) == 0)
        {
            config.queueDepth = parseUnsignedValue<std::size_t>(readValue(i, argc, argv, argument), "queue-depth");
            continue;
        }
        if (argument.rfind("--width", 0) == 0)
        {
            config.windowWidth = parseUnsignedValue<uint32_t>(readValue(i, argc, argv, argument), "width");
            continue;
        }
        if (argument.rfind("--height", 0) == 0)
        {
            config.windowHeight = parseUnsignedValue<uint32_t>(readValue(i, argc, argv, argument), "height");
            continue;
        }
        if (argument.rfind("--max-fps", 0) == 0)
        {
            config.maxFps = parseUnsignedValue<uint32_t>(readValue(i, argc, argv, argument), "max-fps");
            continue;
        }
        if (argument.rfind("--compare-stats", 0) == 0)
        {
            config.enableCompareStats = parseBoolValue(readValue(i, argc, argv, argument));
            continue;
        }
        if (argument.rfind("--glsl-delay-ms", 0) == 0)
        {
            config.glslArtificialDelayMs = parseUnsignedValue<uint32_t>(readValue(i, argc, argv, argument), "glsl-delay-ms");
            continue;
        }

        throw std::runtime_error("Unknown argument: " + argument);
    }

    // 做最小值归一化，保证运行阶段不出现 0 尺寸或 0 容量。
    config.queueDepth = std::max<std::size_t>(1, config.queueDepth);
    config.windowWidth = std::max<uint32_t>(1, config.windowWidth);
    config.windowHeight = std::max<uint32_t>(1, config.windowHeight);
    return config;
}

std::string runtimeConfigUsage(std::string_view exeName)
{
    std::string usage;
    usage += "Usage: ";
    usage += exeName;
    usage += " [options]\n";
    usage += "  --scenario=<name>         Scenario name (default: default)\n";
    usage += "  --dataset=<name>          Dataset name (default: cube)\n";
    usage += "  --queue-depth=<n>         Bounded queue depth (default: 3)\n";
    usage += "  --width=<n>               Window width (default: 1920)\n";
    usage += "  --height=<n>              Window height (default: 1080)\n";
    usage += "  --max-fps=<n>             Mesh thread FPS cap, 0 uncapped (default: 0)\n";
    usage += "  --compare-stats=<bool>    Enable periodic compare stats (default: true)\n";
    usage += "  --glsl-delay-ms=<n>       Artificial delay in GLSL render thread (default: 0)\n";
    usage += "  --help                    Show this help message\n";
    return usage;
}
} // multishader 命名空间
