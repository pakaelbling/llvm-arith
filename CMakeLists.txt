cmake_minimum_required(VERSION 3.26)
project(ArithComp)

include(./cmake/CPM.cmake)
find_package(LLVM REQUIRED CONFIG)
include(./cmake/deps.cmake)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(./cmake-build-debug/_deps/peglib-src)
add_executable(ArithComp main.cpp
        parser.h
        compiler.h)
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(ArithComp ${llvm_libs})