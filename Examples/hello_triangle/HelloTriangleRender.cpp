#include "HelloTriangleRender.h"

#include <chrono>

#include "Codegen/BuiltinVariate.h"
#include "Codegen/ControlFlows.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"

#include GLSL(ht_vert.glsl)
#include GLSL(ht_frag.glsl)

namespace hello_triangle
{
namespace
{
struct VertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
};
} // namespace

void render_edsl(uint32_t threadIndex, const RenderContext& ctx)
{
    using namespace EmbeddedShader;
    using namespace EmbeddedShader::Ast;

    Texture2D<ktm::fvec4> outColor = ctx.finalOutputImages[threadIndex];

    auto vsLambda = [&](Aggregate<VertexAttributeProxy> vertex) -> Float4 {
        position() = Float4(vertex->position, 1.0f);
        return Float4(vertex->color, 1.0f);
    };

    auto fsLambda = [&](Float4 interpolatedColor) -> Float4 {
        return interpolatedColor;
    };

    RasterizerPipeline rasterizer(vsLambda, fsLambda);
    rasterizer.bindOutputTargets(outColor);

    auto startTime = std::chrono::high_resolution_clock::now();
    while (ctx.running.load())
    {
        float currentTime = std::chrono::duration<float, std::chrono::seconds::period>(
                                std::chrono::high_resolution_clock::now() - startTime)
                                .count();

        HardwareBuffer indexBuffer(indices, BufferUsage::IndexBuffer);
        for (size_t i = 0; i < ctx.rasterizerStorageBuffers[threadIndex].size(); ++i)
        {
            auto model = makeAnimatedModelMatrix(ctx.baseModelMat[i], currentTime);
            auto transformed = transformVerticesForObject(ctx.simpleVertices, ctx.vpMat * model);
            HardwareBuffer vertexBuffer(transformed, BufferUsage::VertexBuffer);
            rasterizer.record(indexBuffer, vertexBuffer);
        }

        ctx.executors[threadIndex] << rasterizer(1920, 1080)
                                   << ctx.executors[threadIndex].commit();
    }
}

void render_glsl(uint32_t threadIndex, const RenderContext& ctx)
{
    RasterizerPipeline<ht_vert_glsl, ht_frag_glsl> rasterizer;
    rasterizer.outColor = ctx.finalOutputImages[threadIndex];

    auto startTime = std::chrono::high_resolution_clock::now();
    while (ctx.running.load())
    {
        float currentTime = std::chrono::duration<float, std::chrono::seconds::period>(
                                std::chrono::high_resolution_clock::now() - startTime)
                                .count();

        HardwareBuffer indexBuffer(indices, BufferUsage::IndexBuffer);
        for (size_t i = 0; i < ctx.rasterizerStorageBuffers[threadIndex].size(); ++i)
        {
            auto model = makeAnimatedModelMatrix(ctx.baseModelMat[i], currentTime);
            auto transformed = transformVerticesForObject(ctx.simpleVertices, ctx.vpMat * model);
            HardwareBuffer vertexBuffer(transformed, BufferUsage::VertexBuffer);
            rasterizer.record(indexBuffer, vertexBuffer);
        }

        ctx.executors[threadIndex] << rasterizer(1920, 1080)
                                   << ctx.executors[threadIndex].commit();
    }
}

} // namespace hello_triangle
