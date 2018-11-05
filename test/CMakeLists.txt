# Maybe someday...
# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")

set(TEST_SOURCE_FILES
        ci/teamcity_messages.cpp
        ci/teamcity_gtest.cpp

        main_gtest.cpp

        test_parser.cpp
        )


include_directories(${gtest_SOURCE_DIR}/include ${COMMON_INCLUDES})
add_executable(test_${PROJECT_NAME} ${TEST_SOURCE_FILES})

if(UNIX AND NOT APPLE)
    target_link_libraries(test_${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}
        solace
        gtest
        rt)
else()
    target_link_libraries(test_${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}
        solace
        gtest)
endif()
