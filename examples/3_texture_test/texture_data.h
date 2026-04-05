#pragma once

#include <array>
#include <cstdint>
#include <vector>

// Vertex layout follows shader inputs: vec3 position + vec3 color + vec2 texCoord.
struct Vertex
{
    std::array<float, 3> position; // layout(location = 0) in vec3 inPosition;
    std::array<float, 3> color;    // layout(location = 1) in vec3 inColor;
    std::array<float, 2> texCoord; // layout(location = 2) in vec2 inTexCoord;
};

// Quad vertex data (4 vertices, 2 triangles).
inline const std::vector<Vertex> vertices =
{
    {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},    // top right
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // bottom right
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},  // bottom left
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}    // top left
};

// Quad index data.
inline const std::vector<uint16_t> indices =
{
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};
