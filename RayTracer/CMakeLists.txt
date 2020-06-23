###############################################################################
## General
cmake_minimum_required(VERSION "3.17.2")
project("RayTracer")

set(CMAKE_CXX_STANDARD 11) # TODO: Verify version
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/lib)
set(SDL2_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sdl2/include)
set(ASSIMP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/include)
set(ASSIMP_CODE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/code)
set(ASSIMP_LIBRARIES_DIR_X86 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x86)
set(ASSIMP_LIBRARIES_DIR_X64 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x64)
set(ASSIMP_LIBRARIES_X86 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x86/assimp-vc141-mt.lib)
set(ASSIMP_LIBRARIES_X64 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x64/assimp-vc141-mt.lib)
set(ASSIMP_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1)
enable_testing()

###############################################################################
## Add subdirectories
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")

include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/tests)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/googletest)

find_package(SDL2 REQUIRED)
include_directories("${PROJECT_NAME}" ${SDL2_INCLUDE_DIR})

include_directories("${PROJECT_NAME}" ${ASSIMP_INCLUDE_DIR})
include_directories("${PROJECT_NAME}" ${ASSIMP_CODE_DIR})
link_directories(${ASSIMP_LIBRARIES_DIR_X86})
link_directories(${ASSIMP_LIBRARIES_DIR_X64})

###############################################################################
## Link libraries
set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES_X86} ${ASSIMP_LIBRARIES_X64})
target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES_X64} ${ASSIMP_LIBRARIES_X86})
target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES_X86} ${ASSIMP_LIBRARIES_X64})
