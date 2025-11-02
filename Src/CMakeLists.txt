file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_library(CabbageHardware STATIC ${SOURCE_FILES})

target_include_directories(CabbageHardware PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${Helicon_SOURCE_DIR}"
)

# 链接库依赖
target_link_libraries(volk PUBLIC Vulkan-Headers)

target_link_libraries(CabbageHardware PUBLIC
    Helicon
    corona::kernel
    volk
    Vulkan-Headers
    VulkanMemoryAllocator)