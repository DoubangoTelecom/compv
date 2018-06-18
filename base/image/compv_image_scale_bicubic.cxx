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

#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse41.h"


// Some documentation:
//	- https://en.wikipedia.org/wiki/Bicubic_interpolation
//	- https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/

#define COMPV_THIS_CLASSNAME	"CompVImageScaleBicubic"

#define COMPV_IMAGE_SCALE_BICUBIC_SAMPLES_PER_THREAD (32 * 32) // CPU-unfriendly

COMPV_NAMESPACE_BEGIN()

// Hermite
static compv_float32_t __hermite_32f(const compv_float32_t A, const compv_float32_t B, const compv_float32_t C, const compv_float32_t D, const compv_float32_t t, const compv_float32_t t2, compv_float32_t t3)
{
	const compv_float32_t a = (A*(-0.5f))	+ (B*(1.5f))	+ (C*(-1.5f))	+ (D*(0.5f));
	const compv_float32_t b = A				+ (B*(-2.5f))	+ (C*(2.0f))	+ (D*(-0.5f));
	const compv_float32_t c = (A*(-0.5f))					+ (C * 0.5f);
	const compv_float32_t d =				B;
	// simulate vpadd_f32 (to have same MD5)
	const compv_float32_t s0 = (a*t3) + (c*t);
	const compv_float32_t s1 = (b*t2) + (d);
	return s0 + s1;
}

static void CompVImageScaleBicubicHermite_32f32s_C(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint1, 
	const compv_float32_t* xfract1,
	const int32_t* yint1, 
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidth,
	const compv_uscalar_t inHeight,
	const compv_uscalar_t inStride
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	const compv_scalar_t inWidthMinus1 = static_cast<compv_scalar_t>(inWidth - 1);
	const compv_scalar_t inHeightMinus1 = static_cast<compv_scalar_t>(inHeight - 1);
	compv_scalar_t x0 = (*xint1 - 1), x1 = (x0 + 1), x2 = (x0 + 2), x3 = (x0 + 3);
	compv_scalar_t y0 = (*yint1 - 1), y1 = (y0 + 1), y2 = (y0 + 2), y3 = (y0 + 3);
	x0 = COMPV_MATH_CLIP3(0, inWidthMinus1, x0); // SIMD: max(0, min(x0, inWidthMinus1))
	x1 = COMPV_MATH_CLIP3(0, inWidthMinus1, x1);
	x2 = COMPV_MATH_CLIP3(0, inWidthMinus1, x2);
	x3 = COMPV_MATH_CLIP3(0, inWidthMinus1, x3);
	y0 = COMPV_MATH_CLIP3(0, inHeightMinus1, y0);
	y1 = COMPV_MATH_CLIP3(0, inHeightMinus1, y1);
	y2 = COMPV_MATH_CLIP3(0, inHeightMinus1, y2);
	y3 = COMPV_MATH_CLIP3(0, inHeightMinus1, y3);

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
	
	const compv_float32_t c0 = __hermite_32f(p0[x0], p0[x1], p0[x2], p0[x3], xfract, xfract2, xfract3); // TODO(dmi): AVX - use gather
	const compv_float32_t c1 = __hermite_32f(p1[x0], p1[x1], p1[x2], p1[x3], xfract, xfract2, xfract3);
	const compv_float32_t c2 = __hermite_32f(p2[x0], p2[x1], p2[x2], p2[x3], xfract, xfract2, xfract3);
	const compv_float32_t c3 = __hermite_32f(p3[x0], p3[x1], p3[x2], p3[x3], xfract, xfract2, xfract3);
	*outPtr = __hermite_32f(c0, c1, c2, c3, yfract, yfract2, yfract3);

	//*outPtr = static_cast<uint8_t>(COMPV_MATH_CLIP3(0, 255.f, value)); // SIMD(dmi): saturation (no need to clip)
}

static void CompVImageScaleBicubicIndices_32s32f_C(int32_t* intergral, compv_float32_t* fraction, const compv_float32_t* sv1, const compv_uscalar_t outSize)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPGPU implementation could be found");
	const compv_float32_t& sv = *sv1;
	compv_float32_t v = -0.5f;
	for (compv_uscalar_t i = 0; i < outSize; ++i) {
		intergral[i] = static_cast<int32_t>(v);
		fraction[i] = static_cast<compv_float32_t>(v - std::floor(v));
		v += sv;
	}
}

COMPV_ERROR_CODE CompVImageScaleBicubicProcessor::init()
{
	bicubic_32f32s = CompVImageScaleBicubicHermite_32f32s_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(bicubic_32f32s = CompVImageScaleBicubicHermite_32f32s_Intrin_SSE41);
		//COMPV_EXEC_IFDEF_ASM_X64(bicubic_8u32f = CompVImageScaleBicubicHermite_8u32f_Asm_X64_SSE41);
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

	const compv_uscalar_t inStride = imageIn->stride();
	const compv_uscalar_t inHeight = static_cast<compv_uscalar_t>(imageIn->rows());
	const compv_uscalar_t inWidth = static_cast<compv_uscalar_t>(imageIn->cols());

	// Compute "yintMat", "yfractMat", "xintMat" and "xfractMat"
	CompVMatPtr yintMat, yfractMat, xintMat, xfractMat;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&yintMat, 1, outHeight));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&yfractMat, 1, outHeight));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&xintMat, 1, outWidth));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&xfractMat, 1, outWidth));

	void(*CompVImageScaleBicubicIndices_32s32f)(int32_t* intergral, compv_float32_t* fraction, const compv_float32_t* sv1, const compv_uscalar_t outSize)
		= CompVImageScaleBicubicIndices_32s32f_C;
#if COMPV_ARCH_X86
#elif COMPV_ARCH_ARM
#endif

	const compv_float32_t ySV = static_cast<compv_float32_t>(inHeight) / static_cast<compv_float32_t>(outHeight - 1);
	const compv_float32_t xSV = static_cast<compv_float32_t>(inWidth) / static_cast<compv_float32_t>(outWidth - 1);
	CompVImageScaleBicubicIndices_32s32f(yintMat->ptr<int32_t>(), yfractMat->ptr<compv_float32_t>(), &ySV, outHeight);
	CompVImageScaleBicubicIndices_32s32f(xintMat->ptr<int32_t>(), xfractMat->ptr<compv_float32_t>(), &xSV, outWidth);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Convert from 8u to 32f not MT");
	CompVMatPtr imageIn32f;
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(imageIn, &imageIn32f)));
	const compv_float32_t* inPtr = imageIn32f->ptr<const compv_float32_t>();

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD for COMPV_MATH_ROUNDFU_2_NEAREST_INT at the end");
	
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const int32_t yendInt32 = static_cast<int32_t>(yend);
		uint8_t* outPtr = imageOut->ptr<uint8_t>(ystart);
		const int32_t* yintPtr = yintMat->ptr<const int32_t>();
		const compv_float32_t* yfractPtr = yfractMat->ptr<const compv_float32_t>();
		const int32_t* xintPtr = xintMat->ptr<const int32_t>();
		const compv_float32_t* xfractPtr = xfractMat->ptr<const compv_float32_t>();
		compv_float32_t out;
		for (int32_t y = static_cast<int32_t>(ystart); y < yendInt32; ++y) {
			for (int x = 0; x < outWidth; ++x) {
				processor.bicubic_32f32s(
					&out,
					inPtr,
					&xintPtr[x], 
					&xfractPtr[x], 
					&yintPtr[y], 
					&yfractPtr[y],
					inWidth,
					inHeight,
					inStride
				);
				outPtr[x] = COMPV_MATH_ROUNDFU_2_NEAREST_INT(COMPV_MATH_CLIP3(0, 255.f, out), uint8_t);
			}
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
	
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
