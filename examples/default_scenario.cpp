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
    std::vector<SimpleVertex> transformed_vertices;
};

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

struct DefaultScenario::Impl
{
    explicit Impl(RuntimeConfig cfg) : config(std::move(cfg)) {}

    void ensure_edsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(edsl_mutex);
        if (edsl_rasterizer)
        {
            return;
        }

        using namespace EmbeddedShader;
        using namespace EmbeddedShader::Ast;

        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        edsl_output = mutable_output_image;

        auto vs_lambda = [&](Aggregate<VertexAttributeProxy> vertex) -> Float4 {
            position() = Float4(vertex->position, 1.0f);
            return Float4(vertex->color, 1.0f);
        };

        auto fs_lambda = [&](Float4 interpolated_color) -> Float4 {
            return interpolated_color;
        };

        edsl_rasterizer = std::make_unique<RasterizerPipeline<>>(vs_lambda, fs_lambda);
        edsl_rasterizer->bindOutputTargets(edsl_output);
        edsl_index_buffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    void ensure_glsl_pipeline(const HardwareImage &output_image)
    {
        std::lock_guard<std::mutex> lock(glsl_mutex);
        if (glsl_rasterizer)
        {
            return;
        }

        glsl_rasterizer = std::make_unique<RasterizerPipeline<vert_glsl, frag_glsl>>();
        auto &mutable_output_image = const_cast<HardwareImage &>(output_image);
        glsl_rasterizer->outColor = mutable_output_image;
        glsl_index_buffer = std::make_unique<HardwareBuffer>(indices, BufferUsage::IndexBuffer);
    }

    RuntimeConfig config;
    std::vector<SimpleVertex> simple_vertices;
    bool initialized{false};

    std::mutex edsl_mutex;
    EmbeddedShader::Texture2D<ktm::fvec4> edsl_output;
    std::unique_ptr<RasterizerPipeline<>> edsl_rasterizer;
    std::unique_ptr<HardwareBuffer> edsl_index_buffer;

    std::mutex glsl_mutex;
    std::unique_ptr<RasterizerPipeline<vert_glsl, frag_glsl>> glsl_rasterizer;
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
    impl_->ensure_edsl_pipeline(outputs[0]);
    impl_->ensure_glsl_pipeline(outputs[1]);
    impl_->initialized = true;
    return true;
}

std::shared_ptr<const void> DefaultScenario::mesh_tick(uint64_t frame_id,
                                                       Clock::time_point now,
                                                       std::string &error_message)
{
    (void)frame_id;
    (void)now;
    if (!impl_->initialized)
    {
        error_message = "Scenario has not been initialized before mesh_tick.";
        return nullptr;
    }

    auto payload = std::make_shared<DefaultMeshPayload>();
    payload->transformed_vertices = impl_->simple_vertices;
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
    if (!impl_->edsl_rasterizer || !impl_->edsl_index_buffer)
    {
        impl_->ensure_edsl_pipeline(output_image);
    }

    auto payload = std::static_pointer_cast<const DefaultMeshPayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->transformed_vertices, BufferUsage::VertexBuffer);
    impl_->edsl_rasterizer->record(*impl_->edsl_index_buffer, vertex_buffer);

    executor << (*impl_->edsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
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
    if (!impl_->glsl_rasterizer || !impl_->glsl_index_buffer)
    {
        impl_->ensure_glsl_pipeline(output_image);
    }

    auto payload = std::static_pointer_cast<const DefaultMeshPayload>(mesh_frame.payload);
    HardwareBuffer vertex_buffer(payload->transformed_vertices, BufferUsage::VertexBuffer);
    impl_->glsl_rasterizer->record(*impl_->glsl_index_buffer, vertex_buffer);

    executor << (*impl_->glsl_rasterizer)(static_cast<uint16_t>(impl_->config.window_width),
                                          static_cast<uint16_t>(impl_->config.window_height))
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
    impl_->glsl_rasterizer.reset();
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
        register_scenario("hello_triangle", create_default_scenario);
        return true;
    }();
    (void)registered;
}
