#include "texture_scenario.h"

#include <array>
#include <cmath>
#include <filesystem>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Codegen/BuiltinVariate.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"
#include "texture_data.h"
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

struct TextureVertex
{
    std::array<float, 3> position{};
    std::array<float, 3> color{};
    std::array<float, 2> tex_coord{};
};

struct TexturePayload
{
    std::vector<TextureVertex> vertices;
};

static std::filesystem::path resolve_texture_path()
{
#ifdef HELICON_ROOT_PATH
    return std::filesystem::path(HELICON_ROOT_PATH) / "examples" / "3_texture_test" / "container.jpg";
#else
    return std::filesystem::current_path() / "examples" / "3_texture_test" / "container.jpg";
#endif
}

static bool load_texture_image(const std::filesystem::path &texture_path, HardwareImage &image, std::string &error_message)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(1);
    stbi_uc *pixels = stbi_load(texture_path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr)
    {
        std::ostringstream oss;
        oss << "Failed to load texture: " << texture_path.string();
        if (const char *reason = stbi_failure_reason())
        {
            oss << ", reason: " << reason;
        }
        error_message = oss.str();
        return false;
    }

    image = HardwareImage(static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height),
                          ImageFormat::RGBA8_SRGB,
                          ImageUsage::SampledImage,
                          1,
                          pixels);
    stbi_image_free(pixels);

    if (!image)
    {
        error_message = "Failed to create HardwareImage for texture.";
        return false;
    }
    return true;
}

static std::vector<TextureVertex> make_quad_vertices()
{
    constexpr std::size_t kVertexStride = 8;
    const std::size_t scalar_count = sizeof(vertices) / sizeof(vertices[0]);
    const std::size_t vertex_count = scalar_count / kVertexStride;

    std::vector<TextureVertex> result;
    result.reserve(vertex_count);
    for (std::size_t i = 0; i < vertex_count; ++i)
    {
        const std::size_t base = i * kVertexStride;
        result.push_back({
            {vertices[base + 0], vertices[base + 1], vertices[base + 2]},
            {vertices[base + 3], vertices[base + 4], vertices[base + 5]},
            {vertices[base + 6], vertices[base + 7]},
        });
    }
    return result;
}

static std::vector<uint16_t> make_quad_indices()
{
    const std::size_t index_count = sizeof(indices) / sizeof(indices[0]);
    std::vector<uint16_t> result;
    result.reserve(index_count);
    for (std::size_t i = 0; i < index_count; ++i)
    {
        result.push_back(static_cast<uint16_t>(indices[i]));
    }
    return result;
}

static std::vector<TextureVertex> rotate_vertices_z(const std::vector<TextureVertex> &source, float radians)
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

    bool ensure_edsl_pipeline(const HardwareImage &output_image, std::string &error_message)
    {
        (void)error_message;
        std::lock_guard<std::mutex> lock(edsl_mutex);
        if (edsl_rasterizer && edsl_index_buffer)
        {
            return true;
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
        return true;
    }

    bool ensure_glsl_pipeline(const HardwareImage &output_image, std::string &error_message)
    {
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (glsl_rasterizer && glsl_index_buffer)
        {
            return true;
        }
        if (!texture_image)
        {
            error_message = "Texture image is not initialized for GLSL pipeline.";
            return false;
        }

        glsl_rasterizer = std::make_unique<RasterizerPipeline<>>(texture_vert_glsl::spirv, texture_frag_glsl::spirv);
        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        (*glsl_rasterizer)[texture_frag_glsl::FragColor] = mutable_output_image;
        glsl_index_buffer = std::make_unique<HardwareBuffer>(index_data, BufferUsage::IndexBuffer);
        return true;
    }

    RuntimeConfig config;
    std::vector<TextureVertex> base_vertices;
    std::vector<uint16_t> index_data;
    HardwareImage texture_image;
    Clock::time_point start_time{};
    bool initialized{false};

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<>> glsl_rasterizer;
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
    impl_->base_vertices = make_quad_vertices();
    impl_->index_data = make_quad_indices();

    if (!load_texture_image(resolve_texture_path(), impl_->texture_image, error_message))
    {
        return false;
    }
    if (!impl_->ensure_edsl_pipeline(outputs[0], error_message))
    {
        return false;
    }
    if (!impl_->ensure_glsl_pipeline(outputs[1], error_message))
    {
        return false;
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
        if (!impl_->ensure_edsl_pipeline(output_image, error_message))
        {
            return false;
        }
    }

    auto payload = std::static_pointer_cast<const TexturePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);

    executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
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
        if (!impl_->ensure_glsl_pipeline(output_image, error_message))
        {
            return false;
        }
    }

    auto payload = std::static_pointer_cast<const TexturePayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->vertices, BufferUsage::VertexBuffer);
    const uint32_t texture_descriptor = impl_->texture_image.storeDescriptor();
    (*impl_->glsl_rasterizer)[texture_frag_glsl::pc::textureHandle] = ktm::uvec2(texture_descriptor, 0u);
    impl_->glsl_rasterizer->record(*impl_->glsl_index_buffer, vertex_buffer);

    executor << (*impl_->glsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
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
