cmake_minimum_required(VERSION 3.3)
project(clime)

include(CheckCXXCompilerFlag)


# libclime requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -pedantic -pedantic-errors")

# Enable all the warnings one can get
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wshadow -Wmissing-include-dirs -Wpacked -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual -Wnoexcept")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal -Wno-sign-compare ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wcast-align -Wwrite-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security")

check_cxx_compiler_flag("-D_FORTIFY_SOURCE=2" WITH_FORTIFY_SOURCE)
if (WITH_FORTIFY_SOURCE AND UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
endif()


# Build project dependencies (should be removed eventually)
add_subdirectory(external/gtest/googletest EXCLUDE_FROM_ALL)

include_directories(include)

include_directories(../libsolace/include)
#set(CMAKE_CURRENT_LIST_DIR "${CMAKE_CURRENT_LIST_DIR} ../libsolace/build/src")
link_directories(../libsolace/build/src)

#add_subdirectory(../libsolace binary_dir EXCLUDE_FROM_ALL)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)

