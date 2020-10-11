###############################################################################
## Add common header files

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/ArgParser.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/mathUtility.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/materialUtility.hpp"
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/ArgParser.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/mathUtility.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/materialUtility.cpp"
)
