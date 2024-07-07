
add_library(page INTERFACE page.hpp)
target_include_directories(page INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lru INTERFACE lru.hpp)
target_include_directories(lru INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})