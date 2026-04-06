#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace triangle_test_data
{
    // Vertex layout follows shader inputs: vec3 position + vec3 color.
    struct Vertex
    {
        std::array<float, 3> position; // layout(location = 0) in vec3 inPosition;
        std::array<float, 3> color;    // layout(location = 1) in vec3 inColor;
    };

    // Triangle vertex data (3 vertices, 1 triangle).
    inline const std::vector<Vertex> kTriangleVertices =
    {
        {{0.0f, -0.55f, 0.0f}, {1.0f, 0.25f, 0.25f}}, // 0
        {{0.58f, 0.45f, 0.0f}, {0.2f, 1.0f, 0.3f}},   // 1
        {{-0.58f, 0.45f, 0.0f}, {0.25f, 0.45f, 1.0f}} // 2
    };

    // Triangle index data.
    inline const std::vector<uint16_t> kTriangleIndices = {0, 1, 2};
} // namespace triangle_test_data
