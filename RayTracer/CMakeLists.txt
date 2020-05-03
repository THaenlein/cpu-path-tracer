###############################################################################
## General
cmake_minimum_required(VERSION "3.17.2")
project("RayTracer")

set(CMAKE_CXX_STANDARD 17) # TODO: Verify version
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
enable_testing()

###############################################################################
## Add subdirectories
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
#add_subdirectory(src)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/googletest)

add_library(core ${HEADERFILES} ${SOURCEFILES})
target_include_directories(core PUBLIC ${HEADERFILES}) # TODO: Check use of list instead of dir is ok. 


###############################################################################
## Add executable
add_executable("${PROJECT_NAME}" ${SOURCEFILES})
target_link_libraries("${PROJECT_NAME}" core)

###############################################################################
## Add tests
add_subdirectory(tests)