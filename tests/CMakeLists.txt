add_executable(CabbageHardwareLifecycleTests
    lifecycle_tests.cpp
)

target_include_directories(CabbageHardwareLifecycleTests PRIVATE
    "${PROJECT_SOURCE_DIR}/Src"
    "${PROJECT_SOURCE_DIR}/examples"
)

target_link_libraries(CabbageHardwareLifecycleTests PRIVATE
    CabbageHardware
)

add_test(NAME CabbageHardwareLifecycleTests COMMAND CabbageHardwareLifecycleTests)
