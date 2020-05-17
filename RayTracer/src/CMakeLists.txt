###############################################################################
## Add common header files

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/main.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Application.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/ErrorHandler.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/RayTracer.hpp"

  "${CMAKE_CURRENT_LIST_DIR}/exceptions.hpp"
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Application.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/ErrorHandler.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/RayTracer.cpp"
)
