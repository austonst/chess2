# Copyright (c) 2014 Auston Sterling
# See license.txt for copying permission.

cmake_minimum_required(VERSION 2.8)

project(chess2)

# Enable C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Use SDL 2.0
include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

#Set files
set(HDRS
  ./army.hpp
  ./bitboard.hpp
  ./board.hpp
  ./game.hpp
  ./move.hpp
  ./netgame.hpp
  ./piece.hpp
  ./position.hpp
  )

set(SRCS
  ./army.cpp
  ./bitboard.cpp
  ./board.cpp
  ./game.cpp
  ./move.cpp
  ./netgame.cpp
  ./piece.cpp
  ./position.cpp
  ./sdlclient.cpp
  )

# Specify output, includes, and links
include_directories(
  ${SDL2_INCLUDE_DIRS}
  ${SDL2IMAGE_INCLUDE_DIRS}
  )
link_directories(
  ${SDL2_LIBRARY_DIRS}
  ${SDL2IMAGE_LIBRARY_DIRS}
  )
add_executable(chess2-sdl ${HDRS} ${SRCS})
target_link_libraries(chess2-sdl
  ${SDL2_LIBRARIES}
  ${SDL2IMAGE_LIBRARIES}
  )