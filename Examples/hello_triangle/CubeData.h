#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include <ktm/ktm.h>

#include "../CubeData.h"

namespace hello_triangle
{
struct SimpleVertex
{
    std::array<float, 3> position;
    std::array<float, 3> color;
};

inline std::vector<SimpleVertex> makeSimpleVerticesFromCubeData()
{
    std::vector<SimpleVertex> simpleVertices(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        simpleVertices[i].position = vertices[i].position;
        simpleVertices[i].color = vertices[i].color;
    }
    return simpleVertices;
}

inline std::vector<ktm::fmat4x4> makeBaseModelMatrices(size_t objectCount = 20)
{
    std::vector<ktm::fmat4x4> baseModelMat(objectCount);
    for (size_t i = 0; i < objectCount; ++i)
    {
        baseModelMat[i] = ktm::translate3d(
                              ktm::fvec3(static_cast<float>(i % 5) - 2.0f,
                                         static_cast<float>(i / 5) - 0.5f,
                                         0.0f))
            * ktm::scale3d(ktm::fvec3(0.1f, 0.1f, 0.1f))
            * ktm::rotate3d_axis(ktm::radians(static_cast<float>(i) * 30.0f),
                                 ktm::fvec3(0.0f, 0.0f, 1.0f));
    }
    return baseModelMat;
}

inline ktm::fmat4x4 makeAnimatedModelMatrix(const ktm::fmat4x4& base, float t)
{
    return base * ktm::rotate3d_axis(t * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
}

inline std::vector<SimpleVertex> transformVerticesForObject(const std::vector<SimpleVertex>& src,
                                                             const ktm::fmat4x4& mvp)
{
    auto dst = src;
    for (auto& v : dst)
    {
        ktm::fvec4 clip = mvp * ktm::fvec4(v.position[0], v.position[1], v.position[2], 1.0f);
        float invW = 1.0f / clip[3];
        v.position = {clip[0] * invW, clip[1] * invW, clip[2] * invW};
    }
    return dst;
}
} // namespace hello_triangle
