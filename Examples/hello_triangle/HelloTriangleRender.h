#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include <ktm/ktm.h>

#include "CabbageHardware.h"
#include "CubeData.h"

namespace hello_triangle
{
struct RenderContext
{
    std::atomic_bool& running;
    std::vector<HardwareImage>& finalOutputImages;
    std::vector<HardwareExecutor>& executors;
    std::vector<std::vector<HardwareBuffer>>& rasterizerStorageBuffers;
    const std::vector<SimpleVertex>& simpleVertices;
    const std::vector<ktm::fmat4x4>& baseModelMat;
    const ktm::fmat4x4& vpMat;
};

void render_edsl(uint32_t threadIndex, const RenderContext& ctx);
void render_glsl(uint32_t threadIndex, const RenderContext& ctx);

} // namespace hello_triangle
