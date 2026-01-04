cmake_minimum_required(VERSION 4.0)

project(
    CabbageHardware
    VERSION 0.5.0
    HOMEPAGE_URL "https://github.com/CoronaEngine/CabbageHardware"
    LANGUAGES C CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(CABBAGE_HARDWARE_BUILD_EXAMPLES "Build CabbageHardware examples" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    NOMINMAX

    $<$<CONFIG:Debug>:CABBAGE_ENGINE_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:CABBAGE_ENGINE_DEBUG>
    $<$<CONFIG:Release>:CABBAGE_ENGINE_RELEASE>
    $<$<CONFIG:MinSizeRel>:CABBAGE_ENGINE_RELEASE>
)

include(FetchContent)

FetchContent_Declare(
    Helicon
    GIT_REPOSITORY https://github.com/CoronaEngine/Helicon.git
    GIT_TAG main
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(Helicon)

FetchContent_Declare(CoronaFramework
    GIT_REPOSITORY https://github.com/CoronaEngine/CoronaFramework.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(CoronaFramework)

set(VOLK_PULL_IN_VULKAN OFF) # We will provide Vulkan-Headers ourselves
FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG master
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(volk)

FetchContent_Declare(
    Vulkan-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG main
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(Vulkan-Headers)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

add_subdirectory(Src)

if(CABBAGE_HARDWARE_BUILD_EXAMPLES)
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(stb)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG master
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(glfw)

    add_subdirectory(Examples)
endif()


if (MSVC)
    # 启用 AddressSanitizer 编译选项
    add_compile_options(/fsanitize=address)

    # 必须禁用增量链接 (Incremental Linking)
    # ASan 不兼容 /INCREMENTAL
    add_link_options(/INCREMENTAL:NO)
    
    # (可选) 为了获得更好的堆栈追踪信息，可以加上这个
    add_compile_options(/Zi) 
endif()