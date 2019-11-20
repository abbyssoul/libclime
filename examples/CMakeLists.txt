# Build examples

# CLI single command flavour:
set(EXAMPLE_APPFRAMEWORK_SOURCE_FILES cli_single.cpp)
add_executable(cli_single ${EXAMPLE_APPFRAMEWORK_SOURCE_FILES})
target_link_libraries(cli_single PUBLIC ${PROJECT_NAME} ${CONAN_LIBS})

# CLI multi-command flavour:
set(EXAMPLE_APPFRAMEWORK_SOURCE_FILES cli_multi.cpp)
add_executable(cli_multi ${EXAMPLE_APPFRAMEWORK_SOURCE_FILES})
target_link_libraries(cli_multi PUBLIC ${PROJECT_NAME} ${CONAN_LIBS})


add_custom_target(examples
    DEPENDS cli_multi cli_single)
