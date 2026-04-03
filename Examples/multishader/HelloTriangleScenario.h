#pragma once

#include <memory>

#include "RuntimeConfig.h"
#include "Scenario.h"

namespace multishader
{
// 默认示例场景：使用立方体数据，分别走 EDSL/GLSL 两条渲染路径。
class HelloTriangleScenario final : public ScenarioHooks
{
  public:
    // 构造时仅保存配置，不做重资源初始化。
    explicit HelloTriangleScenario(const RuntimeConfig &config);
    ~HelloTriangleScenario() override;

    std::string name() const override;
    // 初始化场景资源与固定矩阵，并预热两条渲染管线。
    bool init(const RuntimeConfig &config,
              const std::array<HardwareImage, 2> &outputs,
              std::string &errorMessage) override;
    // 每帧在 mesh 线程执行：生成两个 render 线程共享的顶点变换结果。
    std::shared_ptr<const void> meshTick(uint64_t frameId,
                                         Clock::time_point now,
                                         std::string &errorMessage) override;
    // EDSL 渲染线程入口。
    bool renderEDSLTick(const MeshFrame &meshFrame,
                        HardwareExecutor &executor,
                        const HardwareImage &outputImage,
                        std::string &errorMessage) override;
    // GLSL 渲染线程入口。
    bool renderGLSLTick(const MeshFrame &meshFrame,
                        HardwareExecutor &executor,
                        const HardwareImage &outputImage,
                        std::string &errorMessage) override;
    // 预留给 display 阶段扩展（目前无额外逻辑）。
    void displayTick(const RenderFrame &renderFrame) override;
    // 释放场景持有的管线与缓冲资源。
    void shutdown() override;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// 场景工厂：供注册表调用创建实例。
std::unique_ptr<ScenarioHooks> createHelloTriangleScenario(const RuntimeConfig &config);
// 注册内置场景（default 与 hello_triangle）。
void registerHelloTriangleScenario();
} // multishader 命名空间
