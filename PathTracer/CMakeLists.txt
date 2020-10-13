###############################################################################
## General
cmake_minimum_required(VERSION "3.17.2")
project("PathTracer")

set(CMAKE_CXX_STANDARD 17) # Required for std::clamp() and filesystem
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/ ${CMAKE_CURRENT_LIST_DIR}/lib/sdl2/)
set(ASSIMP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/include)

set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
enable_testing()

###############################################################################
## Platform specific
if(WIN32)
	set(ASSIMP_LIB_SEARCH_DIR ${CMAKE_CURRENT_LIST_DIR}/prebuiltLibs/x86/win32/)
	set(ASSIMP_LIBRARIES_DEBUG assimp-vc141-mtd)
	set(ASSIMP_LIBRARIES_RELEASE assimp-vc141-mt)
elseif(UNIX AND NOT APPLE)
	set(ASSIMP_LIB_SEARCH_DIR ${CMAKE_CURRENT_LIST_DIR}/prebuiltLibs/x86/unix/)
	set(ASSIMP_LIBRARIES_DEBUG assimp-vc141-mtd)  #TODO
	set(ASSIMP_LIBRARIES_RELEASE assimp-vc141-mt) #TODO
endif()

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
## Link directories
link_directories(${ASSIMP_LIB_SEARCH_DIR})

###############################################################################
## Include additional cmake files
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")

###############################################################################
## Add subdirectories
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/googletest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/sdl2)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1)

###############################################################################
## Find packages
find_package(SDL2 REQUIRED)

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
	target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} debug ${ASSIMP_LIBRARIES_DEBUG} optimized ${ASSIMP_LIBRARIES_RELEASE})
	target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} debug ${ASSIMP_LIBRARIES_DEBUG} optimized ${ASSIMP_LIBRARIES_RELEASE})
	target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} debug ${ASSIMP_LIBRARIES_DEBUG} optimized ${ASSIMP_LIBRARIES_RELEASE})
elseif(UNIX AND NOT APPLE)
	target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES} stdc++fs)
	target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES} stdc++fs)
	target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES} stdc++fs)
endif()

###############################################################################
## Custom commands
add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSIMP_LIB_SEARCH_DIR}/debug"
        "${CMAKE_BINARY_DIR}/Debug")
add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSIMP_LIB_SEARCH_DIR}/release"
        "${CMAKE_BINARY_DIR}/Release")