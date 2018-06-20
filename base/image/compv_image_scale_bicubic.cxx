/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_scale_bicubic.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse2.h"
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse41.h"
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_avx2.h"


// Some documentation:
//	- https://en.wikipedia.org/wiki/Bicubic_interpolation
//	- https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/

#define COMPV_THIS_CLASSNAME	"CompVImageScaleBicubic"

#define COMPV_IMAGE_SCALE_BICUBIC_SAMPLES_PER_THREAD (32 * 32) // CPU-unfriendly

COMPV_NAMESPACE_BEGIN()

#define HERMITE_32F_C(A, B, C, D, t, t2, t3, ret) { \
	const compv_float32_t a = /*(A*(-0.5f))	+ (B*(1.5f))	+ (C*(-1.5f))	+ (D*(0.5f))*/ ((D - A) * 0.5f) + ((B - C) * 1.5f); \
	const compv_float32_t b = A				+ (B*(-2.5f))	+ (C*(2.0f))	+ (D*(-0.5f)); \
	const compv_float32_t c = /*(A*(-0.5f))					+ (C * 0.5f)*/ ((C - A) * 0.5f); \
	const compv_float32_t d =				B; \
	/* simulate vpadd_f32 (to have same MD5) */ \
	const compv_float32_t s0 = (a*t3) + (c*t); \
	const compv_float32_t s1 = (b*t2) + (d); \
	ret = s0 + s1; \
}

static void CompVImageScaleBicubicHermite_32f32s_C(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint1,
	const compv_float32_t* xfract1,
	const int32_t* yint1,
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidthMinus1,
	const compv_uscalar_t inHeightMinus1,
	const compv_uscalar_t inStride
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	const compv_scalar_t inWidthMinus1_ = static_cast<compv_scalar_t>(inWidthMinus1);
	const compv_scalar_t inHeightMinus1_ = static_cast<compv_scalar_t>(inHeightMinus1);
	compv_scalar_t x0 = (*xint1 - 1), x1 = (x0 + 1), x2 = (x0 + 2), x3 = (x0 + 3);
	compv_scalar_t y0 = (*yint1 - 1), y1 = (y0 + 1), y2 = (y0 + 2), y3 = (y0 + 3);
	x0 = COMPV_MATH_CLIP3(0, inWidthMinus1_, x0); // SIMD: max(0, min(x0, inWidthMinus1))
	x1 = COMPV_MATH_CLIP3(0, inWidthMinus1_, x1);
	x2 = COMPV_MATH_CLIP3(0, inWidthMinus1_, x2);
	x3 = COMPV_MATH_CLIP3(0, inWidthMinus1_, x3);
	y0 = COMPV_MATH_CLIP3(0, inHeightMinus1_, y0);
	y1 = COMPV_MATH_CLIP3(0, inHeightMinus1_, y1);
	y2 = COMPV_MATH_CLIP3(0, inHeightMinus1_, y2);
	y3 = COMPV_MATH_CLIP3(0, inHeightMinus1_, y3);

	const compv_float32_t* p0 = &inPtr[y0 * inStride];
	const compv_float32_t* p1 = &inPtr[y1 * inStride];
	const compv_float32_t* p2 = &inPtr[y2 * inStride];
	const compv_float32_t* p3 = &inPtr[y3 * inStride];

	const compv_float32_t& xfract = *xfract1;
	const compv_float32_t& yfract = *yfract1;
	const compv_float32_t xfract2 = (xfract * xfract);
	const compv_float32_t xfract3 = (xfract2 * xfract);
	const compv_float32_t yfract2 = (yfract * yfract);
	const compv_float32_t yfract3 = (yfract2 * yfract);

	compv_float32_t c0, c1, c2, c3;
	HERMITE_32F_C(p0[x0], p0[x1], p0[x2], p0[x3], xfract, xfract2, xfract3, c0);
	HERMITE_32F_C(p1[x0], p1[x1], p1[x2], p1[x3], xfract, xfract2, xfract3, c1);
	HERMITE_32F_C(p2[x0], p2[x1], p2[x2], p2[x3], xfract, xfract2, xfract3, c2);
	HERMITE_32F_C(p3[x0], p3[x1], p3[x2], p3[x3], xfract, xfract2, xfract3, c3);

	HERMITE_32F_C(c0, c1, c2, c3, yfract, yfract2, yfract3, *outPtr);
}

static void CompVImageScaleBicubicPostProcessRow_32f32s_C(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint4, 
	const compv_float32_t* xfract4,
	const int32_t* yint4, 
	const compv_float32_t* yfract4,
	const compv_uscalar_t rowCount
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	const compv_float32_t* p0 = &inPtr[yint4[0]];
	const compv_float32_t* p1 = &inPtr[yint4[1]];
	const compv_float32_t* p2 = &inPtr[yint4[2]];
	const compv_float32_t* p3 = &inPtr[yint4[3]];

	const compv_float32_t& yfract3 = yfract4[0];
	const compv_float32_t& yfract2 = yfract4[1];
	const compv_float32_t& yfract1 = yfract4[2];

	compv_float32_t c0, c1, c2, c3;

	for (compv_uscalar_t i = 0; i < rowCount; ++i, xint4 += 4, xfract4 += 4) {
		const int32_t& x0 = xint4[0];
		const int32_t& x1 = xint4[1];
		const int32_t& x2 = xint4[2];
		const int32_t& x3 = xint4[3];

		const compv_float32_t& xfract3 = xfract4[0];
		const compv_float32_t& xfract2 = xfract4[1];
		const compv_float32_t& xfract1 = xfract4[2];

		HERMITE_32F_C(p0[x0], p0[x1], p0[x2], p0[x3], xfract1, xfract2, xfract3, c0);
		HERMITE_32F_C(p1[x0], p1[x1], p1[x2], p1[x3], xfract1, xfract2, xfract3, c1);
		HERMITE_32F_C(p2[x0], p2[x1], p2[x2], p2[x3], xfract1, xfract2, xfract3, c2);
		HERMITE_32F_C(p3[x0], p3[x1], p3[x2], p3[x3], xfract1, xfract2, xfract3, c3);

		HERMITE_32F_C(c0, c1, c2, c3, yfract1, yfract2, yfract3, outPtr[i]);
	}
}

static void CompVImageScaleBicubicPreprocess_32s32f_C(int32_t* intergral, compv_float32_t* fraction, const compv_float32_t* sv1, const compv_uscalar_t outSize, const compv_scalar_t intergralMax, const compv_scalar_t intergralStride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPGPU implementation could be found");
	const compv_float32_t& sv = *sv1;
	const int32_t intergralMax_ = static_cast<int32_t>(intergralMax);
	const int32_t intergralStride_ = static_cast<int32_t>(intergralStride);
	for (compv_uscalar_t i = 0, j = 0; i < outSize; i += 1, j += 4) {
		const compv_float32_t fract = static_cast<compv_float32_t>((i + 0.5f) * sv - 0.5f);
		const int32_t intergral_ = static_cast<int32_t>(std::floor(fract));
		const compv_float32_t fraction_ = static_cast<compv_float32_t>(fract - intergral_);
		const compv_float32_t fraction2_ = (fraction_ * fraction_);
		// TODO(dmi): SIMD: 
		//	static const __m128i vecOffset = _mm_setr_epi32(-1, 0, 1, 2); // outside the loop
		//  static const __m128i vecZero = _mm_setzero_si128(); // outside the loop
		//	const __m128i vecIntergralMax = _mm_set1_epi32(intergralMax); // outside the loop
		//	__m128i vecIntergral = _mm_add_epi32(_mm_set1_epi32(intergral_), vecOffset);
		// vecIntergral = _mm_max_epi32(vecZero, _mm_min_epi32(vecIntergral, vecIntergralMax));
		intergral[j + 0] = std::max(0, std::min(intergral_ - 1, intergralMax_)) * intergralStride_;
		intergral[j + 1] = std::max(0, std::min(intergral_ - 0, intergralMax_)) * intergralStride_;
		intergral[j + 2] = std::max(0, std::min(intergral_ + 1, intergralMax_)) * intergralStride_;
		intergral[j + 3] = std::max(0, std::min(intergral_ + 2, intergralMax_)) * intergralStride_;

		fraction[j + 0] = (fraction2_ * fraction_); // x^3
		fraction[j + 1] = (fraction2_); // x^2
		fraction[j + 2] = (fraction_); // x^1
		fraction[j + 3] = (1.f); // x^0
	}
}

COMPV_ERROR_CODE CompVImageScaleBicubicProcessor::init()
{
	NOT_OPTIMIZ_hermite_32f32s = CompVImageScaleBicubicHermite_32f32s_C;
	preprocess_32s32f = CompVImageScaleBicubicPreprocess_32s32f_C;
	postprocessrow_32f32s = CompVImageScaleBicubicPostProcessRow_32f32s_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(postprocessrow_32f32s = CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(NOT_OPTIMIZ_hermite_32f32s = CompVImageScaleBicubicHermite_32f32s_Intrin_SSE41);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(postprocessrow_32f32s = CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_AVX2);
		COMPV_EXEC_IFDEF_INTRIN_X86(NOT_OPTIMIZ_hermite_32f32s = CompVImageScaleBicubicHermite_32f32s_Intrin_AVX2);
	}

#elif COMPV_ARCH_ARM
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScaleBicubic::process(const CompVMatPtr& imageIn, CompVMatPtr& imageOut)
{
	// Internal function, no need to check for input parameters
	// For now only grascale images are fully tested
	COMPV_CHECK_EXP_RETURN(
		imageIn->elmtInBytes() != sizeof(uint8_t) || imageIn->planeCount() != 1,
		COMPV_ERROR_CODE_E_NOT_IMPLEMENTED,
		"Only 8up1 subtypes are supported using bilinear scaling"
	);

	CompVImageScaleBicubicProcessor processor;
	COMPV_CHECK_CODE_RETURN(processor.init());

	const compv_uscalar_t outStride = imageOut->stride();
	const compv_uscalar_t outHeight = static_cast<compv_uscalar_t>(imageOut->rows());
	const compv_uscalar_t outWidth = static_cast<compv_uscalar_t>(imageOut->cols());

	// TODO(dmi): add faster implementation
	if ((outHeight * outWidth) > (32 * 32)) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("OpenCV implementation [fixed-point] is faster [but less accurate]");
	}

	const compv_scalar_t inStride = static_cast<compv_scalar_t>(imageIn->stride());
	const compv_scalar_t inHeight = static_cast<compv_scalar_t>(imageIn->rows());
	const compv_scalar_t inWidth = static_cast<compv_scalar_t>(imageIn->cols());
	const compv_scalar_t inWidthMinus1 = inWidth - 1;
	const compv_scalar_t inHeightMinus1 = inHeight - 1;

	// Compute "yintMat", "yfractMat", "xintMat" and "xfractMat"
	CompVMatPtr yintMat, yfractMat, xintMat, xfractMat;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&yintMat, 1, outHeight << 2));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&yfractMat, 1, outHeight << 2));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&xintMat, 1, outWidth << 2));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&xfractMat, 1, outWidth << 2));

	const compv_float32_t ySV = static_cast<compv_float32_t>(inHeight) / static_cast<compv_float32_t>(outHeight - 1);
	const compv_float32_t xSV = static_cast<compv_float32_t>(inWidth) / static_cast<compv_float32_t>(outWidth - 1);
	processor.preprocess_32s32f(yintMat->ptr<int32_t>(), yfractMat->ptr<compv_float32_t>(), &ySV, outHeight, inHeightMinus1, inStride);
	processor.preprocess_32s32f(xintMat->ptr<int32_t>(), xfractMat->ptr<compv_float32_t>(), &xSV, outWidth, inWidthMinus1, 1);

	CompVMatPtr imageIn32f;
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(imageIn, &imageIn32f)));
	const compv_float32_t* inPtr = imageIn32f->ptr<const compv_float32_t>();

	CompVMatPtr output32f;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&output32f, outHeight, outWidth, outStride));
	
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_float32_t* outPtr = output32f->ptr<compv_float32_t>(ystart);
		const int32_t* yintPtr = yintMat->ptr<const int32_t>();
		const compv_float32_t* yfractPtr = yfractMat->ptr<const compv_float32_t>();
		const int32_t* xintPtr = xintMat->ptr<const int32_t>();
		const compv_float32_t* xfractPtr = xfractMat->ptr<const compv_float32_t>();
		for (size_t y = ystart; y < yend; ++y) {
			processor.postprocessrow_32f32s(
				outPtr,
				inPtr,
				xintPtr,
				xfractPtr,
				&yintPtr[y << 2],
				&yfractPtr[y << 2],
				outWidth
			);
			outPtr += outStride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		outWidth,
		outHeight,
		COMPV_IMAGE_SCALE_BICUBIC_SAMPLES_PER_THREAD
	));

	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static_pixel8(output32f, &imageOut)));
	
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
