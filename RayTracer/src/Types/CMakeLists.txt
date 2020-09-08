###############################################################################
## Add common header files

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/AccelerationStructure.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/BoundingBox.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/RenderJob.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/SynchronizedQueue.hpp"
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/AccelerationStructure.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/BoundingBox.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/RenderJob.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/SynchronizedQueue.cpp"
)
