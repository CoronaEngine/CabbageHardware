#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace texture_test_data
{
    // Keep texture demo mesh data namespaced to avoid ODR conflicts with other scenarios.
    struct Vertex
    {
        std::array<float, 3> position;
        std::array<float, 2> texCoord;
    };

    inline const std::vector<Vertex> kQuadVertices =
    {
        {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},   // top right
        {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},  // bottom right
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}}, // bottom left
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}}   // top left
    };

    inline const std::vector<uint16_t> kQuadIndices =
    {
        0, 1, 3,
        1, 2, 3
    };
} // namespace texture_test_data
