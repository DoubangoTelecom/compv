/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_COMMON_H_)
#define _COMPV_BASE_COMMON_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_allocators.h"

#include <vector>
#include <algorithm>

COMPV_NAMESPACE_BEGIN()

#define COMPV_CAT_(A, B) A ## B
#define COMPV_CAT(A, B) COMPV_CAT_(A, B)
#define COMPV_STRING_(A) #A
#define COMPV_STRING(A) COMPV_STRING_(A)

#define COMPV_VERSION_MAJOR 1
#define COMPV_VERSION_MINOR 0
#define COMPV_VERSION_MICRO 0
#if !defined(COMPV_VERSION_STRING)
#	define COMPV_VERSION_STRING COMPV_STRING(COMPV_CAT(COMPV_VERSION_MAJOR, .)) COMPV_STRING(COMPV_CAT(COMPV_VERSION_MINOR, .)) COMPV_STRING(COMPV_VERSION_MICRO)
#endif

#if !defined(COMPV_SAFE_DELETE_CPP)
#	define COMPV_SAFE_DELETE_CPP(cpp_obj) if(cpp_obj) delete (cpp_obj), (cpp_obj) = NULL;
#endif /* COMPV_SAFE_DELETE_CPP */

#if defined(NDEBUG)
#	define COMPV_ASSERT(x) do { bool __COMPV_b_ret = (x); if (!__COMPV_b_ret) { COMPV_DEBUG_FATAL("Assertion failed!"); abort(); } } while(0)
#else
#	define COMPV_ASSERT(x) do { bool __COMPV_b_ret = (x); assert(__COMPV_b_ret); } while(0)
#endif

#if COMPV_INTRINSIC
#	define COMPV_EXEC_IFDEF_INTRIN(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN(EXPR)
#endif
#if COMPV_INTRINSIC && COMPV_ARCH_X86
#	define COMPV_EXEC_IFDEF_INTRIN_X86(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN_X86(EXPR)
#endif
#if COMPV_INTRINSIC && COMPV_ARCH_ARM
#	define COMPV_EXEC_IFDEF_INTRIN_ARM(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN_ARM(EXPR)
#endif
#if COMPV_INTRINSIC && COMPV_ARCH_ARM64
#	define COMPV_EXEC_IFDEF_INTRIN_ARM64(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN_ARM64(EXPR)
#endif
#if COMPV_ASM
#	define COMPV_EXEC_IFDEF_ASM(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_X64
#	define COMPV_EXEC_IFDEF_ASM_X64(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_X64(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_X86
#	define COMPV_EXEC_IFDEF_ASM_X86(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_X86(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_ARM32
#	define COMPV_EXEC_IFDEF_ASM_ARM32(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_ARM32(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_ARM64
#	define COMPV_EXEC_IFDEF_ASM_ARM64(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_ARM64(EXPR)
#endif
#if COMPV_HAVE_INTEL_IPP
#	define COMPV_EXEC_IFDEF_INTEL_IPP(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntelIppEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTEL_IPP(EXPR)
#endif

#define COMPV_NUM_THREADS_SINGLE	1
#define COMPV_NUM_THREADS_MULTI		-1

#if !defined(COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET)
#	define COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET 32
#endif

#if !defined(COMPV_PLANE_MAX_COUNT)
#	define COMPV_PLANE_MAX_COUNT		4
#endif /* COMPV_PLANE_MAX_COUNT */
#define COMPV_PLANE_Y		0
#define COMPV_PLANE_U		1
#define COMPV_PLANE_V		2
#define COMPV_PLANE_UV		1

// Fixed point Q value
#if COMPV_ARCH_ARM
#	define COMPV_FXPQ	15
#else
#	define COMPV_FXPQ	16
#endif

#define COMPV_IS_ALIGNED(p, a) (!((uintptr_t)(p) & ((a) - 1)))
#define COMPV_IS_ALIGNED_MMX(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_MMX)
#define COMPV_IS_ALIGNED_SSE(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_SSE)
#define COMPV_IS_ALIGNED_AVX(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_AVX)
#define COMPV_IS_ALIGNED_AVX2(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_AVX2)
#define COMPV_IS_ALIGNED_AVX512(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_AVX512)
#define COMPV_IS_ALIGNED_NEON(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_NEON)
#define COMPV_IS_ALIGNED_DEFAULT(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_DEFAULT)

#define COMPV_ALIGNED(x)
#define COMPV_ALIGNED_DEFAULT(x)
#define COMPV_ALIGN_DEFAULT() COMPV_ALIGN(COMPV_ALIGNV_SIMD_DEFAULT)
#define COMPV_ALIGN_AVX() COMPV_ALIGN(COMPV_ALIGNV_SIMD_AVX)
#define COMPV_ALIGN_AVX2() COMPV_ALIGN(COMPV_ALIGNV_SIMD_AVX2)
#define COMPV_ALIGN_SSE() COMPV_ALIGN(COMPV_ALIGNV_SIMD_SSE)
#define COMPV_ALIGN_MMX() COMPV_ALIGN(COMPV_ALIGNV_SIMD_MMX)
#define COMPV_ALIGN_NEON() COMPV_ALIGN(COMPV_ALIGNV_SIMD_NEON)

#define COMPV_DEFAULT_ARG(arg_, val_) arg_

#define COMPV_IS_POW2(x) (((x) != 0) && !((x) & ((x) - 1)))

#define CompVPtrDef(T)			CompVPtr<T* >

#define CompVPtrBox(T)			CompVPtrDef(CompVBox<T >)
#define CompVPtrBoxPoint(T)		CompVPtrDef(CompVBox<CompVPoint<T > >)

#define CompVPtrBoxNew(T)		CompVBox<T >::newObj
#define CompVPtrBoxPointNew(T)	CompVBox<CompVPoint<T > >::newObj

#if defined(_MSC_VER)
#	define snprintf		_snprintf
#	define vsnprintf	_vsnprintf
#	define strdup		_strdup
#	define stricmp		_stricmp
#	define strnicmp		_strnicmp
#else
#	if !HAVE_STRNICMP && !HAVE_STRICMP
#	define stricmp		strcasecmp
#	define strnicmp		strncasecmp
#	endif
#endif

// Should be defined in <inttypes.h>, include in <compv_config.h>
#if !defined(PRIu64)
#	define	PRIu64 "llu"
#endif

/*******************************************************/
/* MACRO for shuffle parameter for _mm_shuffle_ps().   */
/* Argument fp3 is a digit[0123] that represents the fp*/
/* from argument "b" of mm_shuffle_ps that will be     */
/* placed in fp3 of result. fp2 is the same for fp2 in */
/* result. fp1 is a digit[0123] that represents the fp */
/* from argument "a" of mm_shuffle_ps that will be     */
/* places in fp1 of result. fp0 is the same for fp0 of */
/* result                                              */
/*******************************************************/
#define COMPV_MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

/*
Macro to build arg32 values for _mm(256/128)_shuffle_epi8().
a,b,c,d must be <= 16 for _mm128_shuffle_epi8() and <32 for _mm256_shuffle_epi8()
*/
#define COMPV_MM_SHUFFLE_EPI8(fp3,fp2,fp1,fp0) (((fp3) << 24) | ((fp2) << 16) | ((fp1) << 8) | ((fp0) & 0xFF))

typedef int32_t compv_core_id_t;
typedef intptr_t compv_scalar_t;  /* This type *must* have the width of a general-purpose register on the target CPU. 64bits or 32bits. */
typedef uintptr_t compv_uscalar_t;  /* This type *must* have the width of a general-purpose register on the target CPU. 64bits or 32bits. */
typedef float compv_float32_t;
typedef double compv_float64_t;
typedef compv_float32_t compv_float32x2_t[2];
typedef compv_float32_t compv_float32x3_t[3];
typedef compv_float32_t compv_float32x4_t[4];
typedef compv_float64_t compv_float64x3_t[3];
typedef uint8_t compv_uint8x3_t[3];
typedef uint8_t compv_uint8x4_t[4];
typedef uint8_t compv_uint8x256_t[256];
typedef std::vector<std::string> CompVVecString;
typedef std::vector<int32_t> CompVVec32s;

enum COMPV_DEBUG_LEVEL {
	COMPV_DEBUG_LEVEL_VERBOSE = 5,
    COMPV_DEBUG_LEVEL_INFO = 4,
    COMPV_DEBUG_LEVEL_WARN = 3,
    COMPV_DEBUG_LEVEL_ERROR = 2,
    COMPV_DEBUG_LEVEL_FATAL = 1,
};

#define kErrorCodeSuccessStart		0
#define kErrorCodeWarnStart			10000
#define kErrorCodeErrorStart		(kErrorCodeWarnStart << 1)
#define kErrorCodeFatalStart		(kErrorCodeErrorStart << 1)

// TODO(dmi) complete COMPVGetErrorString(code) with all the newly added codes
enum COMPV_ERROR_CODE {
    COMPV_ERROR_CODE_S_OK = kErrorCodeSuccessStart,

    COMPV_ERROR_CODE_W = kErrorCodeWarnStart,
    COMPV_ERROR_CODE_W_WINDOW_CLOSED,

    COMPV_ERROR_CODE_E = kErrorCodeErrorStart,
    COMPV_ERROR_CODE_E_NOT_IMPLEMENTED,
    COMPV_ERROR_CODE_E_NOT_INITIALIZED,
    COMPV_ERROR_CODE_E_NOT_FOUND,
    COMPV_ERROR_CODE_E_INVALID_CALL,
    COMPV_ERROR_CODE_E_INVALID_STATE,
    COMPV_ERROR_CODE_E_INVALID_PARAMETER,
    COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT,
    COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT,
	COMPV_ERROR_CODE_E_INVALID_SUBTYPE,
    COMPV_ERROR_CODE_E_FAILED_TO_OPEN_FILE,
    COMPV_ERROR_CODE_E_FAILED_TO_READ_FILE,
	COMPV_ERROR_CODE_E_END_OF_FILE,
    COMPV_ERROR_CODE_E_OUT_OF_MEMORY,
    COMPV_ERROR_CODE_E_OUT_OF_BOUND,
    COMPV_ERROR_CODE_E_DECODER_NOT_FOUND,
    COMPV_ERROR_CODE_E_FILE_NOT_FOUND,
    COMPV_ERROR_CODE_E_TIMEDOUT,
    COMPV_ERROR_CODE_E_UNITTEST_FAILED,
    COMPV_ERROR_CODE_E_SYSTEM,
	COMPV_ERROR_CODE_E_MEMORY_LEAK,
	COMPV_ERROR_CODE_E_MEMORY_NOT_ALIGNED,
    COMPV_ERROR_CODE_E_THIRD_PARTY_LIB,
    COMPV_ERROR_CODE_E_PTHREAD,
	COMPV_ERROR_CODE_E_THREAD_FORKING,
	COMPV_ERROR_CODE_E_RECURSIVE_CALL,
	COMPV_ERROR_CODE_E_JNI,
    COMPV_ERROR_CODE_E_DIRECTSHOW,
    COMPV_ERROR_CODE_E_MFOUNDATION,
    COMPV_ERROR_CODE_E_EGL,
    COMPV_ERROR_CODE_E_GL,
    COMPV_ERROR_CODE_E_GL_NO_CONTEXT,
    COMPV_ERROR_CODE_E_GLEW,
    COMPV_ERROR_CODE_E_GLFW,
	COMPV_ERROR_CODE_E_OPENCL,
	COMPV_ERROR_CODE_E_CUDA,
    COMPV_ERROR_CODE_E_SDL,
	COMPV_ERROR_CODE_E_SKIA,
	COMPV_ERROR_CODE_E_INTEL_IPP,
	COMPV_ERROR_CODE_E_INTEL_TBB,
	COMPV_ERROR_CODE_E_FFMPEG,
	COMPV_ERROR_CODE_E_FREETYPE,
	COMPV_ERROR_CODE_E_STBI,

    COMPV_ERROR_CODE_F = kErrorCodeFatalStart,
};
extern COMPV_BASE_API const char* CompVGetErrorString(COMPV_NAMESPACE::COMPV_ERROR_CODE code);

#define COMPV_ERROR_CODE_IS_SUCCESS(code_) ((code_) < kErrorCodeWarnStart)
#define COMPV_ERROR_CODE_IS_OK(code_) COMPV_ERROR_CODE_IS_SUCCESS((code_))
#define COMPV_ERROR_CODE_IS_FAILURE(code_) (!COMPV_ERROR_CODE_IS_SUCCESS((code_)))
#define COMPV_ERROR_CODE_IS_NOK(code_) COMPV_ERROR_CODE_IS_FAILURE((code_))
#define COMPV_ERROR_CODE_IS_WARN(code_) ((code_) >= kErrorCodeWarnStart && (code_) < kErrorCodeErrorStart)
#define COMPV_ERROR_CODE_IS_ERROR(code_) ((code_) >= kErrorCodeErrorStart && (code_) < kErrorCodeFatalStart)
#define COMPV_ERROR_CODE_IS_FATAL(code_) ((code_) >= kErrorCodeFatalStart)

// In COMPV_CHECK_HR(errcode) When (errcode) is a function it will be executed twice when used in "COMPV_DEBUG_ERROR(errcode)" and "If(errcode)"
#define COMPV_CHECK_CODE_NOP(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); } } while(0)
#define COMPV_CHECK_CODE_BAIL(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); goto bail; } } while(0)
#define COMPV_CHECK_CODE_RETURN(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); return __code__; } } while(0)
#define COMPV_CHECK_CODE_ASSERT(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); COMPV_ASSERT(false); } } while(0)
#define COMPV_CHECK_EXP_NOP(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_NOP(errcode,  __VA_ARGS__); } while(0)
#define COMPV_CHECK_EXP_BAIL(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_BAIL(errcode,  __VA_ARGS__); } while(0)
#define COMPV_CHECK_EXP_RETURN(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_RETURN(errcode,  __VA_ARGS__); } while(0)
#define COMPV_CHECK_EXP_ASSERT(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_ASSERT(errcode,  __VA_ARGS__); } while(0)

enum COMPV_SORT_TYPE {
    COMPV_SORT_TYPE_BUBBLE, /**< https://en.wikipedia.org/wiki/Bubble_sort */
    COMPV_SORT_TYPE_QUICK, /**< https://en.wikipedia.org/wiki/Quicksort */
};

enum COMPV_BORDER_TYPE {
	COMPV_BORDER_TYPE_ZERO,
	COMPV_BORDER_TYPE_IGNORE,
	COMPV_BORDER_TYPE_REPLICATE
};

enum COMPV_MODELEST_TYPE {
	COMPV_MODELEST_TYPE_NONE,
	COMPV_MODELEST_TYPE_RANSAC
};

enum COMPV_INTERPOLATION_TYPE {
	COMPV_INTERPOLATION_TYPE_BILINEAR,
	COMPV_INTERPOLATION_TYPE_NEAREST,
};

enum COMPV_MAT_TYPE {
    COMPV_MAT_TYPE_RAW,
	COMPV_MAT_TYPE_STRUCT,
    COMPV_MAT_TYPE_PIXELS
};

enum COMPV_SUBTYPE {
	COMPV_SUBTYPE_NONE,

    COMPV_SUBTYPE_RAW_OPAQUE,
	COMPV_SUBTYPE_RAW_INT8,
	COMPV_SUBTYPE_RAW_UINT8,
	COMPV_SUBTYPE_RAW_INT16,
	COMPV_SUBTYPE_RAW_UINT16,
	COMPV_SUBTYPE_RAW_INT32,
	COMPV_SUBTYPE_RAW_UINT32,
	COMPV_SUBTYPE_RAW_SIZE,
	COMPV_SUBTYPE_RAW_FLOAT32,
	COMPV_SUBTYPE_RAW_FLOAT64,
	COMPV_SUBTYPE_RAW_USCALAR,
	COMPV_SUBTYPE_RAW_SCALAR,

    COMPV_SUBTYPE_PIXELS_RGB24,			// RGB24
    COMPV_SUBTYPE_PIXELS_BGR24,			// BGR8
    COMPV_SUBTYPE_PIXELS_RGBA32,		// RGB32
    COMPV_SUBTYPE_PIXELS_BGRA32,		// BGRA32
    COMPV_SUBTYPE_PIXELS_ABGR32,		// ABGR32
    COMPV_SUBTYPE_PIXELS_ARGB32,		// ARGB32
	COMPV_SUBTYPE_PIXELS_RGB565LE,		// RGB565LE: Microsoft: DirectDraw with dwFlags = DDPF_RGB and dwRGBBitCount = 16, use by android bitmaps and wince consumers
	COMPV_SUBTYPE_PIXELS_RGB565BE,		// RGB565BE
	COMPV_SUBTYPE_PIXELS_BGR565LE,		// BGR565LE
	COMPV_SUBTYPE_PIXELS_BGR565BE,		// BGR565BE
	COMPV_SUBTYPE_PIXELS_HSV,			// HSV/HSB: Hue Saturation Value/Brightness: http://codeitdown.com/hsl-hsb-hsv-color/
	COMPV_SUBTYPE_PIXELS_HSL,			// HSL: Hue Saturation Lightness: http://codeitdown.com/hsl-hsb-hsv-color/
    COMPV_SUBTYPE_PIXELS_Y,				// Y-only: Grayscale
	COMPV_SUBTYPE_PIXELS_NV12,			// Microsoft: NV12, iOS camera, Android camera, Semi-Planar, Info: https://www.fourcc.org/pixel-format/yuv-nv12/
	COMPV_SUBTYPE_PIXELS_NV21,			// Microsoft: NV21, Android camera, Semi-Planar, Info: https://www.fourcc.org/pixel-format/yuv-nv21/
    COMPV_SUBTYPE_PIXELS_YUV420P,		// Microsoft: I420, Planar, Info: https://www.fourcc.org/pixel-format/yuv-i420/
	COMPV_SUBTYPE_PIXELS_YVU420P,		// Microsoft YV12, Planar, Info: https://www.fourcc.org/pixel-format/yuv-yv12/
	COMPV_SUBTYPE_PIXELS_YUV422P,		// Microsoft: YUV422, Planar
	COMPV_SUBTYPE_PIXELS_YUYV422,		// Microsoft: YUY2, V4L2/DirectShow preferred format, Packed, Info: https://www.fourcc.org/pixel-format/yuv-yuy2/
	COMPV_SUBTYPE_PIXELS_UYVY422,		// Microsoft: UYVY, iOS camera, Info: https://www.fourcc.org/pixel-format/yuv-uyvy/
	COMPV_SUBTYPE_PIXELS_YUV444P,		// Microsoft: YUV444, Planar

	// Aliases
	COMPV_SUBTYPE_PIXELS_RGB565 = COMPV_SUBTYPE_PIXELS_RGB565LE,
	COMPV_SUBTYPE_PIXELS_BGR565 = COMPV_SUBTYPE_PIXELS_BGR565LE,
    COMPV_SUBTYPE_PIXELS_I420 = COMPV_SUBTYPE_PIXELS_YUV420P,
	COMPV_SUBTYPE_PIXELS_IYUV = COMPV_SUBTYPE_PIXELS_YUV420P,
	COMPV_SUBTYPE_PIXELS_YUV422 = COMPV_SUBTYPE_PIXELS_UYVY422,
	COMPV_SUBTYPE_PIXELS_YUV444 = COMPV_SUBTYPE_PIXELS_YUV444P,
	COMPV_SUBTYPE_PIXELS_YUY2 = COMPV_SUBTYPE_PIXELS_YUYV422,
	COMPV_SUBTYPE_PIXELS_UYVY = COMPV_SUBTYPE_PIXELS_UYVY422,
	COMPV_SUBTYPE_PIXELS_Y422 = COMPV_SUBTYPE_PIXELS_UYVY,
	COMPV_SUBTYPE_PIXELS_Y420SP = COMPV_SUBTYPE_PIXELS_NV21,
	COMPV_SUBTYPE_PIXELS_GRAY = COMPV_SUBTYPE_PIXELS_Y,
	COMPV_SUBTYPE_PIXELS_HSB = COMPV_SUBTYPE_PIXELS_HSV,
};
extern COMPV_BASE_API const char* CompVGetSubtypeString(COMPV_NAMESPACE::COMPV_SUBTYPE subtype);

enum COMPV_IMAGE_FORMAT {
    COMPV_IMAGE_FORMAT_NONE,
    COMPV_IMAGE_FORMAT_RAW,
    COMPV_IMAGE_FORMAT_JPEG,
    COMPV_IMAGE_FORMAT_JPG = COMPV_IMAGE_FORMAT_JPEG,
    COMPV_IMAGE_FORMAT_BMP,
    COMPV_IMAGE_FORMAT_BITMAP = COMPV_IMAGE_FORMAT_BMP,
    COMPV_IMAGE_FORMAT_PNG
};

enum COMPV_MATH_PARABOLA_TYPE {
	COMPV_MATH_PARABOLA_TYPE_REGULAR, // y = ax^2 + bx + c
	COMPV_MATH_PARABOLA_TYPE_SIDEWAYS // x = ay^2 + by + c -> http://www.coolmath.com/algebra/21-advanced-graphing/05-sideways-parabolas-01
};

// Mathematical morphology: Structuring element type
// https://en.wikipedia.org/wiki/Mathematical_morphology
enum COMPV_MATH_MORPH_STREL_TYPE {
	COMPV_MATH_MORPH_STREL_TYPE_RECT,
	COMPV_MATH_MORPH_STREL_TYPE_DIAMOND,
	COMPV_MATH_MORPH_STREL_TYPE_CROSS
};

// Mathematical morphology: Basic operation type
// https://en.wikipedia.org/wiki/Mathematical_morphology
enum COMPV_MATH_MORPH_OP_TYPE {
	COMPV_MATH_MORPH_OP_TYPE_ERODE, // https://en.wikipedia.org/wiki/Erosion_(morphology)
	COMPV_MATH_MORPH_OP_TYPE_DILATE, // https://en.wikipedia.org/wiki/Dilation_(morphology)
	COMPV_MATH_MORPH_OP_TYPE_OPEN, // https://en.wikipedia.org/wiki/Opening_(morphology)
	COMPV_MATH_MORPH_OP_TYPE_CLOSE, // https://en.wikipedia.org/wiki/Closing_(morphology)
	COMPV_MATH_MORPH_OP_TYPE_GRADIENT, // https://en.wikipedia.org/wiki/Morphological_gradient
	COMPV_MATH_MORPH_OP_TYPE_TOPHAT, // https://en.wikipedia.org/wiki/Top-hat_transform
	COMPV_MATH_MORPH_OP_TYPE_BLACKHAT,
	COMPV_MATH_MORPH_OP_TYPE_HITMISS // https://en.wikipedia.org/wiki/Hit-or-miss_transform
};

enum COMPV_DRAWING_COLOR_TYPE {
	COMPV_DRAWING_COLOR_TYPE_STATIC,
	COMPV_DRAWING_COLOR_TYPE_RANDOM
};

enum COMPV_DRAWING_LINE_TYPE {
	COMPV_DRAWING_LINE_TYPE_SIMPLE,
	COMPV_DRAWING_LINE_TYPE_MATCH,
};

enum COMPV_DRAWING_LINE_CONNECT {
	COMPV_DRAWING_LINE_CONNECT_NONE, // GL_LINES: Vertices 0 and 1 are considered a line. Vertices 2 and 3 are considered a line. And so on. If the user specifies a non-even number of vertices, then the extra vertex is ignored.
	COMPV_DRAWING_LINE_CONNECT_STRIP, // GL_LINE_STRIP: The adjacent vertices are considered lines. Thus, if you pass n vertices, you will get n-1 lines. If the user only specifies 1 vertex, the drawing command is ignored.
	COMPV_DRAWING_LINE_CONNECT_LOOP // GL_LINE_LOOP: As line strips, except that the first and last vertices are also used as a line. Thus, you get n lines for n input vertices. If the user only specifies 1 vertex, the drawing command is ignored. The line between the first and last vertices happens after all of the previous lines in the sequence.
};

struct CompVImageInfo {
    COMPV_IMAGE_FORMAT format;
    COMPV_SUBTYPE pixelFormat; // COMPV_SUBTYPE_PIXELS_XXX
    int32_t width;
    int32_t stride;
    int32_t height;
public:
    CompVImageInfo() : format(COMPV_IMAGE_FORMAT_RAW), pixelFormat(COMPV_SUBTYPE_NONE), width(0), stride(0), height(0) { }
};

struct CompVVec3f {
public:
    union {
        struct {
			compv_float32_t x, y, z;
        };
        struct {
			compv_float32_t r, g, b;
        };
        struct {
			compv_float32_t s, t, p;
        };
    };
    CompVVec3f(compv_float32_t x_, compv_float32_t y_, compv_float32_t z_) : x(x_), y(y_), z(z_) { }
};

struct CompVRatio {
public:
    int numerator;
    int denominator;
    CompVRatio(int numerator_ = 1, int denominator_ = 1) : numerator(numerator_), denominator(denominator_) { }
	bool operator==(const CompVRatio &other) const {
		return numerator == other.numerator && denominator == other.denominator;
	}
};

template <typename T>
struct CompVPoint {
public:
	CompVPoint(T x_ = 0, T y_ = 0, T z_ = 1) {
		x = x_, y = y_, z = z_;
	}
	bool operator==(const CompVPoint &other) const {
		return x == other.x && y == other.y && z == other.z;
	}
	T x, y, z;
};
typedef CompVPoint<compv_float32_t> CompVPointFloat32;
typedef CompVPoint<compv_float64_t> CompVPointFloat64;
typedef CompVPoint<int32_t> CompVPointInt32;
typedef CompVPoint<int16_t> CompVPointInt16;
typedef CompVPoint<int> CompVPointInt;
typedef CompVPoint<size_t> CompVPointSz;
typedef std::vector<CompVPointFloat32, CompVAllocatorNoDefaultConstruct<CompVPointFloat32> > CompVPointFloat32Vector;
typedef std::vector<CompVPointFloat64, CompVAllocatorNoDefaultConstruct<CompVPointFloat64> > CompVPointFloat64Vector;
typedef std::vector<CompVPointInt32, CompVAllocatorNoDefaultConstruct<CompVPointInt32> > CompVPointInt32Vector;
typedef std::vector<CompVPointInt, CompVAllocatorNoDefaultConstruct<CompVPointInt> > CompVPointIntVector;
typedef std::vector<CompVPointSz, CompVAllocatorNoDefaultConstruct<CompVPointSz> > CompVPointSzVector;

template <typename T>
struct CompVPoint2D {
public:
	CompVPoint2D(T x_ = 0, T y_ = 0) {
		x = x_, y = y_;
	}
	bool operator==(const CompVPoint2D &other) const {
		return x == other.x && y == other.y;
	}
	T x, y;
};
typedef CompVPoint2D<compv_float32_t> CompVPoint2DFloat32;
typedef CompVPoint2D<int16_t> CompVPoint2DInt16;
typedef std::vector<CompVPoint2DFloat32, CompVAllocatorNoDefaultConstruct<CompVPoint2DFloat32> > CompVPoint2DFloat32Vector;
typedef std::vector<CompVPoint2DInt16, CompVAllocatorNoDefaultConstruct<CompVPoint2DInt16> > CompVPoint2DInt16Vector;

template <typename T>
struct CompVRect {
public:
	T left;
	T top;
	T right;
	T bottom;
	CompVRect(T left_ = 0, T top_ = 0, T right_ = 0, T bottom_ = 0) : left(left_), top(top_), right(right_), bottom(bottom_) {  }
	bool operator==(const CompVRect<T> &other) const {
		return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
	}
	static CompVRect makeFromWidthHeight(T x, T y, T width, T height) {
		return CompVRect(x, y, x + width, y + height);
	}
	COMPV_ALWAYS_INLINE bool isEmpty()const { return (left == right) && (top == bottom); }
	COMPV_ALWAYS_INLINE T width()const { return (right - left) + 1; }
	COMPV_ALWAYS_INLINE T height()const { return (bottom - top) + 1; }
	COMPV_ALWAYS_INLINE bool contains(const CompVPoint2D<T>& point)const {
		return (left <= point.x && point.x <= right && top <= point.y && point.y <= bottom);
	}
	COMPV_ALWAYS_INLINE bool contains(const CompVRect<T>& other)const {
		return ((other.left >= left) && (other.right <= right) && (other.top >= top) && (other.bottom <= bottom));
	}
	COMPV_ALWAYS_INLINE bool overlap(const CompVRect<T>& other)const {
		return ((left <= other.right) && (right >= other.left) && (top <= other.bottom) && (bottom >= other.top));
	}
};
typedef CompVRect<compv_float32_t> CompVRectFloat32;
typedef CompVRect<compv_float64_t> CompVRectFloat64;
typedef CompVRect<int32_t> CompVRectInt32;
typedef CompVRect<int16_t> CompVRectInt16;
typedef CompVRect<int> CompVRectInt;
typedef std::vector<CompVRectFloat32, CompVAllocatorNoDefaultConstruct<CompVRectFloat32> > CompVRectFloat32Vector;
typedef std::vector<CompVRectFloat32, CompVAllocatorNoDefaultConstruct<CompVRectFloat32> > CompVRectFloat32Vector;
typedef std::vector<CompVRectInt32, CompVAllocatorNoDefaultConstruct<CompVRectInt32> > CompVRectInt32Vector;
typedef std::vector<CompVRectInt16, CompVAllocatorNoDefaultConstruct<CompVRectInt16> > CompVRectInt16Vector;
typedef std::vector<CompVRectInt, CompVAllocatorNoDefaultConstruct<CompVRectInt> > CompVRectIntVector;

typedef CompVPointFloat32 CompVQuadrilateralFloat32[4];
typedef CompVPointFloat64 CompVQuadrilateralFloat64[4];
typedef CompVPointInt32 CompVQuadrilateralInt32[4];
typedef CompVPointInt16 CompVQuadrilateralInt16[4];
typedef CompVPointInt CompVQuadrilateralInt[4];
typedef CompVPointSz CompVQuadrilateralSz[4];
typedef std::vector<CompVQuadrilateralFloat32, CompVAllocatorNoDefaultConstruct<CompVQuadrilateralFloat32> > CompVQuadrilateralFloat32Vector;
typedef std::vector<CompVQuadrilateralFloat64, CompVAllocatorNoDefaultConstruct<CompVQuadrilateralFloat64> > CompVQuadrilateralFloat64Vector;
typedef std::vector<CompVQuadrilateralInt32, CompVAllocatorNoDefaultConstruct<CompVQuadrilateralInt32> > CompVQuadrilateralInt32Vector;
typedef std::vector<CompVQuadrilateralInt16, CompVAllocatorNoDefaultConstruct<CompVQuadrilateralInt16> > CompVQuadrilateralInt16Vector;
typedef std::vector<CompVQuadrilateralInt, CompVAllocatorNoDefaultConstruct<CompVQuadrilateralInt> > CompVQuadrilateralIntVector;
typedef std::vector<CompVQuadrilateralSz, CompVAllocatorNoDefaultConstruct<CompVQuadrilateralSz> > CompVQuadrilateralSzVector;

template <typename T>
struct CompVSize {
public:
	CompVSize(T width_ = 0, T height_ = 0) {
		width = width_, height = height_;
	}
	bool operator==(const CompVSize &other) const {
		return width == other.width && height == other.height;
	}
	T width, height;
};
typedef CompVSize<compv_float32_t> CompVSizeFloat32;
typedef CompVSize<compv_float64_t> CompVSizeFloat64;
typedef CompVSize<int32_t> CompVSizeInt32;
typedef CompVSize<int16_t> CompVSizeInt16;
typedef CompVSize<int> CompVSizeInt;
typedef CompVSize<size_t> CompVSizeSz;

template <typename T>
struct CompVRange {
public:
	CompVRange(T start_ = 0, T end_ = 0) {
		start = start_, end = end_;
	}
	bool operator==(const CompVRange &other) const {
		return start == other.start && end == other.end;
	}
	T start, end;
};
typedef CompVRange<compv_float32_t> CompVRangeFloat32;
typedef CompVRange<compv_float64_t> CompVRangeFloat64;
typedef CompVRange<int32_t> CompVRangeInt32;
typedef CompVRange<int16_t> CompVRangeInt16;
typedef CompVRange<int> CompVRangeInt;
typedef CompVRange<size_t> CompVRangeSz;

template <typename T>
struct CompVLine {
	CompVPoint<T> a, b;
public:
	CompVLine(const CompVPoint<T>& a_, const CompVPoint<T>& b_): a(a_), b(b_) { }
	CompVLine() { }
};
typedef CompVLine<compv_float32_t> CompVLineFloat32;
typedef CompVLine<compv_float64_t> CompVLineFloat64;
typedef CompVLine<int32_t> CompVLineInt32;
typedef CompVLine<int> CompVLineInt;
typedef std::vector<CompVLineFloat32, CompVAllocatorNoDefaultConstruct<CompVLineFloat32> > CompVLineFloat32Vector;
typedef std::vector<CompVLineFloat64, CompVAllocatorNoDefaultConstruct<CompVLineFloat64> > CompVLineFloat64Vector;
typedef std::vector<CompVLineInt32, CompVAllocatorNoDefaultConstruct<CompVLineInt32> > CompVLineInt32Vector;
typedef std::vector<CompVLineInt, CompVAllocatorNoDefaultConstruct<CompVLineInt> > CompVLineIntVector;

typedef std::vector<struct CompVInterestPoint, CompVAllocatorNoDefaultConstruct<struct CompVInterestPoint> > CompVInterestPointVector;
struct CompVInterestPoint {
	compv_float32_t x; /**< Point.x */
	compv_float32_t y; /**< Point.y */
	compv_float32_t strength; /**< Corner/edge strength/response (e.g. FAST response or Harris response) */
	compv_float32_t orient; /**< angle in degree ([0-360]) */
	int level; /**< pyramid level (when image is scaled, level0 is the first one) */
	compv_float32_t size; /**< patch size (e.g. BRIEF patch size-circle diameter-) */

public:
	CompVInterestPoint(compv_float32_t x_ = 0.f, compv_float32_t y_ = 0.f, compv_float32_t strength_ = -1.f, compv_float32_t orient_ = -1.f, int32_t level_ = 0, compv_float32_t size_ = 0.f) {
		x = x_, y = y_, strength = strength_, orient = orient_, level = level_, size = size_;
	}
	static void selectBest(CompVInterestPointVector& interestPoints, size_t max) {
		if (max > 1) {
#if 0
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("Slow and worst matches");
			std::sort(interestPoints.begin(), interestPoints.end(), InterestPointStrengthGreater());
			interestPoints.resize(static_cast<size_t>(max));
#else
			// This code gives better matches (tested with object recognition sample)
			std::nth_element(interestPoints.begin(), interestPoints.begin() + max, interestPoints.end(),
				[](const CompVInterestPoint& i, const CompVInterestPoint& j) {return i.strength > j.strength; });
			const float pivot = interestPoints.at(max - 1).strength;
			interestPoints.resize(std::partition(interestPoints.begin() + max, interestPoints.end(),
				[pivot](CompVInterestPoint i) { return i.strength >= pivot; }) - interestPoints.begin());
#endif
		}
	}
	static void eraseTooCloseToBorder(CompVInterestPointVector& interestPoints, size_t img_width, size_t img_height, int border_size) {
		float w = static_cast<compv_float32_t>(img_width), h = static_cast<compv_float32_t>(img_height), b = static_cast<compv_float32_t>(border_size);
		auto new_end = std::remove_if(interestPoints.begin(), interestPoints.end(), [&w, &h, &b](const CompVInterestPoint& p) { 
			return ((p.x < b || (p.x + b) >= w || (p.y < b) || (p.y + b) >= h));
		});
		interestPoints.erase(new_end, interestPoints.end());
	}
};

struct CompVMatIndex {
	size_t row;
	size_t col;
public:
	CompVMatIndex(size_t row_ = 0, size_t col_ = 0): row(row_), col(col_) {}
};
typedef std::vector<CompVMatIndex, CompVAllocatorNoDefaultConstruct<CompVMatIndex> > CompVMatIndexVector;

struct CompVDMatch {
	int queryIdx;
	int trainIdx;
	int imageIdx;
	int distance;
public:
	CompVDMatch(int queryIdx_ = 0, int trainIdx_ = 0, int distance_ = 0, int imageIdx_ = 0) {
		queryIdx = queryIdx_, trainIdx = trainIdx_, distance = distance_, imageIdx = imageIdx_;
	}
};
typedef std::vector<CompVDMatch, CompVAllocatorNoDefaultConstruct<CompVInterestPoint> > CompVDMatchVector;

struct CompVHoughLine {
	compv_float32_t rho;
	compv_float32_t theta;
	size_t strength;
public:
	CompVHoughLine(compv_float32_t rho_ = 0.f, compv_float32_t theta_ = 0.f, size_t strength_ = 0): rho(rho_), theta(theta_), strength(strength_) { }
};
typedef std::vector<CompVHoughLine, CompVAllocatorNoDefaultConstruct<CompVHoughLine> > CompVHoughLineVector;

struct CompVDrawingOptions {
	COMPV_DRAWING_COLOR_TYPE colorType = COMPV_DRAWING_COLOR_TYPE_RANDOM;
	compv_float32x4_t color = {1.f, 1.f, 1.f, 1.f};
	compv_float32_t pointSize = 7.f;
	compv_float32_t lineWidth = 2.f;
	COMPV_DRAWING_LINE_TYPE lineType = COMPV_DRAWING_LINE_TYPE_SIMPLE;
	COMPV_DRAWING_LINE_CONNECT lineConnect = COMPV_DRAWING_LINE_CONNECT_NONE;
	size_t fontSize = 16; // Pixel Size (FreeType)
	std::string fontFullPath; // Full path to the font (e.g. "C:/Windows/Fonts/arial.ttf") - on Android or iOS, to retrieve the full path (from the assets/bundle), use 'COMPV_PATH_FROM_NAME' (a.k.a 'CompVFileUtils::getFullPathFromFileName')
	bool fontUtf8 = false; // Whether to consider the string passed to drawTexts() as utf8 or not
public:
	COMPV_INLINE void setColor(const compv_float32x4_t& c) {
		color[0] = c[0], color[1] = c[1], color[2] = c[2], color[3] = c[3];
	}
	static CompVDrawingOptions clone(const CompVDrawingOptions* options) {
		return options ? *options : CompVDrawingOptions();
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_COMMON_H_ */
