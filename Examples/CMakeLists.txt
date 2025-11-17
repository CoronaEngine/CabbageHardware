file(GLOB_RECURSE EXAMPLES_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EXAMPLES_SOURCE_FILES})
add_executable(CabbageHardwareExamples ${EXAMPLES_SOURCE_FILES})

# 设置VS启动项目
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CabbageHardwareExamples)

set_target_properties(CabbageHardwareExamples PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:CabbageHardwareExamples>
)

target_include_directories(CabbageHardwareExamples PRIVATE
    "${glfw_SOURCE_DIR}/include"
    "${stb_SOURCE_DIR}")

target_link_libraries(CabbageHardwareExamples PRIVATE
    glfw
    CabbageHardware)

helicon_install_runtime_deps(CabbageHardwareExamples)
corona_copy_tbb_runtime_artifacts(CabbageHardwareExamples)