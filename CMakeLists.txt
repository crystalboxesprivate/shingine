# Only for macOS for now
cmake_minimum_required(VERSION 3.12)
project(shingine)

if(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
endif()

include_directories(External/include)

file (GLOB_RECURSE cpp_files ${CMAKE_SOURCE_DIR}/Source/*.cpp ${CMAKE_SOURCE_DIR}/Source/*.c)
file (GLOB_RECURSE h_files ${CMAKE_SOURCE_DIR}/Source/*.h)

set (source_files ${cpp_files} ${h_files})
add_executable(shingine ${source_files})

if(WIN32)
target_link_libraries(shingine ${CMAKE_SOURCE_DIR}/External/lib/x64/glfw3.lib)
endif()

if(APPLE)
target_link_libraries(shingine ${CMAKE_SOURCE_DIR}/External/lib/x64/libglfw3.a)
endif()