set(CMAKE_VERBOSE_MAKEFILE on)

## Enable C/C++/ASM ##
enable_language(CXX C ASM)

## C++11 ##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # no "-std=gnu++11" needed

## Set Libs build type (STATIC or SHARED) ##
if (NOT LIB_BUILD_TYPE)
	if (NOT TARGET_SUPPORTS_SHARED_LIBS) # some embedded OSes don't support shared libs
		set(LIB_BUILD_TYPE STATIC)
	else()
		set(LIB_BUILD_TYPE SHARED)
		set(CMAKE_CXX_VISIBILITY_PRESET hidden) # "-fvisibility=hidden"
	endif()
endif()
if ("${LIB_BUILD_TYPE}" MATCHES "SHARED")
	set(LIB_LINK_SCOPE PRIVATE)
  else()
	set(LIB_LINK_SCOPE PUBLIC)
endif()

## Set Libs build scheme (Debug or Release) ##
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

## Set default and common build and link flags ##
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE " ${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_ASM_FLAGS "-c -x assembler-with-cpp -Wall -Werror -Wextra -Wfatal-errors")
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-dead_strip")
else()
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,-gc-sections")
endif()
if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "None")
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--strip-all")
endif()

## Detect TARGET_ARCH ##
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i386|i686|amd64|x86_64|AMD64")
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(TARGET_ARCH "x64")
	else(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(TARGET_ARCH "x86")
	endif()
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^aarch64")
	set(TARGET_ARCH "arm64")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^arm")
	set(TARGET_ARCH "arm32")
else()
	message(FATAL_ERROR "Not a valid processor" ${CMAKE_SYSTEM_PROCESSOR})
endif()

## COMPILER and LINKER FLAGS ##
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flax-vector-conversions -fvisibility=hidden -Wno-pragmas")
    if (NOT IOS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
    endif ()
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
	# Intel C++
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
	# Visual Studio C++
endif()

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=_WIN32_WINNT_VISTA")
endif()

# Raspberry


# NVIDIA Jetson
if ("${TARGET_OS}" MATCHES "JETSON")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_OS_JETSON=1")
    if ("${JETSON_TFTRT}" MATCHES "YES")
	   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_HAVE_TFTRT=1")
	endif()
endif ()

# ARM cross compilation
if ("${TARGET_ARCH}" MATCHES "arm64")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ARM_NEON__ -ftree-vectorize -funsafe-math-optimizations -march=armv8-a")
elseif ("${TARGET_ARCH}" MATCHES "arm32")
	if ("${TARGET_OS}" MATCHES "PI")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMPV_OS_PI=1 -mfpu=neon-vfpv4 -funsafe-math-optimizations")
	else()
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ARM_NEON__ -ftree-vectorize -funsafe-math-optimizations -march=armv7-a")
	endif()
endif ()


## Detect SIMD flags ##
if (MSVC)
	set(FLAGS_SSE "/arch:SSE")
	set(FLAGS_SSE2 "/arch:SSE2")
	set(FLAGS_SSSE3 "${FLAGS_SSE2}")
	set(FLAGS_SSE41 "${FLAGS_SSE2}")
	set(FLAGS_SSE42 "${FLAGS_SSE2}")
	set(FLAGS_FMA "")
	set(FLAGS_BMI1 "")
	set(FLAGS_BMI2 "")
	set(FLAGS_AVX "/arch:AVX")
	set(FLAGS_AVX2 "/arch:AVX2")

	set(FLAGS_NEON "/arch:NEON")
else()
	set(FLAGS_SSE "-msse")
	set(FLAGS_SSE2 "-msse2")
	set(FLAGS_SSSE3 "-mssse3")
	set(FLAGS_SSE41 "-msse4.1")
	set(FLAGS_SSE42 "-msse4.2")
	set(FLAGS_FMA "-mfma -D__FMA3__")
	set(FLAGS_BMI1 "-mbmi")
	set(FLAGS_BMI2 "-mbmi2")
	set(FLAGS_AVX "-mavx")
	set(FLAGS_AVX2 "-mavx2 ${FLAGS_FMA}")

	if ("${TARGET_OS}" MATCHES "PI") # we know that all rpi3+ devices support vfpv4 
		set(FLAGS_NEON "-mfpu=neon-vfpv4")
		set(FLAGS_NEONFMA "-mfpu=neon-vfpv4")
	elseif ("${TARGET_ARCH}" MATCHES "arm32") # ARM64/aarch64-linux-gnu: unrecognized command line option '-mfpu=neon'
		set(FLAGS_NEON "-mfpu=neon")
		set(FLAGS_NEONFMA "-mfpu=neon-vfpv4")
	endif()

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

## Objects extension ##
if (MSVC)
	set(OBJECT_EXT obj)
else ()
	set(OBJECT_EXT o)
endif()

## Helper macro to build ASM/YASM objects ##
macro(set_ASM_OBJECTS)
	if ("${TARGET_ARCH}" STREQUAL "x86" OR "${TARGET_ARCH}" STREQUAL "x64")
		foreach (file ${ARGN})
			get_filename_component(file_name ${file} NAME_WE)
			get_filename_component(file_dir ${file} DIRECTORY)
			set(ASM_OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/${file_dir}/${file_name}.${OBJECT_EXT})
			set(ASM_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${file_dir}/${file_name}.s)
			add_custom_command(OUTPUT ${ASM_OBJECT} DEPENDS ${file} COMMAND "${YASM_EXE}" ${YASM_ARGS} "${ASM_FILE}" -o "${ASM_OBJECT}")
			set(ASM_OBJECTS ${ASM_OBJECTS} ${ASM_OBJECT})
		endforeach()
	else ()
		set(ASM_OBJECTS ${ASM_OBJECTS} ${ARGV})
	endif()
endmacro()

## Helper macro to set compiler options for intrin files ##
macro(set_INTRIN_COMPILE_FLAGS)
	foreach (file ${ARGN})
		get_filename_component(file_name ${file} NAME)
		set(CURRENT_FILE_COMPILE_FLAGS )
		if (${file_name} MATCHES "_intrin_(.*_)?sse2.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_SSE2}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?ssse3.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_SSSE3}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?sse41.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_SSE41}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?sse42.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_SSE42}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?avx.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_AVX}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?avx2.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_AVX2}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?avx_fma.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_AVX} ${FLAGS_FMA}")
		elseif (${file_name} MATCHES "_intrin_(.*_)?neon(64)?.cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_NEON}")
		endif ()
		
		# Append others
		if (${file_name} MATCHES "_intrin_fma(3|4)_(.*).cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_FMA}")
		endif ()
		if (${file_name} MATCHES "_intrin_bmi1(2)?_(.*).cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_BMI1}")
		endif ()
		if (${file_name} MATCHES "_intrin_bmi(1)?2_(.*).cxx")
			set(CURRENT_FILE_COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS} ${FLAGS_BMI2}")
		endif ()

		set_source_files_properties(${file} APPEND PROPERTY COMPILE_FLAGS "${CURRENT_FILE_COMPILE_FLAGS}")
	endforeach()
endmacro()

## Helper macro to add contribs ##
macro(add_contrib src_dir project_name)
	if (NOT TARGET ${project_name})
		ExternalProject_Add (${project_name}
			UPDATE_COMMAND ""
			DOWNLOAD_COMMAND ""
			PREFIX "${CMAKE_BINARY_DIR}/contrib"
			SOURCE_DIR "${src_dir}"
			CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/contrib"
			BINARY_DIR "${CMAKE_BINARY_DIR}/contrib/${project_name}"
		)
	endif ()
endmacro()

## Helper macro to check if project already included ##
macro(target_check_return_if_exist target_name)
	if (TARGET ${target_name})
		return()
	endif()
endmacro()
