#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <ktm/ktm.h>

constexpr bool ENABLE_RESOURCE_TRACKING = true;
constexpr bool ENABLE_VERBOSE_LOGGING = true;

// 调试日志宏
#define LOG_INFO(msg)                               \
    if (ENABLE_VERBOSE_LOGGING)                     \
    {                                               \
        std::cout << "[INFO] " << msg << std::endl; \
    }
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl;
#define LOG_RESOURCE(action, type)                                        \
    if (ENABLE_RESOURCE_TRACKING)                                         \
    {                                                                     \
        std::cout << "[RESOURCE] " << action << " " << type << std::endl; \
    }

// 着色器路径解析
inline std::string resolveShaderPath()
{
    std::string runtimePath = std::filesystem::current_path().string();
    std::regex pattern(R"((.*)CabbageHardware\b)");
    std::smatch matches;

    if (std::regex_search(runtimePath, matches, pattern) && matches.size() > 1)
    {
        std::string resultPath = matches[1].str() + "CabbageHardware";
        std::replace(resultPath.begin(), resultPath.end(), '\\', '/');
        return resultPath + "/Examples";
    }

    throw std::runtime_error("Failed to resolve shader path.");
}

inline const std::string shaderPath = resolveShaderPath();

// 文件读取工具
inline std::string readStringFile(const std::string_view file_path)
{
    std::ifstream file(file_path.data());
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + std::string(file_path));
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Uniform Buffer 结构
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

// 渲染配置常量
namespace RenderConfig
{
    constexpr int WINDOW_WIDTH = 1920;
    constexpr int WINDOW_HEIGHT = 1080;
    constexpr int WINDOW_COUNT = 1;
    constexpr int CUBE_COUNT = 20;
    constexpr int COMPUTE_GROUP_SIZE = 8;
    constexpr int FRAME_AVERAGE_COUNT = 100;
}