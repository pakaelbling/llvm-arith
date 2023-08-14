cmake_minimum_required(VERSION 3.26)
project(ArithComp)

include(./cmake/CPM.cmake)
find_package(LLVM REQUIRED CONFIG)
find_package(GTest REQUIRED CONFIG)
include(./cmake/deps.cmake)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(./cmake-build-debug/_deps/peglib-src)
add_executable(ArithComp main.cpp
        parser.cpp
        compiler.cpp
        utils.cpp
        compiler.h
        parser.h
        utils.h)
llvm_map_components_to_libnames(llvm_libs analysis core executionengine instcombine object orcjit runtimedyld scalaropts support native MC mc mcjit)
enable_testing()
add_executable(runTest
        parser.cpp
        compiler.cpp
        utils.cpp
        compiler.h
        parser.h
        utils.h
        test.cpp
)

# Link against LLVM libraries
target_link_libraries(ArithComp ${llvm_libs})
target_link_libraries(runTest ${llvm_libs} GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(runTest)
add_test(NAME runTest
        COMMAND runTest
)