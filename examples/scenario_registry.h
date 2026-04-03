#pragma once

#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "runtime_config.h"
#include "Scenario.h"

using ScenarioFactory = std::function<std::unique_ptr<ScenarioHooks>(const RuntimeConfig &)>;

// 场景注册表：负责场景工厂的注册与实例化。
class ScenarioRegistry
{
  public:
    static ScenarioRegistry &instance();

    // 注册场景工厂。name 重复时会覆盖旧工厂。
    bool register_scenario(const std::string &name, ScenarioFactory factory);

    // 按名称创建场景实例。不存在时返回 nullptr。
    std::unique_ptr<ScenarioHooks> create_scenario(const std::string &name, const RuntimeConfig &config) const;
    // 列出当前所有可用场景名。
    std::vector<std::string> list_scenarios() const;

  private:
    ScenarioRegistry() = default;

    std::unordered_map<std::string, ScenarioFactory> factories_;
    mutable std::mutex mutex_;
};

// 全局便捷函数，转发到单例注册表。
bool register_scenario(const std::string &name, ScenarioFactory factory);
std::unique_ptr<ScenarioHooks> create_scenario(const std::string &name, const RuntimeConfig &config);
std::vector<std::string> list_scenarios();
