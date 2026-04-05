#include "texture_scenario.h"

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
#include "texture_data.h"
#include "../common/asset_utils.h"
#include "../common/texture_loader.h"
#include "../scenario_registry.h"

#ifndef HELICON_STRINGIZE_
#define HELICON_STRINGIZE_(X) #X
#endif
#ifndef GLSL
#define GLSL(path) HELICON_STRINGIZE_(path.hpp)
#endif

#include GLSL(texture_vert.glsl)
#include GLSL(texture_frag.glsl)

struct TextureVertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
    EmbeddedShader::Float2 tex_coord;
};

struct TexturePayload
{
    std::vector<Vertex> vertices;
};

static std::vector<Vertex> rotate_vertices_z(const std::vector<Vertex> &source, float radians)
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

struct TextureScenario::Impl
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

        auto vertex_shader = [&](Aggregate<TextureVertexAttributeProxy> vertex) -> Float4 {
            position() = Float4(vertex->position, 1.0f);
            return Float4(vertex->color, 1.0f);
        };

        auto fragment_shader = [&](Float4 interpolated_color) -> Float4 {
            return interpolated_color;
        };

        edsl_rasterizer = std::make_unique<RasterizerPipeline<>>(vertex_shader, fragment_shader);
        edsl_rasterizer->bindOutputTargets(edsl_output);
        edsl_index_buffer = std::make_unique<HardwareBuffer>(index_data, BufferUsage::IndexBuffer);
    }

    void ensure_glsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (glsl_rasterizer && glsl_index_buffer)
        {
            return;
        }

        glsl_rasterizer = std::make_unique<RasterizerPipeline<texture_vert_glsl, texture_frag_glsl>>();
        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        glsl_rasterizer->FragColor = mutable_output_image;
        glsl_index_buffer = std::make_unique<HardwareBuffer>(index_data, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;
    std::vector<Vertex> base_vertices;
    std::vector<uint16_t> index_data;
    HardwareImage texture_image;
    uint32_t texture_descriptor = 0;
    Clock::time_point start_time{};
    bool initialized = false;

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<texture_vert_glsl, texture_frag_glsl>> glsl_rasterizer;
    std::unique_ptr<HardwareBuffer> glsl_index_buffer;
};

TextureScenario::TextureScenario(const RuntimeConfig &config) : impl_(std::make_unique<Impl>(config)) {}

TextureScenario::~TextureScenario() = default;

std::string TextureScenario::name() const
{
    return "texture";
}

bool TextureScenario::init(const RuntimeConfig &config,
                           const std::array<HardwareImage, 2> &outputs,
                           std::string &error_message)
{
    impl_->config = config;
    impl_->base_vertices = vertices;
    impl_->index_data = indices;

    const auto texture_path = examples::common::resolve_examples_asset("3_texture_test/container.jpg");
    auto texture_result = examples::common::load_texture_rgba8_srgb(texture_path, true, error_message);
    if (!texture_result.success)
    {
        return false;
    }

    impl_->texture_image = std::move(texture_result.texture);
    impl_->texture_descriptor = texture_result.descriptor_id;

    impl_->ensure_edsl_pipeline(outputs[0]);
    impl_->ensure_glsl_pipeline(outputs[1]);

    impl_->start_time = Clock::now();
    impl_->initialized = true;
    return true;
}

std::shared_ptr<const void> TextureScenario::mesh_tick(uint64_t frame_id,
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
    const float rotation_radians = elapsed_seconds * 0.5f;

    auto payload = std::make_shared<TexturePayload>();
    payload->vertices = rotate_vertices_z(impl_->base_vertices, rotation_radians);
    return payload;
}

bool TextureScenario::render_edsl_tick(const MeshFrame &mesh_frame,
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

    auto payload = std::static_pointer_cast<const TexturePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);

    executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width), static_cast<uint16_t>(impl_->config.window_height))
             << executor.commit();
    return true;
}

bool TextureScenario::render_glsl_tick(const MeshFrame &mesh_frame,
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

    auto payload = std::static_pointer_cast<const TexturePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->glsl_rasterizer->record(*impl_->glsl_index_buffer, vertex_buffer);

    executor << (*impl_->glsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width), static_cast<uint16_t>(impl_->config.window_height))
             << executor.commit();
    return true;
}

void TextureScenario::display_tick(const RenderFrame &render_frame)
{
    (void)render_frame;
}

void TextureScenario::shutdown()
{
    impl_->edsl_index_buffer.reset();
    impl_->glsl_index_buffer.reset();
    impl_->edsl_rasterizer.reset();
    impl_->glsl_rasterizer.reset();
    impl_->texture_image = HardwareImage();
    impl_->texture_descriptor = 0;
    impl_->initialized = false;
}

std::unique_ptr<ScenarioHooks> create_texture_scenario(const RuntimeConfig &config)
{
    return std::make_unique<TextureScenario>(config);
}

void register_texture_scenario()
{
    static const bool registered = [] {
        register_scenario("texture", create_texture_scenario);
        return true;
    }();
    (void)registered;
}
