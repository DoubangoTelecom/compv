/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_morph.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_basic_ops.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/image/compv_image.h"

#include "compv/base/math/intrin/x86/compv_math_morph_intrin_avx2.h"
#include "compv/base/math/intrin/x86/compv_math_morph_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_morph_intrin_neon.h"

// Mathematical morphology: https://en.wikipedia.org/wiki/Mathematical_morphology

#define COMPV_MATH_MORPH_BASIC_OPER_SAMPLES_PER_THREAD			(10 * 10)
#define COMPV_MATH_MORPH_OPEN_CLOSE_OPER_SAMPLES_PER_THREAD		(COMPV_MATH_MORPH_BASIC_OPER_SAMPLES_PER_THREAD >> (1+1))

#define COMPV_THIS_CLASSNAME	"CompVMathMorph"

COMPV_NAMESPACE_BEGIN()

// X64
#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathMorphProcessErode_8u_Asm_X64_SSE2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathMorphProcessDilate_8u_Asm_X64_SSE2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathMorphProcessErode_8u_Asm_X64_AVX2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathMorphProcessDilate_8u_Asm_X64_AVX2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

// ARM32
#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathMorphProcessErode_8u_Asm_NEON32(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathMorphProcessDilate_8u_Asm_NEON32(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

// ARM64
#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathMorphProcessErode_8u_Asm_NEON64(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathMorphProcessDilate_8u_Asm_NEON64(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM64 */


#define CompVMathMorphT	uint8_t // default type

template<typename T>
class CompVMathMorphOpMin { /* Op for Erode: https://en.wikipedia.org/wiki/Erosion_(morphology) */
public:
	COMPV_ALWAYS_INLINE T operator()(const T& x, const T& y) const { return COMPV_MATH_MIN(x, y); }
};
typedef CompVMathMorphOpMin<uint8_t> CompVMathMorphOpErode8u;
typedef CompVMathMorphOpMin<compv_float32_t> CompVMathMorphOpErode32f;

template<typename T>
class CompVMathMorphOpMax { /* Op for Dilate: https://en.wikipedia.org/wiki/Dilation_(morphology) */
public:
	COMPV_ALWAYS_INLINE T operator()(const T& x, const T& y) const { return COMPV_MATH_MAX(x, y); }
};
typedef CompVMathMorphOpMax<uint8_t> CompVMathMorphOpDilate8u;
typedef CompVMathMorphOpMax<compv_float32_t> CompVMathMorphOpDilate32f;

template <typename T, class CompVMathMorphOp>
static COMPV_ERROR_CODE basicOper(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_BORDER_TYPE borderTypeRightLeft, COMPV_BORDER_TYPE borderTypeTop, COMPV_BORDER_TYPE borderTypeBottom);

template <typename T, class CompVMathMorphOp1, class CompVMathMorphOp2>
static COMPV_ERROR_CODE openCloseOper(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_BORDER_TYPE borderTypeRightLeft, COMPV_BORDER_TYPE borderTypeTop, COMPV_BORDER_TYPE borderTypeBottom);

template <typename T>
static COMPV_ERROR_CODE gradientOper(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_BORDER_TYPE borderTypeRightLeft, COMPV_BORDER_TYPE borderTypeTop, COMPV_BORDER_TYPE borderTypeBottom);

template <typename T>
static COMPV_ERROR_CODE buildStructuringElementInputPtrs(const CompVMatPtr& strel, const CompVMatPtr& input, CompVMatPtrPtr strelInputPtrs);

template <typename T>
static COMPV_ERROR_CODE buildStructuringElementGeneric(CompVMatPtrPtr strel, const CompVSizeSz size, COMPV_MATH_MORPH_STREL_TYPE type);

template <typename T>
static COMPV_ERROR_CODE addBordersVt(const CompVMatPtr input, CompVMatPtr output, const size_t strelHeight, const COMPV_BORDER_TYPE borderTypeTop, const COMPV_BORDER_TYPE borderTypeBottom);

template <typename T>
static COMPV_ERROR_CODE addBordersHz(const CompVMatPtr input, CompVMatPtr output, const size_t strelWidth, const COMPV_BORDER_TYPE borderType);

template <typename T, class CompVMathMorphOp>
static void CompVMathMorphProcess_C(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);

COMPV_ERROR_CODE CompVMathMorph::buildStructuringElement(CompVMatPtrPtr strel, const CompVSizeSz size, COMPV_MATH_MORPH_STREL_TYPE type COMPV_DEFAULT(COMPV_MATH_MORPH_STREL_TYPE_RECT))
{
	COMPV_CHECK_CODE_RETURN(buildStructuringElementGeneric<CompVMathMorphT>(strel, size, type));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMorph::process(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_MATH_MORPH_OP_TYPE opType, COMPV_BORDER_TYPE borderType COMPV_DEFAULT(COMPV_BORDER_TYPE_REPLICATE))
{
	switch (opType) {
	case COMPV_MATH_MORPH_OP_TYPE_ERODE:
		COMPV_CHECK_CODE_RETURN((basicOper<CompVMathMorphT, CompVMathMorphOpErode8u>(input, strel, output, borderType, borderType, borderType)));
		break;
	case COMPV_MATH_MORPH_OP_TYPE_DILATE:
		COMPV_CHECK_CODE_RETURN((basicOper<CompVMathMorphT, CompVMathMorphOpDilate8u>(input, strel, output, borderType, borderType, borderType)));
		break;
	case COMPV_MATH_MORPH_OP_TYPE_OPEN:
		// Erode then dilate
		COMPV_CHECK_CODE_RETURN((openCloseOper<CompVMathMorphT, CompVMathMorphOpErode8u, CompVMathMorphOpDilate8u>(input, strel, output, borderType, borderType, borderType)));
		break;
	case COMPV_MATH_MORPH_OP_TYPE_CLOSE:
		// Dilate then erode
		COMPV_CHECK_CODE_RETURN((openCloseOper<CompVMathMorphT, CompVMathMorphOpDilate8u, CompVMathMorphOpErode8u>(input, strel, output, borderType, borderType, borderType)));
		break;
#if 0 // Gradient function is correct but disabled because "CompVMathBasicOps::subs" not optimized (one of the input params not aligned)
	case COMPV_MATH_MORPH_OP_TYPE_GRADIENT:
		// Dilate - Erode
		COMPV_CHECK_CODE_RETURN((gradientOper<CompVMathMorphT>(input, strel, output, borderType, borderType, borderType)));
		break;
#endif
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Input morph op type not implemented yet");
		break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <typename T, class CompVMathMorphOp>
static COMPV_ERROR_CODE basicOper(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_BORDER_TYPE borderTypeRightLeft, COMPV_BORDER_TYPE borderTypeTop, COMPV_BORDER_TYPE borderTypeBottom)
{
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->elmtInBytes() != sizeof(T) || input->planeCount() != 1 ||
		!strel || strel->isEmpty() || strel->elmtInBytes() != sizeof(T) || strel->planeCount() != 1 ||
		input->cols() < (strel->cols() >> 1) || input->rows() < (strel->rows() >> 1) ||
		!output,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);

	const size_t input_width = input->cols();
	const size_t input_height = input->rows();
	const size_t input_stride = input->stride();

	// output can't be equal to input
	// create new output only if doesn't match the required format
	CompVMatPtr output_ = (input == *output) ? nullptr : *output;
	if (!output_ || output_->planeCount() != 1 || output_->elmtInBytes() != sizeof(T) || output_->cols() != input_width || output_->rows() != input_height || output_->stride() != input_stride) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&output_, input_height, input_width, input_stride));
	}

	/* Local variables */
	const size_t strel_width = strel->cols();
	const size_t strel_height = strel->rows();
	const size_t strel_width_div2 = strel_width >> 1;
	const size_t strel_height_div2 = strel_height >> 1;
	T* outPtr = output_->ptr<T>(strel_height_div2, strel_width_div2);
	const compv_uscalar_t op_height = static_cast<compv_uscalar_t>(input_height - (strel_height_div2 << 1));
	const compv_uscalar_t op_width = static_cast<compv_uscalar_t>(input_width - (strel_width_div2 << 1));
	const bool isErode = std::is_same<CompVMathMorphOpMin<T>, CompVMathMorphOp>::value;

	/* Hook to processing function */
	typedef void(*CompVMathMorphProcessFunPtr)(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);
	CompVMathMorphProcessFunPtr CompVMathMorphProcess = [](const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride) {
		CompVMathMorphProcess_C<T, CompVMathMorphOp >(strelInputPtrsPtr, strelInputPtrsCount, outPtr, width, height, stride);
	};

	if (std::is_same<T, uint8_t>::value) {
		void(*CompVMathMorphProcess_8u)(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Intrin_SSE2 : CompVMathMorphProcessDilate_8u_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Asm_X64_SSE2 : CompVMathMorphProcessDilate_8u_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Intrin_AVX2 : CompVMathMorphProcessDilate_8u_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Asm_X64_AVX2 : CompVMathMorphProcessDilate_8u_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Intrin_NEON : CompVMathMorphProcessDilate_8u_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Asm_NEON32 : CompVMathMorphProcessDilate_8u_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathMorphProcess_8u = isErode ? CompVMathMorphProcessErode_8u_Asm_NEON64 : CompVMathMorphProcessDilate_8u_Asm_NEON64);
		}
#endif
		if (CompVMathMorphProcess_8u) {
			CompVMathMorphProcess = reinterpret_cast<CompVMathMorphProcessFunPtr>(CompVMathMorphProcess_8u);
		}
	}

	/* Collect strel input pointers */
	CompVMatPtr strelInputPtrs;
	COMPV_CHECK_CODE_RETURN((buildStructuringElementInputPtrs<T>(strel, input, &strelInputPtrs)));
	const compv_uscalar_t* strelInputPtrsPtr = strelInputPtrs->ptr<const compv_uscalar_t>();
	const compv_uscalar_t strelInputPtrsCount = strelInputPtrs->cols();
	
	/* Function ptr to the MT process */
	auto mtFuncPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		/* Process */
		CompVMathMorphT* mt_outPtr = outPtr + (ystart * input_stride); // outPtr already has offset -> do not use output_->ptr<CompVMathMorphT>(ystart);
		const compv_uscalar_t mt_op_height = static_cast<compv_uscalar_t>(yend - ystart);
		compv_uscalar_t* mt_strelInputPtrsPtr = const_cast<compv_uscalar_t*>(strelInputPtrsPtr);
		CompVMatPtr mt_strelInputPtrs;
		if (ystart) {
			const compv_uscalar_t offset = static_cast<compv_uscalar_t>(ystart * input_stride);
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_uscalar_t>(&mt_strelInputPtrs, 1, strelInputPtrsCount));
			mt_strelInputPtrsPtr = mt_strelInputPtrs->ptr<compv_uscalar_t>();
			for (compv_uscalar_t i = 0; i < strelInputPtrsCount; ++i) {
				mt_strelInputPtrsPtr[i] = strelInputPtrsPtr[i] + offset;
			}
		}
		CompVMathMorphProcess(
			mt_strelInputPtrsPtr, strelInputPtrsCount,
			mt_outPtr, op_width, mt_op_height, static_cast<compv_uscalar_t>(input_stride)
		);

		/* Add Borders (must be after processing and vt then Hz) using complete input/output (no kernel offset) */
		const CompVRectFloat32 roi = { 
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>(yend - 1 + (strel_height_div2 << 1)) // bottom
		};
		CompVMatPtr mt_input, mt_output;
		COMPV_CHECK_CODE_RETURN(input->bind(&mt_input, roi));
		COMPV_CHECK_CODE_RETURN(output_->bind(&mt_output, roi));
		const bool first = (ystart == 0);
		const bool last = (yend == op_height);
		COMPV_CHECK_CODE_RETURN(addBordersVt<T>(mt_input, mt_output, strel_height, 
			first ? borderTypeTop : COMPV_BORDER_TYPE_IGNORE, 
			last ? borderTypeBottom : COMPV_BORDER_TYPE_IGNORE
		));
		COMPV_CHECK_CODE_RETURN(addBordersHz<T>(mt_input, mt_output, strel_width, 
			borderTypeRightLeft
		));
		return COMPV_ERROR_CODE_S_OK;
	};

	/* Dispatch tasks */
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(mtFuncPtr,
		op_width, op_height,
		COMPV_MATH_MAX((strel_height * op_width),
			COMPV_MATH_MORPH_BASIC_OPER_SAMPLES_PER_THREAD
		) // num rows per threads must be >= kernel/strel height
	));
	
	/* Save result */
	*output = *output_;

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T, class CompVMathMorphOp1, class CompVMathMorphOp2>
static COMPV_ERROR_CODE openCloseOper(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_BORDER_TYPE borderTypeRightLeft, COMPV_BORDER_TYPE borderTypeTop, COMPV_BORDER_TYPE borderTypeBottom)
{
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->elmtInBytes() != sizeof(T) || input->planeCount() != 1 ||
		!strel || strel->isEmpty() || strel->elmtInBytes() != sizeof(T) || strel->planeCount() != 1 ||
		input->cols() < (strel->cols() >> 1) || input->rows() < (strel->rows() >> 1) ||
		!output,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);

	/* Local variables */
	const size_t input_width = input->cols();
	const size_t input_height = input->rows();
	const size_t input_stride = input->stride();

	const size_t strel_height = strel->rows();
	const size_t overlap = (strel_height >> 1);
	const size_t overlap2 = (overlap << 1);

	/* output can't be equal to input */
	// create new output only if doesn't match the required format
	CompVMatPtr output_ = (input == *output) ? nullptr : *output;
	if (!output_ || output_->planeCount() != 1 || output_->elmtInBytes() != sizeof(T) || output_->cols() != input_width || output_->rows() != input_height || output_->stride() != input_stride) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&output_, input_height, input_width, input_stride));
	}
		
	/* Function ptr to the MT process */
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const bool first = (ystart == 0);
		const bool last = (yend == input_height);
		const size_t overlap_top2 = first ? 0 : overlap2;
		const size_t overlap_top = first ? 0 : overlap;
		const size_t overlap_bottom2 = last ? 0 : overlap2;
		const size_t overlap_bottom = last ? 0 : overlap;
		
		// Bind to input and perform first oper
		const CompVRectFloat32 roi = { 
			0.f, // left
			static_cast<compv_float32_t>(ystart - overlap_top2), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>(yend + overlap_bottom2) // bottom
		};
		CompVMatPtr mt_inputBind, mt_temp;
		COMPV_CHECK_CODE_RETURN(input->bind(&mt_inputBind, roi));
		COMPV_CHECK_CODE_RETURN((basicOper<T, CompVMathMorphOp1>(mt_inputBind, strel, &mt_temp,
			borderTypeRightLeft,
			first ? borderTypeTop : COMPV_BORDER_TYPE_IGNORE,
			last ? borderTypeBottom : COMPV_BORDER_TYPE_IGNORE
		)));
		
		// Add offset to the output and temp and perform second oper
		CompVMatPtr mt_tempBind;
		const CompVRectFloat32 roi_offset_temp = {
			0.f, // left
			static_cast<compv_float32_t>(0 + overlap_top), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>(mt_temp->rows() - 1 - overlap_bottom) // bottom
		};
		COMPV_CHECK_CODE_RETURN(mt_temp->bind(&mt_tempBind, roi_offset_temp));

		CompVMatPtr mt_outputBind;
		const CompVRectFloat32 roi_offset_output = {
			0.f, // left
			static_cast<compv_float32_t>((ystart - overlap_top)), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>((ystart - overlap_top) + mt_tempBind->rows() - 1) // bottom
		};
		COMPV_CHECK_CODE_RETURN(output_->bind(&mt_outputBind, roi_offset_output));
		COMPV_CHECK_CODE_RETURN((basicOper<T, CompVMathMorphOp2>(mt_tempBind, strel, &mt_outputBind,
			borderTypeRightLeft, 
			first ? borderTypeTop : COMPV_BORDER_TYPE_IGNORE,
			last ? borderTypeBottom : COMPV_BORDER_TYPE_IGNORE
		)));
		return COMPV_ERROR_CODE_S_OK;
	};

	/* Dispatch tasks */
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(funcPtr, 
		input_width, input_height, 
		COMPV_MATH_MAX(((strel_height << 1) * input_width),
			COMPV_MATH_MORPH_OPEN_CLOSE_OPER_SAMPLES_PER_THREAD
		) // num rows per threads should be >= (kernel/strel height) * 2
	));

	/* Save result */
	*output = output_;

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE gradientOper(const CompVMatPtr& input, const CompVMatPtr& strel, CompVMatPtrPtr output, COMPV_BORDER_TYPE borderTypeRightLeft, COMPV_BORDER_TYPE borderTypeTop, COMPV_BORDER_TYPE borderTypeBottom)
{
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->elmtInBytes() != sizeof(T) || input->planeCount() != 1 ||
		!strel || strel->isEmpty() || strel->elmtInBytes() != sizeof(T) || strel->planeCount() != 1 ||
		input->cols() < (strel->cols() >> 1) || input->rows() < (strel->rows() >> 1) ||
		!output,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);

	/* Local variables */
	const size_t input_width = input->cols();
	const size_t input_height = input->rows();
	const size_t input_stride = input->stride();

	const size_t strel_height = strel->rows();
	const size_t overlap = (strel_height >> 1);
	const size_t overlap2 = (overlap << 1);

	/* output can't be equal to input */
	// create new output only if doesn't match the required format
	CompVMatPtr output_ = (input == *output) ? nullptr : *output;
	if (!output_ || output_->planeCount() != 1 || output_->elmtInBytes() != sizeof(T) || output_->cols() != input_width || output_->rows() != input_height || output_->stride() != input_stride) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&output_, input_height, input_width, input_stride));
	}

	/* Function ptr to the MT process */
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const bool first = (ystart == 0);
		const bool last = (yend == input_height);
		const size_t overlap_top2 = first ? 0 : overlap2;
		const size_t overlap_top = first ? 0 : overlap;
		const size_t overlap_bottom2 = last ? 0 : overlap2;
		const size_t overlap_bottom = last ? 0 : overlap;

		// Bind to input and perform first oper
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart - overlap_top2), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>(yend + overlap_bottom2) // bottom
		};
		CompVMatPtr mt_inputBind, mt_temp;
		COMPV_CHECK_CODE_RETURN(input->bind(&mt_inputBind, roi));
		COMPV_CHECK_CODE_RETURN((basicOper<T, CompVMathMorphOpMin<T> >(mt_inputBind, strel, &mt_temp,
			borderTypeRightLeft,
			first ? borderTypeTop : COMPV_BORDER_TYPE_IGNORE,
			last ? borderTypeBottom : COMPV_BORDER_TYPE_IGNORE
			)));

		// Add offset to the output and temp and perform second oper
		CompVMatPtr mt_tempBind;
		const CompVRectFloat32 roi_offset_temp = {
			0.f, // left
			static_cast<compv_float32_t>(0 + overlap_top), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>(mt_temp->rows() - 1 - overlap_bottom) // bottom
		};
		COMPV_CHECK_CODE_RETURN(mt_temp->bind(&mt_tempBind, roi_offset_temp));

		CompVMatPtr mt_outputBind;
		const CompVRectFloat32 roi_offset_output = {
			0.f, // left
			static_cast<compv_float32_t>((ystart - overlap_top)), // top
			static_cast<compv_float32_t>(input_width - 1), // right
			static_cast<compv_float32_t>((ystart - overlap_top) + mt_tempBind->rows() - 1) // bottom
		};
		COMPV_CHECK_CODE_RETURN(output_->bind(&mt_outputBind, roi_offset_output));
		COMPV_CHECK_CODE_RETURN(input->bind(&mt_inputBind, roi_offset_output));
		COMPV_CHECK_CODE_RETURN((basicOper<T, CompVMathMorphOpMax<T> >(mt_inputBind, strel, &mt_outputBind,
			borderTypeRightLeft,
			first ? borderTypeTop : COMPV_BORDER_TYPE_IGNORE,
			last ? borderTypeBottom : COMPV_BORDER_TYPE_IGNORE
			)));

		COMPV_CHECK_CODE_RETURN(CompVMathBasicOps::subs(mt_outputBind, 
			mt_tempBind, 
			&mt_outputBind
		));
		return COMPV_ERROR_CODE_S_OK;
	};

	/* Dispatch tasks */
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(funcPtr,
		input_width, input_height,
		COMPV_MATH_MAX(((strel_height << 1) * input_width),
			COMPV_MATH_MORPH_OPEN_CLOSE_OPER_SAMPLES_PER_THREAD
		) // num rows per threads should be >= (kernel/strel height) * 2
	));

	/* Save result */
	*output = output_;

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE buildStructuringElementInputPtrs(const CompVMatPtr& strel, const CompVMatPtr& input, CompVMatPtrPtr strelInputPtrs)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(!strelInputPtrs || input->elmtInBytes() != sizeof(T) || strel->elmtInBytes() != sizeof(T), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const compv_uscalar_t width = static_cast<compv_uscalar_t>(strel->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(strel->rows());
	const compv_uscalar_t stride = static_cast<compv_uscalar_t>(strel->stride());

	// Count number of non-zero elements
	compv_uscalar_t count = 0;
	const T* strelPtr = strel->ptr<const T>();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			if (strelPtr[i]) {
				++count;
			}
		}
		strelPtr += stride;
	}

	// Collect non-zero elements
	COMPV_CHECK_EXP_RETURN(!count, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Structured element is full of zeros");
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_uscalar_t>(strelInputPtrs, 1, count));
	strelPtr = strel->ptr<const T>();
	compv_uscalar_t* strelInputPtrsPtr = (*strelInputPtrs)->ptr<compv_uscalar_t>();
	for (compv_uscalar_t j = 0, index = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			if (strelPtr[i]) {
				strelInputPtrsPtr[index++] = reinterpret_cast<compv_uscalar_t>(input->ptr<const T>(j, i));
			}
		}
		strelPtr += stride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE buildStructuringElementGeneric(CompVMatPtrPtr strel, const CompVSizeSz size, COMPV_MATH_MORPH_STREL_TYPE type)
{
	COMPV_CHECK_EXP_RETURN(!strel || !size.width || !size.height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr strel_ = *strel;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&strel_, size.height, size.width)); // For now only 8u elements are supported
	T* strelPtr_ = strel_->ptr<T>();
	const size_t stride = strel_->stride();
	const size_t width = strel_->cols();
	const size_t height = strel_->rows();
	const T maxx = std::numeric_limits<T>::max();
	switch (type) {
	case COMPV_MATH_MORPH_STREL_TYPE_RECT: {
		for (size_t j = 0; j < height; ++j) {
			for (size_t i = 0; i < width; ++i) {
				strelPtr_[i] = maxx;
			}
			strelPtr_ += stride;
		}
		break;
	}
	case COMPV_MATH_MORPH_STREL_TYPE_CROSS: {
		COMPV_CHECK_CODE_RETURN(strel_->zero_rows());
		strelPtr_ = strel_->ptr<T>(height >> 1);
		for (size_t i = 0; i < width; ++i) {
			strelPtr_[i] = maxx;
		}
		strelPtr_ = strel_->ptr<T>(0, width >> 1);
		for (size_t j = 0; j < height; ++j) {
			strelPtr_[0] = maxx;
			strelPtr_ += stride;
		}
		break;
	}
	case COMPV_MATH_MORPH_STREL_TYPE_DIAMOND: {
		COMPV_CHECK_CODE_RETURN(strel_->zero_rows());
		size_t i, j, count;
		const size_t height_div2 = height >> 1;
		const size_t width_div2 = width >> 1;
		const size_t stride_plus1 = stride + 1;
		const size_t stride_minus1 = stride - 1;
		// Top
		strelPtr_ = strel_->ptr<T>(0, width_div2);
		for (j = 0, count = 1; j < height_div2; ++j, count += 2) {
			for (i = 0; i < count; ++i) {
				strelPtr_[i] = maxx;
			}
			strelPtr_ += stride_minus1;
		}
		// Bottom
		for (j = 0; j <= height_div2; ++j, count -= 2) {
			for (i = 0; i < count; ++i) {
				strelPtr_[i] = maxx;
			}
			strelPtr_ += stride_plus1;
		}
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Invalid structuring element type");
		break;
	}

	*strel = strel_;
	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE addBordersVt(const CompVMatPtr input, CompVMatPtr output, const size_t strelHeight, const COMPV_BORDER_TYPE borderTypeTop, const COMPV_BORDER_TYPE borderTypeBottom)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->planeCount() != 1 || input->elmtInBytes() != sizeof(T) ||
		!output || output->planeCount() != 1 || output->elmtInBytes() != sizeof(T) ||
		output->cols() != input->cols() || output->rows() != input->rows() || output->stride() != input->stride(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t strelHeightDiv2 = (strelHeight >> 1);
	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();

	// Segmentation fault when the input image has an offset (e.g. bound to non-zero cols):
	//		-> for the last row reset 'width' only instead of 'stride'.
	const size_t bSizeInSamples = (((strelHeightDiv2 - 1) * stride) + width);
	T* outPtr = output->ptr<T>();
	const T* inPtr = input->ptr<const T>();

	// Top
	if (borderTypeTop == COMPV_BORDER_TYPE_ZERO) {
		CompVMem::zero(outPtr, bSizeInSamples * sizeof(T));
	}
	else if (borderTypeTop == COMPV_BORDER_TYPE_REPLICATE) {
		memcpy(outPtr, inPtr, bSizeInSamples * sizeof(T));
	}
	else if (borderTypeTop != COMPV_BORDER_TYPE_IGNORE) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}

	// Bottom
	if (borderTypeBottom == COMPV_BORDER_TYPE_ZERO) {
		const size_t offsetInSamples = ((height - strelHeightDiv2) * stride);
		CompVMem::zero(outPtr + offsetInSamples, bSizeInSamples * sizeof(T));
	}
	else if (borderTypeBottom == COMPV_BORDER_TYPE_REPLICATE) {
		const size_t offsetInSamples = ((height - strelHeightDiv2) * stride);
		memcpy(&outPtr[offsetInSamples], &inPtr[offsetInSamples], bSizeInSamples * sizeof(T));
	}
	else if (borderTypeBottom != COMPV_BORDER_TYPE_IGNORE) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE addBordersHz(const CompVMatPtr input, CompVMatPtr output, const size_t strelWidth, const COMPV_BORDER_TYPE borderType)
{
	// Internal function -> minimum test for input parameters
	COMPV_CHECK_EXP_RETURN(
		!input || input->isEmpty() || input->planeCount() != 1 || input->elmtInBytes() != sizeof(T) ||
		!output || output->planeCount() != 1 || output->elmtInBytes() != sizeof(T) ||
		output->cols() != input->cols() || output->rows() != input->rows() || output->stride() != input->stride(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t strelWidthDiv2 = (strelWidth >> 1);
	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	T* outPtr = output->ptr<T>();
	const T* inPtr = input->ptr<const T>();

	// Set hz borders to zero
	// We must not accept garbage in the border (could be used by the calling function -e.g to find the max value for normalization)
	if (borderType == COMPV_BORDER_TYPE_ZERO) {
		T *outPtr0 = outPtr, *outPtr1 = outPtr + (width - strelWidthDiv2);
		switch (strelWidthDiv2) { // 1 and 2 (kernel sizes 3 and 5 are very common)
		case 1: {
			const size_t kmax = (stride * height);
			for (size_t k = 0; k < kmax; k += stride) {
				outPtr0[k] = 0, outPtr1[k] = 0;
			}
			break;
		}
		case 2: {
			const size_t kmax = (stride * height);
			for (size_t k = 0; k < kmax; k += stride) {
				outPtr0[k] = 0, outPtr0[k + 1] = 0, outPtr1[k] = 0, outPtr1[k + 1] = 0;
			}
			break;
		}
		default: {
			for (size_t row = 0; row < height; ++row) {
				for (size_t col = 0; col < strelWidthDiv2; ++col) {
					outPtr0[col] = 0, outPtr1[col] = 0;
				}
				outPtr0 += stride;
				outPtr1 += stride;
			}
			break;
		}
		}
	}
	else if (borderType == COMPV_BORDER_TYPE_REPLICATE) {
		const T *inPtr0 = inPtr, *inPtr1 = inPtr + (width - strelWidthDiv2);
		T *outPtr0 = outPtr, *outPtr1 = outPtr + (width - strelWidthDiv2);
		switch (strelWidthDiv2) { // 1 and 2 (kernel sizes 3 and 5 are very common)
		case 1: {
			const size_t kmax = (stride * height);
			for (size_t k = 0; k < kmax; k += stride) {
				outPtr0[k] = inPtr0[k], outPtr1[k] = inPtr1[k];
			}
			break;
		}
		case 2: {
			const size_t kmax = (stride * height);
			for (size_t k = 0; k < kmax; k += stride) {
				outPtr0[k] = inPtr0[k], outPtr0[k + 1] = inPtr0[k + 1], outPtr1[k] = inPtr1[k], outPtr1[k + 1] = inPtr1[k + 1];
			}
			break;
		}
		default: {
			for (size_t row = 0; row < height; ++row) {
				for (size_t col = 0; col < strelWidthDiv2; ++col) {
					outPtr0[col] = inPtr0[col], outPtr1[col] = inPtr1[col];
				}
				outPtr0 += stride;
				outPtr1 += stride;
				inPtr0 += stride;
				inPtr1 += stride;
			}
			break;
		}
		}
	}
	else if (borderType != COMPV_BORDER_TYPE_IGNORE) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T, class CompVMathMorphOp>
static void CompVMathMorphProcess_C(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, T* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	CompVMathMorphOp Op;
	const compv_uscalar_t strelInputPtrsPad = (stride - width);
	for (compv_uscalar_t j = 0, k = 0; j < height; ++j, k += strelInputPtrsPad) {
		for (compv_uscalar_t i = 0; i < width; ++i, ++k) {
			T rr = *reinterpret_cast<const T*>(k + strelInputPtrsPtr[0]);
			for (compv_uscalar_t v = 1; v < strelInputPtrsCount; ++v) {
				rr = Op(rr, *reinterpret_cast<const T*>(k + strelInputPtrsPtr[v]));
			}
			outPtr[i] = rr;
		}
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()
