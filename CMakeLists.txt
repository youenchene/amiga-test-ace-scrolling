cmake_minimum_required(VERSION 3.14.0)
project(hello LANGUAGES C)

# Lowercase project name for binaries and packaging
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

if(NOT AMIGA)
	message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAMIGA -Wall -Wextra -fomit-frame-pointer")
file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE HEADERS src/*.h)

include_directories(${PROJECT_SOURCE_DIR}/src)

# Debugging flag exposed via CMake specifically for your game code
if(GAME_DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGAME_DEBUG")
endif()


set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)

# ACE
set(ACE_DEBUG ON)
set(ACE_DEBUG_UAE ON)
add_subdirectory(deps/ace ace)
include_directories(deps/ace/include)
# If you built standalone and installed ACE, use following instead:
# find_package(ace REQUIRED)
# include_directories(${ace_INCLUDE_DIRS})

# Force lower-case binary name for Linux etc.
set(TARGET_NAME ${PROJECT_NAME_LOWER})

if(ELF2HUNK)
  # Add elf2hunk step for Bartman compiler
  set(GAME_LINKED ${TARGET_NAME}.elf) # Intermediate executable
  set(GAME_EXE ${TARGET_NAME}.exe) # Use this to launch the game
  add_executable(${GAME_LINKED} ${SOURCES} ${HEADERS})
  add_custom_command(
    TARGET ${GAME_LINKED} POST_BUILD
    COMMAND ${ELF2HUNK} ${GAME_LINKED} ${GAME_EXE}
  )
else()
  # Just produce the executable with Bebbo compiler
  SET(GAME_LINKED ${TARGET_NAME})
  SET(GAME_EXE ${TARGET_NAME})
  add_executable(${GAME_LINKED} ${SOURCES} ${HEADERS})
endif()

target_link_libraries(${GAME_LINKED} ace)


# Create directory for converted files next to built executable
file(MAKE_DIRECTORY ${DATA_DIR})
file(MAKE_DIRECTORY ${DATA_DIR}/fonts)

# Convert palette and background image
convertPalette(${GAME_LINKED} ${RES_DIR}/overworld.gpl ${DATA_DIR}/overworld.plt)
convertBitmaps(
  TARGET ${GAME_LINKED} PALETTE ${RES_DIR}/overworld.gpl
  INTERLEAVED SOURCES ${RES_DIR}/overworld.png
  DESTINATIONS ${DATA_DIR}/overworld.bm
)

file(COPY ${RES_DIR}/overworld.dat DESTINATION ${DATA_DIR})
file(COPY ${RES_DIR}/fonts/silkscreen.fnt DESTINATION ${DATA_DIR}/fonts)


