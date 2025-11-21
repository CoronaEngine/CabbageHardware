file(GLOB_RECURSE EXAMPLES_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(CabbageHardwareExamples ${EXAMPLES_SOURCE_FILES})

target_include_directories(CabbageHardwareExamples PRIVATE
    "${glfw_SOURCE_DIR}/include"
    "${stb_SOURCE_DIR}")

target_link_libraries(CabbageHardwareExamples PRIVATE
    glfw
    CabbageHardware)

helicon_install_runtime_deps(CabbageHardwareExamples)
corona_copy_tbb_runtime_artifacts(CabbageHardwareExamples)