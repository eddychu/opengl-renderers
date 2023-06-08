cmake_minimum_required(VERSION 3.20)
project(renderer CXX C)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h src/*.c)

add_executable(renderer ${SOURCES})

target_include_directories(renderer PRIVATE src)

find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})

# find_package(assimp CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${TINYGLTF_INCLUDE_DIRS})

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})