#pragma once

#include <memory>

#include "../runtime_config.h"
#include "../scenario.h"

class TriangleScenario final : public ScenarioHooks
{
  public:
    explicit TriangleScenario(const RuntimeConfig &config);
    ~TriangleScenario() override;

    std::string name() const override;

    bool init(const RuntimeConfig &config,
              Backend backend,
              const HardwareImage &output,
              std::string &error_message) override;

    std::shared_ptr<const void> mesh_tick(uint64_t frame_id,
                                          Clock::time_point now,
                                          std::string &error_message) override;

    bool render_tick(const MeshFrame &mesh_frame,
                     Backend backend,
                     HardwareExecutor &executor,
                     const HardwareImage &output_image,
                     std::string &error_message) override;

    void display_tick(const RenderFrame &render_frame) override;
    void shutdown() override;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

std::unique_ptr<ScenarioHooks> create_triangle_scenario(const RuntimeConfig &config);

void register_triangle_scenario();
