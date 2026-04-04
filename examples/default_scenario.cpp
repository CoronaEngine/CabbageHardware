#include "default_scenario.h"

#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <ktm/ktm.h>

#include "Codegen/BuiltinVariate.h"
#include "Codegen/ControlFlows.h"
#include "Codegen/CustomLibrary.h"
#include "Codegen/TypeAlias.h"
#include "cube_data.h"
#include "scenario_registry.h"

#include GLSL(vert.glsl)
#include GLSL(frag.glsl)
#include GLSL(compute.glsl)

struct VertexAttributeProxy
{
    EmbeddedShader::Float3 position;
    EmbeddedShader::Float3 color;
};

struct SimpleVertex
{
    std::array<float, 3> position{};
    std::array<float, 3> color{};
};

struct DefaultMeshPayload
{
    std::vector<std::vector<SimpleVertex>> transformed_vertices_per_object;
};

constexpr std::size_t kDefaultObjectCount = 20;
constexpr uint32_t kComputeGroupSize = 8;

static std::vector<SimpleVertex> make_simple_vertices_from_cube_data()
{
    std::vector<SimpleVertex> result;
    result.reserve(vertices.size());
    for (const auto &vertex : vertices)
    {
        result.push_back({vertex.position, vertex.color});
    }
    return result;
}

static std::vector<SimpleVertex> transform_vertices_for_object(const std::vector<SimpleVertex> &source,
                                                               const ktm::fmat4x4 &mvp)
{
    auto transformed = source;
    for (auto &vertex : transformed)
    {
        ktm::fvec4 clip_position =
            mvp * ktm::fvec4(vertex.position[0], vertex.position[1], vertex.position[2], 1.0f);
        float inverse_w = 1.0f / clip_position[3];
        vertex.position = {clip_position[0] * inverse_w,
                           clip_position[1] * inverse_w,
                           clip_position[2] * inverse_w};
    }
    return transformed;
}

static uint16_t compute_group_count(uint32_t size, uint32_t group_size)
{
    const uint32_t groups = (size + group_size - 1u) / group_size;
    return static_cast<uint16_t>(groups);
}

struct DefaultScenario::Impl
{
    explicit Impl(RuntimeConfig cfg) : config(std::move(cfg)) {}

    void ensure_edsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(edsl_mutex);
        if (edsl_rasterizer && edsl_compute && edsl_index_buffer)
        {
            return;
        }

        using namespace EmbeddedShader;
        using namespace EmbeddedShader::Ast;

        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        edsl_output = mutable_output_image;

        auto vertex_shader = [&](Aggregate<VertexAttributeProxy> vertex) -> Float4 {
            position() = Float4(vertex->position, 1.0f);
            return Float4(vertex->color, 1.0f);
        };

        auto fragment_shader = [&](Float4 interpolated_color) -> Float4 {
            return interpolated_color;
        };

        auto aces_filmic_tone_map_curve = [&](Float3 x) {
            Float a = 2.51f;
            Float b = 0.03f;
            Float c = 2.43f;
            Float d = 0.59f;
            Float e = 0.14f;
            return clamp((x * (a * x + b)) / (x * (c * x + d) + e), ktm::fvec3(0.0f), ktm::fvec3(1.0f));
        };

        auto compute_shader = [&] {
            Float4 color = edsl_output[dispatchThreadID()->xy()];
            edsl_output[dispatchThreadID()->xy()] = Float4(aces_filmic_tone_map_curve(color->xyz()), 1.0f);
        };

        edsl_rasterizer = std::make_unique<RasterizerPipeline<>>(vertex_shader, fragment_shader);
        edsl_rasterizer->bindOutputTargets(edsl_output);
        edsl_compute = std::make_unique<ComputePipeline<>>(compute_shader, ktm::uvec3(kComputeGroupSize, kComputeGroupSize, 1));
        edsl_index_buffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    void ensure_glsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (glsl_rasterizer && glsl_compute && glsl_index_buffer)
        {
            return;
        }

        glsl_rasterizer = std::make_unique<RasterizerPipeline<vert_glsl, frag_glsl>>();
        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        glsl_rasterizer->outColor = mutable_output_image;

        glsl_compute = std::make_unique<ComputePipeline<compute_glsl>>();
        glsl_compute->GlobalUniformParam.imageID = mutable_output_image.storeDescriptor();

        glsl_index_buffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;
    std::vector<SimpleVertex> simple_vertices;
    std::vector<ktm::fmat4x4> base_model_matrices;
    ktm::fmat4x4 vp_matrix{};
    Clock::time_point start_time{};
    bool initialized{false};

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<ComputePipeline<>> edsl_compute;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<vert_glsl, frag_glsl>> glsl_rasterizer;
    std::unique_ptr<ComputePipeline<compute_glsl>> glsl_compute;
    std::unique_ptr<HardwareBuffer> glsl_index_buffer;
};

DefaultScenario::DefaultScenario(const RuntimeConfig &config) : impl_(std::make_unique<Impl>(config)) {}

DefaultScenario::~DefaultScenario() = default;

std::string DefaultScenario::name() const
{
    return "default";
}

bool DefaultScenario::init(const RuntimeConfig &config,
                           const std::array<HardwareImage, 2> &outputs,
                           std::string &error_message)
{
    if (config.data_set != "cube")
    {
        error_message = "Unsupported dataset for default scenario: " + config.data_set;
        return false;
    }

    impl_->config = config;
    impl_->simple_vertices = make_simple_vertices_from_cube_data();

    auto view_matrix = ktm::look_at_lh(ktm::fvec3(2.0f, 2.0f, 2.0f),
                                       ktm::fvec3(0.0f, 0.0f, 0.0f),
                                       ktm::fvec3(0.0f, 0.0f, 1.0f));
    float aspect_ratio = (config.window_height == 0) ? 1.0f : static_cast<float>(config.window_width) / static_cast<float>(config.window_height);
    auto projection_matrix = ktm::perspective_lh(ktm::radians(45.0f), aspect_ratio, 0.1f, 10.0f);
    impl_->vp_matrix = projection_matrix * view_matrix;

    impl_->base_model_matrices.resize(kDefaultObjectCount);
    for (std::size_t i = 0; i < kDefaultObjectCount; ++i)
    {
        impl_->base_model_matrices[i] =
            ktm::translate3d(ktm::fvec3(static_cast<float>(i % 5) - 2.0f, static_cast<float>(i / 5) - 0.5f, 0.0f))
            * ktm::scale3d(ktm::fvec3(0.1f, 0.1f, 0.1f))
            * ktm::rotate3d_axis(ktm::radians(static_cast<float>(i) * 30.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
    }

    impl_->ensure_edsl_pipeline(outputs[0]);
    impl_->ensure_glsl_pipeline(outputs[1]);
    impl_->start_time = Clock::now();
    impl_->initialized = true;
    return true;
}

std::shared_ptr<const void> DefaultScenario::mesh_tick(uint64_t frame_id,
                                                       Clock::time_point now,
                                                       std::string &error_message)
{
    (void)frame_id;
    if (!impl_->initialized)
    {
        error_message = "Scenario has not been initialized before mesh_tick.";
        return nullptr;
    }

    float current_time = std::chrono::duration<float, std::chrono::seconds::period>(now - impl_->start_time).count();
    auto payload = std::make_shared<DefaultMeshPayload>();
    payload->transformed_vertices_per_object.reserve(impl_->base_model_matrices.size());

    for (const auto &base_model : impl_->base_model_matrices)
    {
        auto model = base_model * ktm::rotate3d_axis(current_time * ktm::radians(90.0f), ktm::fvec3(0.0f, 0.0f, 1.0f));
        payload->transformed_vertices_per_object.push_back(
            transform_vertices_for_object(impl_->simple_vertices, impl_->vp_matrix * model));
    }

    return payload;
}

bool DefaultScenario::render_edsl_tick(const MeshFrame &mesh_frame,
                                       HardwareExecutor &executor,
                                       const HardwareImage &output_image,
                                       std::string &error_message)
{
    if (!mesh_frame.payload)
    {
        error_message = "render_edsl_tick received an empty mesh payload.";
        return false;
    }
    if (!impl_->edsl_rasterizer || !impl_->edsl_compute || !impl_->edsl_index_buffer)
    {
        impl_->ensure_edsl_pipeline(output_image);
    }

    auto payload = std::static_pointer_cast<const DefaultMeshPayload>(mesh_frame.payload);
    for (const auto &transformed_vertices : payload->transformed_vertices_per_object)
    {
        HardwareBuffer vertex_buffer(transformed_vertices, BufferUsage::VertexBuffer);
        impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);
    }

    executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
             << (*impl_->edsl_compute)(compute_group_count(impl_->config.window_width, kComputeGroupSize),
                                       compute_group_count(impl_->config.window_height, kComputeGroupSize),
                                       1u)
             << executor.commit();
    return true;
}

bool DefaultScenario::render_glsl_tick(const MeshFrame &mesh_frame,
                                       HardwareExecutor &executor,
                                       const HardwareImage &output_image,
                                       std::string &error_message)
{
    if (!mesh_frame.payload)
    {
        error_message = "render_glsl_tick received an empty mesh payload.";
        return false;
    }
    if (!impl_->glsl_rasterizer || !impl_->glsl_compute || !impl_->glsl_index_buffer)
    {
        impl_->ensure_glsl_pipeline(output_image);
    }

    auto payload = std::static_pointer_cast<const DefaultMeshPayload>(mesh_frame.payload);
    for (const auto &transformed_vertices : payload->transformed_vertices_per_object)
    {
        HardwareBuffer vertex_buffer(transformed_vertices, BufferUsage::VertexBuffer);
        impl_->glsl_rasterizer->record(*impl_->glsl_index_buffer, vertex_buffer);
    }

    executor << (*impl_->glsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
             << (*impl_->glsl_compute)(compute_group_count(impl_->config.window_width, kComputeGroupSize),
                                       compute_group_count(impl_->config.window_height, kComputeGroupSize),
                                       1u)
             << executor.commit();
    return true;
}

void DefaultScenario::display_tick(const RenderFrame &render_frame)
{
    (void)render_frame;
}

void DefaultScenario::shutdown()
{
    impl_->edsl_index_buffer.reset();
    impl_->glsl_index_buffer.reset();
    impl_->edsl_rasterizer.reset();
    impl_->edsl_compute.reset();
    impl_->glsl_rasterizer.reset();
    impl_->glsl_compute.reset();
    impl_->initialized = false;
}

std::unique_ptr<ScenarioHooks> create_default_scenario(const RuntimeConfig &config)
{
    return std::make_unique<DefaultScenario>(config);
}

void register_default_scenario()
{
    static const bool registered = [] {
        register_scenario("default", create_default_scenario);
        return true;
    }();
    (void)registered;
}
