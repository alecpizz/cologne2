cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)

#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !

option(PRODUCTION_BUILD "Make this a production build" OFF)


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization

if (MSVC)
    add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif ()

project(goon)

include(FetchContent)

set(SDL_STATIC ON)
set(SDLMIXER_VENDORED OFF)
set(SDLMIXER_STATIC ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/SDL)
add_subdirectory(vendor/SDL_mixer)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/stb_truetype)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui-docking)
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/freetype)

FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.3.0"
        UPDATE_DISCONNECTED TRUE
        SOURCE_SUBDIR "Jolt"
)
set(DOUBLE_PRECISION OFF)
set(GENERATE_DEBUG_SYMBOLS OFF)
set(CROSS_PLATFORM_DETERMINISTIC OFF)
set(OBJECT_LAYER_BITS 32)
set(USE_SSE4_1 ON)
set(USE_SSE4_2 ON)
set(USE_AVX ON)
set(USE_AVX2 ON)
set(USE_AVX512 OFF)
set(USE_LZCNT ON)
set(USE_TZCNT ON)
set(INTERPROCEDURAL_OPTIMIZATION ON)
set(USE_F16C ON)
set(USE_FMADD ON)
set(FLOATING_POINT_EXCEPTIONS_ENABLED OFF)

if (NOT JoltPhysics_POPULATED)
    FetchContent_MakeAvailable(JoltPhysics)
    set(PHYSICS_REPO_ROOT ${joltphysics_SOURCE_DIR})
    include(${joltphysics_SOURCE_DIR}/Jolt/Jolt.cmake)
    target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:RelWithDebInfo>:JPH_PROFILE_ENABLED;JPH_DEBUG_RENDERER>")
    target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Release>:JPH_DEBUG_RENDERER>")
    target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Debug>:JPH_PROFILE_ENABLED;JPH_DEBUG_RENDERER>")
    if (FLOATING_POINT_EXCEPTIONS_ENABLED AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:RelWithDebInfo>:JPH_FLOATING_POINT_EXCEPTIONS_ENABLED>")
        target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Debug>:JPH_FLOATING_POINT_EXCEPTIONS_ENABLED>")
    endif ()
endif ()


# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)

if (PRODUCTION_BUILD)
    # setup the ASSETS_PATH macro to be in the root folder of your exe
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)

else ()
    # This is useful to get an ASSETS_PATH in your IDE during development
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)

endif ()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})


if (MSVC) # If using the VS compiler...

    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)
    #set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console


endif ()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_precompile_headers("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gpch.h")
target_include_directories(${CMAKE_PROJECT_NAME}  PUBLIC ${JoltPhysics_SOURCE_DIR})
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm spdlog::spdlog_header_only
        stb_image stb_truetype imgui SDL3-static assimp Jolt freetype SDL3_mixer-static)


