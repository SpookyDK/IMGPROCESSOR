cmake_minimum_required(VERSION 3.21)
project(IMGPROCESSOR LANGUAGES CXX CUDA)

# =====================
# Build options (future backends)
# =====================
option(ENABLE_CUDA "Enable CUDA backend" ON)
option(ENABLE_OPENCL "Enable OpenCL backend" OFF)
option(ENABLE_HIP "Enable HIP backend" OFF)
option(ENABLE_ROCM "Enable ROCm backend" OFF)

# =====================
# Language standards
# =====================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Profiling-friendly flags
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer -fno-inline ")
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)

# =====================
# CUDA settings
# =====================
if(ENABLE_CUDA)
    set(CMAKE_CUDA_ARCHITECTURES native)
endif()

# =====================
# Qt
# =====================
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# =====================
# Source files
# =====================
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CUDA_SOURCES "src/*.cu") # only used if CUDA enabled

# Separate main.cpp from other sources
list(FILTER CPP_SOURCES EXCLUDE REGEX "src/main.cpp$")

# =====================
# Core library
# =====================
add_library(MyLib
    ${CPP_SOURCES}
)

target_include_directories(MyLib PUBLIC include)
target_link_libraries(MyLib PUBLIC Qt6::Widgets)

# Add CUDA sources if enabled
if(ENABLE_CUDA)
    message(STATUS "CUDA backend enabled")
    target_sources(MyLib PRIVATE ${CUDA_SOURCES})
    set_target_properties(MyLib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
endif()

# =====================
# Main executable
# =====================
add_executable(MyExecutable src/main.cpp)
target_link_libraries(MyExecutable PRIVATE MyLib Qt6::Widgets)

# =====================
# Tests
# =====================
file(GLOB TEST_SOURCES "tests/*.cpp")

if(TEST_SOURCES)
    add_executable(MyTests ${TEST_SOURCES})
    target_include_directories(MyTests PUBLIC include)
    target_link_libraries(MyTests PRIVATE MyLib Qt6::Widgets)

    enable_testing()
    add_test(NAME RunUnitTests COMMAND MyTests)
endif()

# =====================
# Future backends (placeholders)
# =====================
if(ENABLE_OPENCL)
    message(STATUS "OpenCL backend enabled (not yet implemented)")
    # find_package(OpenCL REQUIRED)
    # target_link_libraries(MyLib OpenCL::OpenCL)
endif()

if(ENABLE_HIP)
    message(STATUS "HIP backend enabled (not yet implemented)")
    # find_package(HIP REQUIRED)
    # target_link_libraries(MyLib HIP::hip)
endif()

if(ENABLE_ROCM)
    message(STATUS "ROCm backend enabled (not yet implemented)")
    # find_package(ROCM REQUIRED)
    # target_link_libraries(MyLib ROCM::rocm)
endif()

# =====================
# Build info
# =====================
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CUDA Compiler: ${CMAKE_CUDA_COMPILER_ID} ${CMAKE_CUDA_COMPILER_VERSION}")
message(STATUS "CUDA Architectures: ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
