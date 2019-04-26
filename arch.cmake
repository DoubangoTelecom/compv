## Detect TARGET_ARCH ##
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i386|i686|amd64|x86_64|AMD64")
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(TARGET_ARCH "x64")
	else(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(TARGET_ARCH "x86")
	endif()
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^arm")
	set(TARGET_ARCH "arm32")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^aarch64")
	set(TARGET_ARCH "arm64")
else()
	message(FATAL_ERROR "Not a valid processor" ${CMAKE_SYSTEM_PROCESSOR})
endif()

## COMPILER FLAGS (http://www.brianlheim.com/2018/04/09/cmake-cheat-sheet.html) ##
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	set(COMPILER_CXX_FLAGS "${COMPILER_CXX_FLAGS} -Wno-pragmas -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	set(COMPILER_CXX_FLAGS "${COMPILER_CXX_FLAGS} -Wno-pragmas -std=gnu++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
	# Intel C++
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
	# Visual Studio C++
endif()

## Detect SIMD flags ##
if (MSVC)
	set(FLAGS_SSE "/arch:SSE")
	set(FLAGS_SSE2 "/arch:SSE2")
	set(FLAGS_SSSE3 "${FLAGS_SSE2}")
	set(FLAGS_SSE41 "${FLAGS_SSE2}")
	set(FLAGS_SSE42 "${FLAGS_SSE2}")
	set(FLAGS_AVX "/arch:AVX")
	set(FLAGS_AVX2 "/arch:AVX2")

	set(FLAGS_NEON "/arch:NEON")
else()
	set(FLAGS_SSE "-msse")
	set(FLAGS_SSE2 "-msse2")
	set(FLAGS_SSSE3 "-mssse3")
	set(FLAGS_SSE41 "-msse4.1")
	set(FLAGS_SSE42 "-msse4.2")
	set(FLAGS_AVX "-mavx")
	set(FLAGS_AVX2 "-mavx2 -mfma")

	set(FLAGS_NEON "-mfpu=neon")
endif(MSVC)


## Detect YASM exe and args ##
if ("${TARGET_ARCH}" STREQUAL "x86" OR "${TARGET_ARCH}" STREQUAL "x64")
	## Detect YASM ##
	if (MSVC)
		find_program(YASM_EXE NAMES vsyasm) # prefere vsyasm for Visual studio
	endif ()
	if (NOT YASM_EXE)
		find_program(YASM_EXE NAMES yasm)
	endif ()
	if (NOT YASM_EXE)
		message(FATAL_ERROR "YASM is required to build assembler. https://yasm.tortall.net/")
	endif ()

	## YASM ARGS ##
	set(YASM_ARGS -Xvc -rnasm -pnasm)
	if ("${TARGET_ARCH}" STREQUAL "x64")
		if (WIN32)
			set(YASM_ARGS ${YASM_ARGS} -f win64 -d WINDOWS)
        elseif (APPLE)
            set(YASM_ARGS ${YASM_ARGS} -f macho64)
		else ()
			set(YASM_ARGS ${YASM_ARGS} -f elf64)
		endif ()
	else ()
		if (WIN32)
			set(YASM_ARGS ${YASM_ARGS} -f win32 -d WINDOWS)
        elseif (APPLE)
            set(YASM_ARGS ${YASM_ARGS} -f macho32)
		else ()
			set(YASM_ARGS ${YASM_ARGS} -f elf32)
		endif ()
	endif ()

endif()
