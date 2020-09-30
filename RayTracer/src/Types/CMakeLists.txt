###############################################################################
## Add common header files

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/AccelerationStructure.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/BoundingBox.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/BoundingVolume.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/KdNode.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/KdTree.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Material.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Plane.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/RenderJob.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/SynchronizedQueue.hpp"
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/AccelerationStructure.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/BoundingBox.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/BoundingVolume.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/KdNode.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/KdTree.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Material.cpp"
)
