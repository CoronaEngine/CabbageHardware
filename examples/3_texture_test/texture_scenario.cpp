#include "texture_scenario.h"

#include <array>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <stb_image.h>

#include "Codegen/BuiltinVariate.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"
#include "texture_data.h"
#include "../common/asset_utils.h"
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
    EmbeddedShader::Float2 tex_coord;
};

struct TextureEdslVertex
{
    std::array<float, 3> position{};
    std::array<float, 2> texCoord{};
};

struct TexturePayload
{
    std::vector<TextureEdslVertex> edsl_vertices;
    std::vector<TextureEdslVertex> glsl_vertices;
};

template <typename TVertex>
static std::vector<TVertex> rotate_vertices_z(const std::vector<TVertex> &source, float radians)
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

static std::vector<TextureEdslVertex> make_edsl_vertices(const std::vector<texture_test_data::Vertex> &source)
{
    std::vector<TextureEdslVertex> result;
    result.reserve(source.size());
    for (const auto &vertex : source)
    {
        result.push_back({vertex.position, vertex.texCoord});
    }
    return result;
}

struct TextureScenario::Impl
{
    explicit Impl(RuntimeConfig cfg) : config(std::move(cfg)) {}

    bool ensure_edsl_pipeline(const HardwareImage &output_image, std::string &error_message)
    {
        std::lock_guard<std::mutex> lock(edsl_mutex);
        if (!texture_image)
        {
            error_message = "Texture image is not initialized for EDSL pipeline.";
            return false;
        }

        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        edsl_output = mutable_output_image;
        edsl_texture = texture_image;

        if (edsl_rasterizer && edsl_index_buffer)
        {
            return true;
        }

        using namespace EmbeddedShader;
        using namespace EmbeddedShader::Ast;

        auto vertex_shader = [&](Aggregate<TextureVertexAttributeProxy> vertex) -> Float2 {
            position() = Float4(vertex->position, 1.0f);
            return vertex->tex_coord;
        };

        auto fragment_shader = [&](Float2 interpolated_tex_coord) -> Float4 {
            Float2 uv = clamp(interpolated_tex_coord, ktm::fvec2(0.0f), ktm::fvec2(1.0f));
            return texture(edsl_texture, uv);
        };

        edsl_rasterizer = std::make_unique<RasterizerPipeline<>>(vertex_shader, fragment_shader);
        edsl_rasterizer->bindOutputTargets(edsl_output);
        edsl_index_buffer = std::make_unique<HardwareBuffer>(index_data, BufferUsage::IndexBuffer);
        return true;
    }

    bool ensure_glsl_pipeline(const HardwareImage &output_image, std::string &error_message)
    {
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (!texture_image)
        {
            error_message = "Texture image is not initialized for GLSL pipeline.";
            return false;
        }

        if (!glsl_rasterizer || !glsl_index_buffer)
        {
            glsl_rasterizer = std::make_unique<RasterizerPipeline<texture_vert_glsl, texture_frag_glsl>>();
            glsl_index_buffer = std::make_unique<HardwareBuffer>(index_data, BufferUsage::IndexBuffer);
        }

        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        glsl_rasterizer->FragColor = mutable_output_image;
        if constexpr (requires { texture_frag_glsl::texture1; })
        {
            (*glsl_rasterizer)[texture_frag_glsl::texture1] = texture_image;
        }
        else
        {
            error_message = "Generated shader bindings are missing texture1. Rebuild shader headers.";
            return false;
        }
        return true;
    }

    RuntimeConfig config;
    std::vector<TextureEdslVertex> base_vertices;
    std::vector<TextureEdslVertex> base_edsl_vertices;
    std::vector<uint16_t> index_data;
    HardwareImage texture_image;
    Clock::time_point start_time{};
    bool initialized = false;

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_texture;
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
                           Backend backend,
                           const HardwareImage &output,
                           std::string &error_message)
{
    impl_->config = config;
    impl_->base_vertices = make_edsl_vertices(texture_test_data::kQuadVertices);
    impl_->base_edsl_vertices = impl_->base_vertices;
    impl_->index_data = texture_test_data::kQuadIndices;

    const auto texture_path = resolve_examples_asset("3_texture_test/container.jpg");
    stbi_set_flip_vertically_on_load(1);
    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;
    stbi_uc *texture_pixels = stbi_load(texture_path.string().c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if (texture_pixels == nullptr)
    {
        error_message = "Failed to load texture pixels.";
        return false;
    }
    if (texture_width <= 0 || texture_height <= 0)
    {
        stbi_image_free(texture_pixels);
        error_message = "Loaded texture has invalid dimensions.";
        return false;
    }

    HardwareImageCreateInfo texture_info;
    texture_info.width = static_cast<uint32_t>(texture_width);
    texture_info.height = static_cast<uint32_t>(texture_height);
    texture_info.format = ImageFormat::RGBA8_SRGB;
    texture_info.usage = ImageUsage::SampledImage;
    texture_info.arrayLayers = 1;
    texture_info.mipLevels = 1;
    impl_->texture_image = HardwareImage(texture_info);
    if (!impl_->texture_image)
    {
        stbi_image_free(texture_pixels);
        error_message = "Failed to create sampled texture image.";
        return false;
    }

    HardwareExecutor upload_executor;
    upload_executor << impl_->texture_image.copyFrom(texture_pixels)
                    << upload_executor.commit();
    stbi_image_free(texture_pixels);

    if (backend == Backend::EDSL)
    {
        if (!impl_->ensure_edsl_pipeline(output, error_message))
        {
            return false;
        }
    }
    else
    {
        if (!impl_->ensure_glsl_pipeline(output, error_message))
        {
            return false;
        }
    }

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
    payload->edsl_vertices = rotate_vertices_z(impl_->base_edsl_vertices, rotation_radians);
    payload->glsl_vertices = payload->edsl_vertices;
    return payload;
}

bool TextureScenario::render_tick(const MeshFrame &mesh_frame,
                                  Backend backend,
                                  HardwareExecutor &executor,
                                  const HardwareImage &output_image,
                                  std::string &error_message)
{
    if (!mesh_frame.payload)
    {
        error_message = "render_tick received an empty mesh payload.";
        return false;
    }
    if (!impl_->texture_image)
    {
        error_message = "render_tick cannot run without a valid texture image.";
        return false;
    }

    auto payload = std::static_pointer_cast<const TexturePayload>(mesh_frame.payload);
    if (backend == Backend::EDSL)
    {
        if (!impl_->ensure_edsl_pipeline(output_image, error_message))
        {
            return false;
        }
        HardwareBuffer vertex_buffer(payload->edsl_vertices, BufferUsage::VertexBuffer);
        impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);
        executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width), static_cast<uint16_t>(impl_->config.window_height))
                 << executor.commit();
        return true;
    }

    if (!impl_->ensure_glsl_pipeline(output_image, error_message))
    {
        return false;
    }
    HardwareBuffer vertex_buffer(payload->glsl_vertices, BufferUsage::VertexBuffer);
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
