cmake_minimum_required(VERSION 3.21)

project(IMGPROCESSOR LANGUAGES CXX CUDA) # CUDA added, others later

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
    # Let CMake detect GPU architecture automatically (CMake ≥ 3.21 recommended)
    set(CMAKE_CUDA_ARCHITECTURES native)
endif()

# =====================
# Qt
# =====================
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# =====================
# Sources
# =====================
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CUDA_SOURCES "src/*.cu") # only used if CUDA enabled

# =====================
# Executable
# =====================
add_executable(MyExecutable ${CPP_SOURCES})

target_include_directories(MyExecutable PUBLIC include)
target_link_libraries(MyExecutable Qt6::Widgets)

# =====================
# Backends
# =====================
if(ENABLE_CUDA)
    message(STATUS "CUDA backend enabled")
    target_sources(MyExecutable PRIVATE ${CUDA_SOURCES})
    set_target_properties(MyExecutable PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
    )
endif()

if(ENABLE_OPENCL)
    message(STATUS "OpenCL backend enabled (not yet implemented)")
    # find_package(OpenCL REQUIRED)
    # target_link_libraries(MyExecutable OpenCL::OpenCL)
endif()

if(ENABLE_HIP)
    message(STATUS "HIP backend enabled (not yet implemented)")
    # find_package(HIP REQUIRED)
    # target_link_libraries(MyExecutable HIP::hip)
endif()

if(ENABLE_ROCM)
    message(STATUS "ROCm backend enabled (not yet implemented)")
    # find_package(ROCM REQUIRED)
    # target_link_libraries(MyExecutable ROCM::rocm)
endif()

# =====================
# Build Info
# =====================
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CUDA Compiler: ${CMAKE_CUDA_COMPILER_ID} ${CMAKE_CUDA_COMPILER_VERSION}")
message(STATUS "CUDA Architectures: ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
