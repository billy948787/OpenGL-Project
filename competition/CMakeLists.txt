cmake_minimum_required(VERSION 3.10)
project(competition)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/CG_HW3)

set(CMAKE_CXX_STANDARD 17)
set(OpenCV_DIR ${PROJECT_SOURCE_DIR}/build_opencv)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND APPLE) 
    set(FBX_SDK_LIB ${PROJECT_SOURCE_DIR}/Library/fbx_sdk/lib/clang/debug)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" AND APPLE)
    set(FBX_SDK_LIB ${PROJECT_SOURCE_DIR}/Library/fbx_sdk/lib/clang/release)
endif()

if(LINUX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FBX_SDK_LIB ${PROJECT_SOURCE_DIR}/Library/fbx_linux/lib/debug)
elseif(LINUX AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(FBX_SDK_LIB ${PROJECT_SOURCE_DIR}/Library/fbx_linux/lib/release)
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FBX_SDK_LIB ${PROJECT_SOURCE_DIR}/Library/fbx_win/lib/x64/debug)
elseif(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(FBX_SDK_LIB ${PROJECT_SOURCE_DIR}/Library/fbx_win/lib/x64/release)
endif()

if(WIN32)
    set(FBX_SDK_INCLUDE ${PROJECT_SOURCE_DIR}/Library/fbx_win/include)
elseif(APPLE)
    set(FBX_SDK_INCLUDE ${PROJECT_SOURCE_DIR}/Library/fbx_sdk/include)
elseif(LINUX)
    set(FBX_SDK_INCLUDE ${PROJECT_SOURCE_DIR}/Library/fbx_linux/include)
endif()

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/Library/glfw-3.4)
add_subdirectory(${PROJECT_SOURCE_DIR}/Library/glew-2.2.0/build/cmake)
add_subdirectory(${PROJECT_SOURCE_DIR}/Library/glm-1.0.1)
add_subdirectory(${PROJECT_SOURCE_DIR}/Library/assimp-5.4.3)

file(GLOB SOURCES "CG_HW3/*.cpp")
file(GLOB IMGUI_SOURCES "Library/imgui/*.cpp")

add_executable(competition ${SOURCES} ${IMGUI_SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/CG_HW3)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/Library/imgui/include)
include_directories(${PROJECT_SOURCE_DIR}/Library/glm-1.0.1/glm)
include_directories(${PROJECT_SOURCE_DIR}/Library/glfw-3.4/include)
include_directories(${PROJECT_SOURCE_DIR}/Library/glew-2.2.0/include)
include_directories(${PROJECT_SOURCE_DIR}/Library/imgui/backends/)
include_directories(${PROJECT_SOURCE_DIR}/Library/assimp-5.4.3/include)
include_directories(SYSTEM ${FBX_SDK_INCLUDE})

set_target_properties(competition PROPERTIES
    BUILD_RPATH "${FBX_SDK_LIB}"
    INSTALL_RPATH "${FBX_SDK_LIB}"
)

target_link_libraries(competition glew)
target_link_libraries(competition ${OpenCV_LIBS})
target_link_libraries(competition OpenGL::GL)
target_link_libraries(competition glfw)
target_link_libraries(competition assimp)
target_link_libraries(competition ${FBX_SDK_LIB}/libfbxsdk.so)
target_link_libraries(competition ${FBX_SDK_LIB}/libfbxsdk.a)


if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

