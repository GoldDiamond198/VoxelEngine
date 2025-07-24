cmake_minimum_required(VERSION 3.10)
project(VoxelEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GLFW and Vulkan
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# (Optional: if you use GLM elsewhere)
# find_package(glm CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(VoxelDemo ${SRC_FILES})

target_link_libraries(VoxelDemo PRIVATE
    glfw
    Vulkan::Vulkan           # ← link the Vulkan loader
    # glm::glm               # ← if you need GLM
)

set_target_properties(VoxelDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
