#include "triangle_scenario.h"

#include <array>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "Codegen/BuiltinVariate.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"
#include "triangle_data.h"
#include "../scenario_registry.h"

#ifndef HELICON_STRINGIZE_
#define HELICON_STRINGIZE_(X) #X
#endif
#ifndef GLSL
#define GLSL(path) HELICON_STRINGIZE_(path.hpp)
#endif

#include GLSL(triangle_vert.glsl)
#include GLSL(triangle_frag.glsl)

struct TriangleVertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
};

struct TriangleVertex
{
    std::array<float, 3> position{};
    std::array<float, 3> color{};
};

struct TrianglePayload
{
    std::vector<TriangleVertex> vertices;
};

static std::vector<TriangleVertex> make_triangle_vertices()
{
    std::vector<TriangleVertex> result;
    result.reserve(vertices.size());
    for (const auto &vertex : vertices)
    {
        result.push_back({vertex.position, vertex.color});
    }
    return result;
}

static std::vector<TriangleVertex> rotate_vertices_z(const std::vector<TriangleVertex> &source, float radians)
{
    const float cos_v = std::cos(radians);
    const float sin_v = std::sin(radians);

    auto rotated = source;
    for (auto &vertex : rotated)
    {
        const float x = vertex.position[0];
        const float y = vertex.position[1];
        vertex.position[0] = x * cos_v - y * sin_v;
        vertex.position[1] = x * sin_v + y * cos_v;
    }
    return rotated;
}

struct TriangleScenario::Impl
{
    explicit Impl(RuntimeConfig cfg) : config(std::move(cfg)) {}

    void ensure_edsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(edsl_mutex);
        if (edsl_rasterizer && edsl_index_buffer)
        {
            return;
        }

        using namespace EmbeddedShader;
        using namespace EmbeddedShader::Ast;

        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        edsl_output = mutable_output_image;

        auto vertex_shader = [&](Aggregate<TriangleVertexAttributeProxy> vertex) -> Float4 {
            position() = Float4(vertex->position, 1.0f);
            return Float4(vertex->color, 1.0f);
        };

        auto fragment_shader = [&](Float4 interpolated_color) -> Float4 {
            return interpolated_color;
        };

        edsl_rasterizer = std::make_unique<RasterizerPipeline<>>(vertex_shader, fragment_shader);
        edsl_rasterizer->bindOutputTargets(edsl_output);
        edsl_index_buffer = std::make_unique<HardwareBuffer>(triangle_indices, BufferUsage::IndexBuffer);
    }

    void ensure_glsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (glsl_rasterizer && glsl_index_buffer)
        {
            return;
        }

        glsl_rasterizer = std::make_unique<RasterizerPipeline<triangle_vert_glsl, triangle_frag_glsl>>();
        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        glsl_rasterizer->outColor = mutable_output_image;
        glsl_index_buffer = std::make_unique<HardwareBuffer>(triangle_indices, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;
    std::vector<TriangleVertex> base_vertices;
    Clock::time_point start_time{};
    bool initialized{false};

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<triangle_vert_glsl, triangle_frag_glsl>> glsl_rasterizer;
    std::unique_ptr<HardwareBuffer> glsl_index_buffer;

    static inline const std::array<uint16_t, 3> triangle_indices = {0, 1, 2};
};

TriangleScenario::TriangleScenario(const RuntimeConfig &config) : impl_(std::make_unique<Impl>(config)) {}

TriangleScenario::~TriangleScenario() = default;

std::string TriangleScenario::name() const
{
    return "hello_triangle";
}

bool TriangleScenario::init(const RuntimeConfig &config,
                            const std::array<HardwareImage, 2> &outputs,
                            std::string &error_message)
{
    (void)error_message;

    impl_->config = config;
    impl_->base_vertices = make_triangle_vertices();
    impl_->ensure_edsl_pipeline(outputs[0]);
    impl_->ensure_glsl_pipeline(outputs[1]);
    impl_->start_time = Clock::now();
    impl_->initialized = true;
    return true;
}

std::shared_ptr<const void> TriangleScenario::mesh_tick(uint64_t frame_id,
                                                        Clock::time_point now,
                                                        std::string &error_message)
{
    (void)frame_id;
    if (!impl_->initialized)
    {
        error_message = "Scenario has not been initialized before mesh_tick.";
        return nullptr;
    }

    const float elapsed_seconds = std::chrono::duration<float, std::chrono::seconds::period>(now - impl_->start_time).count();
    const float rotation_radians = elapsed_seconds * 0.75f;

    auto payload = std::make_shared<TrianglePayload>();
    payload->vertices = rotate_vertices_z(impl_->base_vertices, rotation_radians);
    return payload;
}

bool TriangleScenario::render_edsl_tick(const MeshFrame &mesh_frame,
                                        HardwareExecutor &executor,
                                        const HardwareImage &output_image,
                                        std::string &error_message)
{
    if (!mesh_frame.payload)
    {
        error_message = "render_edsl_tick received an empty mesh payload.";
        return false;
    }
    if (!impl_->edsl_rasterizer || !impl_->edsl_index_buffer)
    {
        impl_->ensure_edsl_pipeline(output_image);
    }

    auto payload = std::static_pointer_cast<const TrianglePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);

    executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width), static_cast<uint16_t>(impl_->config.window_height))
             << executor.commit();
    return true;
}

bool TriangleScenario::render_glsl_tick(const MeshFrame &mesh_frame,
                                             HardwareExecutor &executor,
                                             const HardwareImage &output_image,
                                             std::string &error_message)
{
    if (!mesh_frame.payload)
    {
        error_message = "render_glsl_tick received an empty mesh payload.";
        return false;
    }
    if (!impl_->glsl_rasterizer || !impl_->glsl_index_buffer)
    {
        impl_->ensure_glsl_pipeline(output_image);
    }

    auto payload = std::static_pointer_cast<const TrianglePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->glsl_rasterizer->record(*impl_->glsl_index_buffer, vertex_buffer);

    executor << (*impl_->glsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width), static_cast<uint16_t>(impl_->config.window_height))
             << executor.commit();
    return true;
}

void TriangleScenario::display_tick(const RenderFrame &render_frame)
{
    (void)render_frame;
}

void TriangleScenario::shutdown()
{
    impl_->edsl_index_buffer.reset();
    impl_->glsl_index_buffer.reset();
    impl_->edsl_rasterizer.reset();
    impl_->glsl_rasterizer.reset();
    impl_->initialized = false;
}

std::unique_ptr<ScenarioHooks> create_triangle_scenario(const RuntimeConfig &config)
{
    return std::make_unique<TriangleScenario>(config);
}

void register_triangle_scenario()
{
    static const bool registered = [] {
        register_scenario("triangle", create_triangle_scenario);
        return true;
    }();
    (void)registered;
}
