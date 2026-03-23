## CabbageHardware

一个现代化的跨平台图形硬件抽象层,提供流式 API 设计和 Helicon 着色器 DSL。

### 特性

- **简洁的 API**: 流式命令执行器设计,让 GPU 编程变得直观
- **Vulkan 后端**: 充分利用现代图形硬件性能
- **Helicon DSL**: 类型安全的着色器语言,支持 AST 编译和多后端代码生成
- **资源代理系统**: 统一的资源绑定和管理
- **多线程支持**: 完整的多线程执行模型和同步机制
- **外部内存**: 支持导入导出外部内存,便于跨进程共享

### 快速开始

```cpp
#include "CabbageHardware.h"

// 创建缓冲区
HardwareBuffer vertexBuffer(vertices, BufferUsage::VertexBuffer);

// 创建图像
HardwareImage image(width, height, ImageFormat::RGBA8_SRGB, ImageUsage::SampledImage);

// 流式命令执行
executor << vertexBuffer.copyTo(indexBuffer)
         << dispatchCompute(128, 128, 1)
         << execute();
```

### 核心组件

- **HardwareBuffer**: GPU 缓冲区,支持顶点、索引、uniform、storage 等多种用途
- **HardwareImage**: GPU 图像资源,支持多种格式和用途
- **HardwareExecutor**: 流式命令执行器,支持链式操作
- **ComputePipelineObject / RasterizedPipelineObject**: 计算和光栅化管线抽象

### 构建要求

- CMake 3.20+
- Vulkan SDK
- 支持 C++20 的编译器
- Windows/Linux

### 构建步骤

```bash
git clone https://github.com/CoronaEngine/CabbageHardware.git
cd CabbageHardware
mkdir build && cd build
cmake ..
cmake --build .
```

### 示例代码

查看 [Examples](Examples/) 目录获取完整示例:
- `baseline.cpp` - 基础渲染示例
- `multithreading.cpp` - 多线程使用示例
- 各种 GLSL 着色器文件

### 架构文档

完整文档请参考:
- [架构概述](4-architecture-overview) 正在编写
- [快速开始](2-quick-start) 正在编写
- [构建系统配置](3-build-system-and-cmake-setup) 后续完善

### 待办事项

当前项目正在积极开发中,已知问题包括 UBO bindless 优化、多线程内存泄漏、Intel 核显兼容性等。详见 [README.md](README.md#L1-L15) 完整列表。

### 许可证

本项目使用 MIT 许可证,详见 [LICENSE](LICENSE) 文件。