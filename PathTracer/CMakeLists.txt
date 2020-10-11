###############################################################################
## General
cmake_minimum_required(VERSION "3.17.2")
project("PathTracer")

set(CMAKE_CXX_STANDARD 17) # For std::clamp()
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/lib)
set(SDL2_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sdl2/include)
set(ASSIMP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/include)
set(ASSIMP_CODE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/code)
#set(ASSIMP_INCLUDE_DIR2 ${CMAKE_CURRENT_BINARY_DIR}/lib/assimp-5.0.1/include/assimp)
#set(ASSIMP_LIBRARIES_DIR_X86 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x86)
#set(ASSIMP_LIBRARIES_DIR_X64 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x64)
set(ASSIMP_LIBRARIES_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/assimp-5.0.1/code)
set(ASSIMP_LIBRARIES assimp-vc141-mtd)
#set(ASSIMP_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1)

set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1 /DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG")

enable_testing()

###############################################################################
## Link directories
link_directories(${ASSIMP_LIBRARIES_DIR})

###############################################################################
## Include additional cmake files
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")

###############################################################################
## Add subdirectories
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/googletest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1)

###############################################################################
## Include directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/tests)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/stbi)
include_directories("${PROJECT_NAME}" ${SDL2_INCLUDE_DIR})
include_directories("${PROJECT_NAME}" ${ASSIMP_INCLUDE_DIR})
include_directories("${PROJECT_NAME}" ${ASSIMP_CODE_DIR})

###############################################################################
## Find packages
find_package(SDL2 REQUIRED)

###############################################################################
## Link libraries
set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
