#include "scenario_registry.h"

#include <algorithm>

ScenarioRegistry &ScenarioRegistry::instance()
{
    // 进程级单例，负责全局场景注册与查询。
    static ScenarioRegistry registry;
    return registry;
}

bool ScenarioRegistry::register_scenario(const std::string &name, ScenarioFactory factory)
{
    if (name.empty() || !factory)
    {
        return false;
    }

    // 允许覆盖注册，便于同名场景在调试期快速替换实现。
    std::lock_guard<std::mutex> lock(mutex_);
    auto [it, inserted] = factories_.insert_or_assign(name, std::move(factory));
    (void)it;
    return inserted;
}

std::unique_ptr<ScenarioHooks> ScenarioRegistry::create_scenario(const std::string &name, const RuntimeConfig &config) const
{
    // 场景实例按需创建，避免无关场景提前初始化占资源。
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = factories_.find(name);
    if (it == factories_.end())
    {
        return nullptr;
    }
    return it->second(config);
}

std::vector<std::string> ScenarioRegistry::list_scenarios() const
{
    // 返回排序后的名字，保证日志/帮助输出稳定。
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    names.reserve(factories_.size());
    for (const auto &[name, _] : factories_)
    {
        (void)_;
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool register_scenario(const std::string &name, ScenarioFactory factory)
{
    return ScenarioRegistry::instance().register_scenario(name, std::move(factory));
}

std::unique_ptr<ScenarioHooks> create_scenario(const std::string &name, const RuntimeConfig &config)
{
    return ScenarioRegistry::instance().create_scenario(name, config);
}

std::vector<std::string> list_scenarios()
{
    return ScenarioRegistry::instance().list_scenarios();
}
