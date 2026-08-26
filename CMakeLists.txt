cmake_minimum_required(VERSION 3.7)

project(ft2-clone)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ft2-clone_SOURCE_DIR}/release/other/")

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
	add_compile_options(-O3)
	add_compile_options(-ffast-math)
	add_compile_options(-fno-strict-aliasing)
endif()

set(FT2_IRIX FALSE)
if(CMAKE_SYSTEM_NAME MATCHES "^IRIX")
	set(FT2_IRIX TRUE)
endif()

# The standard SGUG-RSE SDL2 package has no OpenGL renderer support (software renderer only).
# SGUG-FLX ships an SDL2 build that does, but has to be manually enabled (cmake -DFT2_IRIX_USE_SGUGFLX_SDL2=ON)
option(FT2_IRIX_USE_SGUGFLX_SDL2 "On IRIX, use the SGUG-FLX SDL2 build (adds OpenGL renderer support) instead of the standard SGUG-RSE SDL2 (software renderer only)" OFF)

set(FT2_USE_SGUGFLX_SDL2 FALSE)
if(FT2_IRIX AND FT2_IRIX_USE_SGUGFLX_SDL2)
	set(FT2_USE_SGUGFLX_SDL2 TRUE)
endif()

if(FT2_USE_SGUGFLX_SDL2)
	message(STATUS "IRIX: using SGUG-FLX SDL2 (with OpenGL renderer support)")

	# Priority 1: /usr/sgug-flx/lib32/. Priority 2: a bundled ./lib/ next to the binary.
	set(SGUGFLX_SDL2 "/usr/sgug-flx/lib32/libSDL2-2.0.so.0")
	if(EXISTS "${SGUGFLX_SDL2}")
		set(FT2_SDL2_LIB "${SGUGFLX_SDL2}")
	elseif(EXISTS "${CMAKE_SOURCE_DIR}/lib/libSDL2-2.0.so.0")
		set(FT2_SDL2_LIB "${CMAKE_SOURCE_DIR}/lib/libSDL2-2.0.so.0")
	else()
		message(FATAL_ERROR "SDL2 not found in /usr/sgug-flx/lib32 or ${CMAKE_SOURCE_DIR}/lib")
	endif()

	message(STATUS "Using SDL2: ${FT2_SDL2_LIB}")
	link_libraries("${FT2_SDL2_LIB}")
	link_libraries(GLcore audio pthread)

	add_compile_options(
		-I/usr/sgug/lib/gcc/mips-sgi-irix6.5/9/include
		-I/usr/include
		-I/usr/sgug/include
		-I/usr/sgug/include/SDL2
		-D_GNU_SOURCE=1
		-D_SGI_MP_SOURCE
	)
	add_link_options(
		-Wl,--disable-new-dtags
		-Wl,-rpath,$ORIGIN/lib
		-Wl,--allow-shlib-undefined
		-Wl,-rpath-link=/usr/lib32
		-Wl,-rpath=/usr/sgug-flx/lib32:./lib:../lib:/usr/lib32:/usr/sgug/lib32
	)
else()
	find_package(SDL2 REQUIRED)
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

find_package(Threads REQUIRED)

if(FT2_USE_SGUGFLX_SDL2)
	# SDL2 (+ GLcore/audio/pthread) is already linked directory-wide above via link_libraries().
	target_link_libraries(ft2-clone PRIVATE m Threads::Threads)
	target_compile_definitions(ft2-clone PRIVATE FT2_SDL2_HAS_GL_RENDERER)
else()
	target_include_directories(ft2-clone SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})

	if("${SDL2_LIBRARIES}" STREQUAL "")
		message(WARNING "SDL2_LIBRARIES wasn't set, manually setting to SDL2::SDL2")
		set(SDL2_LIBRARIES "SDL2::SDL2")
	endif()

	target_link_libraries(ft2-clone PRIVATE m Threads::Threads ${SDL2_LIBRARIES})
endif()

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
