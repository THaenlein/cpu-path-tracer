###############################################################################
## Add common header files

LIST(APPEND HEADERFILES
  "${CMAKE_CURRENT_LIST_DIR}/CheckerTexture.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/Texture.hpp"  
)

###############################################################################
## Add common source files

LIST(APPEND SOURCEFILES
  "${CMAKE_CURRENT_LIST_DIR}/CheckerTexture.cpp"
)
