cmake_minimum_required(VERSION 3.7)

project(ft2-clone)

find_package(SDL2 REQUIRED)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ft2-clone_SOURCE_DIR}/release/other/")

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
	add_compile_options(-O3)
	add_compile_options(-ffast-math)
endif()

set(FT2_IRIX FALSE)
if(CMAKE_SYSTEM_NAME MATCHES "IRIX")
	set(FT2_IRIX TRUE)
endif()

file(GLOB ft2-clone_SRC
	"${ft2-clone_SOURCE_DIR}/src/*.c"
	"${ft2-clone_SOURCE_DIR}/src/gfxdata/*.c"
	"${ft2-clone_SOURCE_DIR}/src/mixer/*.c"
	"${ft2-clone_SOURCE_DIR}/src/scopes/*.c"
	"${ft2-clone_SOURCE_DIR}/src/modloaders/*.c"
	"${ft2-clone_SOURCE_DIR}/src/smploaders/*.c"
)

if(NOT FT2_IRIX)
	file(GLOB ft2-clone_MIDI_SRC "${ft2-clone_SOURCE_DIR}/src/rtmidi/*.cpp")
	list(APPEND ft2-clone_SRC ${ft2-clone_MIDI_SRC})
endif()

add_executable(ft2-clone ${ft2-clone_SRC})
target_include_directories(ft2-clone SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})

if("${SDL2_LIBRARIES}" STREQUAL "")
	message(WARNING "SDL2_LIBRARIES wasn't set, manually setting to SDL2::SDL2")
	set(SDL2_LIBRARIES "SDL2::SDL2")
endif()

find_package(Threads REQUIRED)
target_link_libraries(ft2-clone PRIVATE m Threads::Threads ${SDL2_LIBRARIES})

if(NOT FT2_IRIX)
	target_compile_definitions(ft2-clone PRIVATE HAS_MIDI)
endif()

if(UNIX)
	if(APPLE)
		find_library(COREAUDIO CoreAudio REQUIRED)
		find_library(COREFOUNDATION CoreFoundation REQUIRED)
		find_library(COREMIDI CoreMIDI REQUIRED)
		find_library(ICONV iconv REQUIRED)
		target_link_libraries(ft2-clone PRIVATE ${COREAUDIO} ${COREMIDI} ${COREFOUNDATION} ${ICONV})
		target_compile_definitions(ft2-clone PRIVATE __MACOSX_CORE__)
	elseif(FT2_IRIX)
		# No ALSA, no MIDI backend on IRIX.
		# Add iconv for IRIX
		find_library(ICONV iconv REQUIRED)
		target_link_libraries(ft2-clone PRIVATE ${ICONV})
	else()
		target_link_libraries(ft2-clone PRIVATE asound)
		target_compile_definitions(ft2-clone PRIVATE __LINUX_ALSA__)
	endif()
endif()

install(TARGETS ft2-clone RUNTIME DESTINATION bin)
