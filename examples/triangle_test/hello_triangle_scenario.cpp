#include "hello_triangle_scenario.h"

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
#include "../scenario_registry.h"

#ifndef HELICON_STRINGIZE_
#define HELICON_STRINGIZE_(X) #X
#endif
#ifndef GLSL
#define GLSL(path) HELICON_STRINGIZE_(path.hpp)
#endif

#include GLSL(ht_vert.glsl)
#include GLSL(ht_frag.glsl)

struct HelloTriangleVertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
};

struct HelloTriangleVertex
{
    std::array<float, 3> position{};
    std::array<float, 3> color{};
};

struct HelloTrianglePayload
{
    std::vector<HelloTriangleVertex> vertices;
};

static std::vector<HelloTriangleVertex> make_triangle_vertices()
{
    return {
        {{{0.0f, -0.55f, 0.0f}}, {{1.0f, 0.25f, 0.25f}}},
        {{{0.58f, 0.45f, 0.0f}}, {{0.2f, 1.0f, 0.3f}}},
        {{{-0.58f, 0.45f, 0.0f}}, {{0.25f, 0.45f, 1.0f}}},
    };
}

static std::vector<HelloTriangleVertex> rotate_vertices_z(const std::vector<HelloTriangleVertex> &source, float radians)
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

struct HelloTriangleScenario::Impl
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

        auto vertex_shader = [&](Aggregate<HelloTriangleVertexAttributeProxy> vertex) -> Float4 {
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

        glsl_rasterizer = std::make_unique<RasterizerPipeline<ht_vert_glsl, ht_frag_glsl>>();
        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        glsl_rasterizer->outColor = mutable_output_image;
        glsl_index_buffer = std::make_unique<HardwareBuffer>(triangle_indices, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;
    std::vector<HelloTriangleVertex> base_vertices;
    Clock::time_point start_time{};
    bool initialized{false};

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<ht_vert_glsl, ht_frag_glsl>> glsl_rasterizer;
    std::unique_ptr<HardwareBuffer> glsl_index_buffer;

    static inline const std::array<uint16_t, 3> triangle_indices = {0, 1, 2};
};

HelloTriangleScenario::HelloTriangleScenario(const RuntimeConfig &config) : impl_(std::make_unique<Impl>(config)) {}

HelloTriangleScenario::~HelloTriangleScenario() = default;

std::string HelloTriangleScenario::name() const
{
    return "hello_triangle";
}

bool HelloTriangleScenario::init(const RuntimeConfig &config,
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

std::shared_ptr<const void> HelloTriangleScenario::mesh_tick(uint64_t frame_id,
                                                             Clock::time_point now,
                                                             std::string &error_message)
{
    (void)frame_id;
    if (!impl_->initialized)
    {
        error_message = "Scenario has not been initialized before mesh_tick.";
        return nullptr;
    }

    const float elapsed_seconds =
        std::chrono::duration<float, std::chrono::seconds::period>(now - impl_->start_time).count();
    const float rotation_radians = elapsed_seconds * 0.75f;

    auto payload = std::make_shared<HelloTrianglePayload>();
    payload->vertices = rotate_vertices_z(impl_->base_vertices, rotation_radians);
    return payload;
}

bool HelloTriangleScenario::render_edsl_tick(const MeshFrame &mesh_frame,
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

    auto payload = std::static_pointer_cast<const HelloTrianglePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);

    executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
             << executor.commit();
    return true;
}

bool HelloTriangleScenario::render_glsl_tick(const MeshFrame &mesh_frame,
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

    auto payload = std::static_pointer_cast<const HelloTrianglePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->glsl_rasterizer->record(*impl_->glsl_index_buffer, vertex_buffer);

    executor << (*impl_->glsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
             << executor.commit();
    return true;
}

void HelloTriangleScenario::display_tick(const RenderFrame &render_frame)
{
    (void)render_frame;
}

void HelloTriangleScenario::shutdown()
{
    impl_->edsl_index_buffer.reset();
    impl_->glsl_index_buffer.reset();
    impl_->edsl_rasterizer.reset();
    impl_->glsl_rasterizer.reset();
    impl_->initialized = false;
}

std::unique_ptr<ScenarioHooks> create_hello_triangle_scenario(const RuntimeConfig &config)
{
    return std::make_unique<HelloTriangleScenario>(config);
}

void register_hello_triangle_scenario()
{
    static const bool registered = [] {
        register_scenario("hello_triangle", create_hello_triangle_scenario);
        return true;
    }();
    (void)registered;
}
