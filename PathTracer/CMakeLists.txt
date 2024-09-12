###############################################################################
## General
cmake_minimum_required(VERSION "3.16")
project("PathTracer")

set(CMAKE_CXX_STANDARD 17) # Required for std::clamp() and filesystem
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(PROJECT_TEST_NAME "${PROJECT_NAME}_test")
set(PROJECT_LIB_NAME "${PROJECT_NAME}_lib")

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
  ${CMAKE_SOURCE_DIR}/src/*.c
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/src/Textures/*.c
  ${CMAKE_SOURCE_DIR}/src/Textures/*.cpp
  ${CMAKE_SOURCE_DIR}/src/Types/*.c
  ${CMAKE_SOURCE_DIR}/src/Types/*.cpp
  ${CMAKE_SOURCE_DIR}/src/Utility/*.c
  ${CMAKE_SOURCE_DIR}/src/Utility/*.cpp)
  
# Add header files
file(GLOB_RECURSE HEADER_FILES 
  ${CMAKE_SOURCE_DIR}/src/*.h
  ${CMAKE_SOURCE_DIR}/src/*.hpp
  ${CMAKE_SOURCE_DIR}/src/Textures/*.h
  ${CMAKE_SOURCE_DIR}/src/Textures/*.hpp
  ${CMAKE_SOURCE_DIR}/src/Types/*.h
  ${CMAKE_SOURCE_DIR}/src/Types/*.hpp
  ${CMAKE_SOURCE_DIR}/src/Utility/*.h
  ${CMAKE_SOURCE_DIR}/src/Utility/*.hpp)

enable_testing()

# Add .lib files
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
#add_executable(${PROJECT_TEST_NAME} ${TEST_HEADERFILES} ${TEST_SOURCEFILES})
add_library(${PROJECT_LIB_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

# Search for additional libraries
find_package(SDL2 REQUIRED)
message(STATUS "Found SDL2 in ${SDL2_INCLUDE_DIR}")

find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

add_library(STB_IMAGE "thirdparty/stb_image.cpp" "thirdparty/stb_image_write.cpp")

set(LIBS SDL2 SDL2main assimp-vc143-mt STB_IMAGE)

# Define the include DIRs
include_directories(
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/tests"
  "${CMAKE_SOURCE_DIR}/include"
)

# Link the libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})
#target_link_libraries(${PROJECT_TEST_NAME} ${LIBS})
target_link_libraries(${PROJECT_LIB_NAME} ${LIBS})

# Copy dlls
# Only for windows
if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  target_compile_options(${PROJECT_NAME} PRIVATE -W4)
  set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
endif()

###############################################################################
## Add compiler flags
#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#  message(STATUS "Clang detected, no additional compile flags.")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#  message(STATUS "GCC detected, adding compile flags.")
#  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -pedantic -DDEBUG -Wextra")
#  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wall")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
#  message(STATUS "Intel detected, no additional compile flags.")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#  message(STATUS "Visual Studio detected, adding compile flags.")
#  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Zi /Ob0 /Od /RTC1 /DDEBUG /W4")
#  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Ot /DNDEBUG /Wall")
#endif()


###############################################################################
## Include additional cmake files
#INCLUDE("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
#INCLUDE("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")

###############################################################################
## Add subdirectories
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/googletest)
