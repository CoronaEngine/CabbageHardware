#pragma once

#include <memory>

#include "../runtime_config.h"
#include "../scenario.h"

// 默认示例场景：使用立方体数据，分别走 EDSL/GLSL 两条渲染路径。
class DefaultScenario final : public ScenarioHooks
{
  public:
    // 构造时仅保存配置，不做重资源初始化。
    explicit DefaultScenario(const RuntimeConfig &config);
    ~DefaultScenario() override;

    std::string name() const override;

    // 初始化场景资源与固定矩阵，并预热两条渲染管线。
    bool init(const RuntimeConfig &config,
              Backend backend,
              const HardwareImage &output,
              std::string &error_message) override;

    // 每帧在 mesh 线程执行：生成两个 render 线程共享的顶点变换结果。
    std::shared_ptr<const void> mesh_tick(uint64_t frame_id,
                                          Clock::time_point now,
                                          std::string &error_message) override;
    // 统一渲染入口，按 backend 分发到 EDSL 或 GLSL。
    bool render_tick(const MeshFrame &mesh_frame,
                     Backend backend,
                     HardwareExecutor &executor,
                     const HardwareImage &output_image,
                     std::string &error_message) override;

    // 预留给 display 阶段扩展（目前无额外逻辑）。
    void display_tick(const RenderFrame &render_frame) override;

    // 释放场景持有的管线与缓冲资源。
    void shutdown() override;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// 场景工厂：供注册表调用创建实例。
std::unique_ptr<ScenarioHooks> create_default_scenario(const RuntimeConfig &config);

// 注册默认场景（default）。
void register_default_scenario();
