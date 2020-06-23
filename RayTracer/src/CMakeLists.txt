###############################################################################
## Add common header files

set(PROJECT_LIB_NAME "${PROJECT_NAME}_lib")

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/main.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Application.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/RayTracer.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Timer.hpp"

  "${CMAKE_CURRENT_LIST_DIR}/exceptions.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/raytracing.hpp"
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Application.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/RayTracer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Timer.cpp"
)

###############################################################################
## Include subdirectories

INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Types/CMakeLists.txt")

###############################################################################
## Add executable

add_executable("${PROJECT_NAME}" ${SOURCEFILES} ${HEADERFILES})
add_library("${PROJECT_LIB_NAME}" STATIC ${SOURCEFILES} ${HEADERFILES})

###############################################################################
## Add compiler flags

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(STATUS "Clang detected, no additional compile flags.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "GCC detected, adding compile flags.")
	target_compile_options("${PROJECT_NAME}" PRIVATE -pedantic -Wall -Wextra -O0)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	message(STATUS "Intel detected, no additional compile flags.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	message(STATUS "Visual Studio detected, adding compile flags.")
	target_compile_options("${PROJECT_NAME}" PRIVATE -W4 -Od)
endif()