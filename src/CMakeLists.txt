link_directories(${SDL_LIBRARY_DIR})
include_directories(${SDL_INCLUDE_DIR})

add_executable(escher escher.cpp)
target_link_libraries(escher ${SDL_LIBRARY})
