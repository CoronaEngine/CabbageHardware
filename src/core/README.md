# Core 基础设施

Core 的总体职责和依赖方向见 [架构概览](../../docs/architecture/overview.md)。

## 日志：当前实现

`util/logging.h` 提供不包含 Quill 头文件的公共日志接口；`util/logging.cpp`
在 `horizon-core` 中统一管理 logger、输出和后端初始化。`info`、`warning`、
`OC_*` 宏与旧 `Corona::Kernel::CoronaLogger` / `CFW_LOG_*` 宏共享同一个实例、
日志级别及 flush 行为。

基础库首次记录日志时默认只输出到控制台，不创建日志文件、不安装信号处理器、
不修改 Windows 控制台设置。应用需要在首次记录日志及直接启动 Quill 之前显式配置：

```cpp
horizon::core::LoggingOptions options;
options.file_path = "logs/application.log";
options.install_signal_handlers = true;
options.configure_utf8_console = true;
horizon::core::initialize_logging(options);
```

首个成功的初始化生效；重复初始化不改变已有配置。配置失败会抛出异常，允许重试。
初始化后可用 `set_log_level` 或已有 `log_level_*` 接口调整级别。
`file_path` 为空时禁用文件输出；指定文件时创建父目录，并在首次初始化时覆盖该文件。
若需要按运行保留历史文件，由应用选择唯一文件名。

`examples/main.cpp` 保留原有示例策略：控制台、带时间戳的 `logs/*_corona.log`、
UTF-8 控制台设置和 Quill 的信号/异常处理。其他应用如需这些行为，也应显式配置。
信号处理使用 Quill 的平台实现；启用不表示所有异常终止都能完整刷新日志。

## 迁移兼容

`include/horizon/core/logging.h` 提供统一入口，并保留旧类型和宏的源代码兼容转发，
不再拥有 logger。旧接口现在采用 Core 的默认配置，不再隐式创建文件或安装信号处理器。
旧宏仍保留调用位置、格式化及 Python/Vue 前缀；`CFW_LOG_ERROR` 仍只记录错误，
不会变成 `OC_ERROR` 的终止进程语义。

`util/logging_quill.h` 是可选的 Quill 互操作入口，供兼容宏和高级调用方使用。
由于公开兼容接口返回 Quill 类型并使用其宏，`horizon-core` 公开传递 `quill::quill`
依赖，调用方只需链接 `horizon-core`。返回的 logger 为非拥有指针，不得删除或替换它的 sinks。

旧 `src/kernel/core/logger.cpp` 已移除。`Horizon` target 公开依赖 `horizon-core`，
旧引擎的短名称通过 `namespace horizon` 内的兼容导入保留，允许与 `horizon::core`
一起使用。

公开头文件统一位于 `include/horizon/core/`：`logging.h`、`storage.h` 和
`stack_trace.h`。`Storage`、`StaticBuffer` 和堆栈诊断函数归属 `horizon::core`，
旧 `Corona::Kernel::Utils` 名称仅保留为兼容别名。原 `include/corona/kernel/`
路径已移除，调用方需要更新 include 路径；存储算法和平台诊断能力保持原状。
