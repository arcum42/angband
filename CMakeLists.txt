cmake_minimum_required(VERSION 3.13)
project(
  Angband
  VERSION 4.2.1
  LANGUAGES C)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(FATAL_ERROR "This CMake build does not currently support windows.")
endif()

option(SUPPORT_NCURSES_FRONTEND "Support NCurses Frontend." OFF)
option(SUPPORT_SDL_FRONTEND "Support SDL 1.2 Frontend." OFF)
option(SUPPORT_X11_FRONTEND "Support X11 Frontend." ON)
option(SUPPORT_SDL_SOUND "Support Sound System with SDL." OFF)

# By default, X11 is the frontend. Set to off if another frontend is specified.
if(SUPPORT_NCURSES_FRONTEND OR SUPPORT_SDL_FRONTEND)
  set(SUPPORT_X11_FRONTEND OFF)
endif()

if(NOT SUPPORT_SDL_FRONTEND)
  set(SUPPORT_SDL_SOUND OFF)
endif()

add_executable(Angband "")
set_target_properties(Angband PROPERTIES C_STANDARD 99)
add_subdirectory(src)

include(CURSES_Frontend)
include(X11_Frontend)
include(SDL_Frontend)
include(SDL_Sound)

if(SUPPORT_X11_FRONTEND)
  configure_x11_frontend(Angband)
elseif(SUPPORT_NCURSES_FRONTEND)
  configure_curses_frontend(Angband)
elseif(SUPPORT_SDL_FRONTEND)
  configure_sdl_frontend(Angband)
  if(SUPPORT_SDL_SOUND)
    configure_sdl_sound(Angband)
  endif()
endif()

#[[
For Linux Systems the compilation without the libm library
throw undefined references to mathematical functions, this requirement
is only present in Linux Systems.

Wikipedia say: Under Linux and BSD, the mathematical functions
(as declared in <math.h>) are bundled separately in the mathematical
library libm. Therefore, if any of those functions are used, the linker
must be given the directive -lm.

Ref: https://en.wikipedia.org/wiki/C_mathematical_functions#libm
]]
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_link_libraries(Angband PRIVATE $<$<PLATFORM_ID:Linux>:m>)
endif()

message(STATUS "Moving files needed")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
