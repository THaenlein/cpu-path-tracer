###############################################################################
## Add common header files

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/RenderJob.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/SynchronizedQueue.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Uint24.hpp"
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/RenderJob.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/SynchronizedQueue.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Uint24.cpp"
)
