###############################################################################
## General
set(PROJECT_TEST_NAME "${PROJECT_NAME}_test")

###############################################################################
## Add test header files
LIST(APPEND TEST_HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/main.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestRayTracer.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestBoundingBox.hpp"
)

###############################################################################
## Add test source files
LIST(APPEND TEST_SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestRayTracer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TestBoundingBox.cpp"
)

###############################################################################
## Add executable
add_executable("${PROJECT_TEST_NAME}" ${TEST_SOURCEFILES} ${TEST_HEADERFILES})

add_test(
	NAME "${PROJECT_TEST_NAME}"
	COMMAND "${PROJECT_TEST_NAME}"
)