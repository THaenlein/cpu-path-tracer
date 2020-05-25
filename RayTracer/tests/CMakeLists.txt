###############################################################################
## General
set(PROJECT_TEST_NAME "${PROJECT_NAME}_test")

###############################################################################
## Add test header files
LIST(APPEND TEST_HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/TestRayTracer.hpp"
)

###############################################################################
## Add test source files
LIST(APPEND TEST_SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/TestRayTracer.cpp"
)

###############################################################################
## Add executable
add_executable("${PROJECT_TEST_NAME}" ${TEST_SOURCEFILES} ${TEST_HEADERFILES})
target_link_libraries("${PROJECT_TEST_NAME}" PUBLIC gtest_main)

add_test(
	NAME "${PROJECT_TEST_NAME}"
	COMMAND "${PROJECT_TEST_NAME}"
)