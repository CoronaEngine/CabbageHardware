file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/include/CabbageHardware.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group("include" FILES ${PUBLIC_HEADERS})

add_library(CabbageHardware STATIC ${SOURCE_FILES} ${PUBLIC_HEADERS})

target_include_directories(CabbageHardware
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${Helicon_SOURCE_DIR}"
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 链接库依赖
target_link_libraries(volk PUBLIC Vulkan-Headers)

target_link_libraries(CabbageHardware PUBLIC
    Helicon
    corona::kernel
    volk
    Vulkan-Headers
    VulkanMemoryAllocator)