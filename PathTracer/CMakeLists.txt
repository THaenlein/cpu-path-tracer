###############################################################################
## General
cmake_minimum_required(VERSION "3.16")
project("PathTracer")

set(CMAKE_CXX_STANDARD 17) # Required for std::clamp() and filesystem
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")
set(ASSIMP_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1)
set(SDL2_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/SDL2-2.0.14)

set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
enable_testing()

###############################################################################
## Add compiler flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(STATUS "Clang detected, no additional compile flags.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "GCC detected, adding compile flags.")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -pedantic -DDEBUG -Wextra")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	message(STATUS "Intel detected, no additional compile flags.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	message(STATUS "Visual Studio detected, adding compile flags.")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Zi /Ob0 /Od /RTC1 /DDEBUG /W4")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Ot /DNDEBUG /Wall")
endif()

###############################################################################
## Include additional cmake files
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")

###############################################################################
## Add subdirectories
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/googletest)

###############################################################################
## Find packages
find_package(SDL2 REQUIRED)
find_package(assimp REQUIRED)

###############################################################################
## Include directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/tests)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/stbi)
include_directories("${PROJECT_NAME}" ${SDL2_INCLUDE_DIRS})
include_directories("${PROJECT_NAME}" ${ASSIMP_INCLUDE_DIR})

###############################################################################
## Link libraries
if(WIN32)
	target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} ${ASSIMP_LIBRARY})
	target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} ${ASSIMP_LIBRARY})
	target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} ${ASSIMP_LIBRARY})
elseif(UNIX AND NOT APPLE)
	target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} ${assimp_LIBRARIES} stdc++fs pthread)
	target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} ${assimp_LIBRARIES} stdc++fs pthread)
	target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} ${assimp_LIBRARIES} stdc++fs pthread)
endif()

###############################################################################
## Platform specific
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(PLATFORM_ARCH x64)
else ()
  set(PLATFORM_ARCH x86)
endif ()

if(WIN32)
	add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${ASSIMP_ROOT_DIR}/lib/${PLATFORM_ARCH}/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll"
            "${CMAKE_BINARY_DIR}/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll")
    add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${ASSIMP_ROOT_DIR}/lib/${PLATFORM_ARCH}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll"
            "${CMAKE_BINARY_DIR}/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.dll")
    
    add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SDL2_DIR}/lib/${PLATFORM_ARCH}"
            "${CMAKE_BINARY_DIR}/Debug")
    add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SDL2_DIR}/lib/${PLATFORM_ARCH}"
            "${CMAKE_BINARY_DIR}/Release")
endif()