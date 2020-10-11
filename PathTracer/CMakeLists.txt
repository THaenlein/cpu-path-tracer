###############################################################################
## General
cmake_minimum_required(VERSION "3.17.2")
project("PathTracer")

set(CMAKE_CXX_STANDARD 17) # For std::clamp()
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/lib)
set(ASSIMP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/include)
set(ASSIMP_CODE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/code)
#set(ASSIMP_LIBRARIES_DIR_X86 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x86)
#set(ASSIMP_LIBRARIES_DIR_X64 ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1/lib/x64)
set(ASSIMP_LIBRARIES_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/assimp-5.0.1/code)
set(ASSIMP_LIBRARIES assimp-vc141-mtd)
#set(ASSIMP_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/assimp-5.0.1)

enable_testing()

###############################################################################
## Add compiler flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(STATUS "Clang detected, no additional compile flags.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "GCC detected, adding compile flags.")
	#target_compile_options("${PROJECT_NAME}" PRIVATE -pedantic -Wall -Wextra -O0)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -pedantic -DDEBUG -Wextra")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	message(STATUS "Intel detected, no additional compile flags.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	message(STATUS "Visual Studio detected, adding compile flags.")
	#target_compile_options("${PROJECT_NAME}" PRIVATE -W4 -Od)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Zi /Ob0 /Od /RTC1 /DDEBUG /W4")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Ot /DNDEBUG /Wall")
endif()

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
## Find packages
find_package(SDL2 REQUIRED)
#find_package(ASSIMP REQUIRED)

###############################################################################
## Include directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/tests)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/stbi)
include_directories("${PROJECT_NAME}" ${SDL2_INCLUDE_DIRS})
include_directories("${PROJECT_NAME}" ${ASSIMP_INCLUDE_DIR})
include_directories("${PROJECT_NAME}" ${ASSIMP_CODE_DIR})

###############################################################################
## Link libraries
set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
target_link_libraries("${PROJECT_NAME}"                                         ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
target_link_libraries("${PROJECT_LIB_NAME}"                                     ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC "${PROJECT_LIB_NAME}" gtest ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
