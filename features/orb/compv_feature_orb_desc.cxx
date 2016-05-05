/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
/* @description
* This class implements ORB (Oriented FAST and Rotated BRIEF) feature descriptor.
* Some literature:
* ORB final: https://www.willowgarage.com/sites/default/files/orb_final.pdf
* BRIEF descriptor: https://www.robots.ox.ac.uk/~vgg/rg/papers/CalonderLSF10.pdf
* Measuring Corner Properties: http://users.cs.cf.ac.uk/Paul.Rosin/corner2.pdf (check "Intensity centroid" used in ORB vs "Gradient centroid")
* Image moments: https://en.wikipedia.org/wiki/Image_moment
* Centroid: https://en.wikipedia.org/wiki/Centroid
*/
#include "compv/features/orb/compv_feature_orb_desc.h"
#include "compv/features/orb/compv_feature_orb_dete.h"
#include "compv/compv_math_utils.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"
#include "compv/compv_gauss.h"
#include "compv/compv_debug.h"

#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_sse.h"
#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_avx2.h"

#include <algorithm>

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void Brief256_31_Asm_X86_SSE41(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X86_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X86_FMA3_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */
#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void Brief256_31_Asm_X64_SSE41(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X64_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X64_FMA3_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#define _kBrief256Pattern31AX_ \
	8, 4, -11, 7, 2, 1, -2, -13, -13, 10, -13, -11, 7, -4, -13, -9, 12, -3, -6, 11,  \
	4, 5, 3, -8, -2, -13, -7, -4, -10, 5, 5, 1, 9, 4, 2, -4, -8, 4, 0, -13, -3, -6, \
	8, 0, 7, -13, 10, -6, 10, -13, -13, 3, 5, -1, 3, 2, -13, -13, -13, -7, 6, -9, -2, \
	-12, 3, -7, -3, 2, -11, -1, 5, -4, -9, -12, 10, 7, -7, -4, 7, -7, -13, -3, 7, \
	-13, 1, 2, -4, -1, 7, 1, 9, -1, -13, 7, 12, 6, 5, 2, 3, 2, 9, -8, -11, 1, 6, 2, \
	6, 3, 7, -11, -10, -5, -10, 8, 4, -10, 4, -2, -5, 7, -9, -5, 8, -9, 1, 7, -2, 11, \
	-12, 3, 5, 0, -9, 0, -1, 5, 3, -13, -5, -4, 6, -7, -13, 1, 4, -2, 2, -2, 4, -6, \
	-3, 7, 4, -13, 7, 7, -7, -8, -13, 2, 10, -6, 8, 2, -11, -12, -11, 5, -2, -1, -13, \
	-10, -3, 2, -9, -4, -4, -6, 6, -13, 11, 7, -1, -4, -7, -13, -7, -8, -5, -13, \
	1, 1, 9, 5, -1, -9, -1, -13, 8, 2, 7, -10, -10, 4, 3, -4, 5, 4, -9, 0, -12, 3, \
	-10, 8, -8, 2, 10, 6, -7, -3, -1, -3, -8, 4, 2, 6, 3, 11, -3, 4, 2, -10, -13, -13, \
	6, 0, -13, -9, -13, 5, 2, -1, 9, 11, 3, -1, 3, -13, 5, 8, 7, -10, 7, 9, 7, -1
#define _kBrief256Pattern31AY_ \
	-3, 2, 9, -12, -13, -7, -10, -13, -3, 4, -8, 7, 7, -5, 2, 0, -6, 6, -13, -13, 7, \
	-3, -7, -7, 11, 12, 3, 2, -12, -12, -6, 0, 11, 7, -1, -12, -5, 11, -8, -2, -2, \
	9, 12, 9, -5, -6, 7, -3, -9, 8, 0, 3, 7, 7, -10, -4, 0, -7, 3, 12, -10, -1, -5, \
	5, -10, -7, -2, 9, -13, 6, -3, -13, -6, -10, 2, 12, -13, 9, -1, 6, 11, 7, -8, -7, \
	-3, -6, 3, -13, 1, -1, 1, -9, -13, 7, -5, 3, -13, -12, 8, 6, -12, 4, 12, 12, -9, \
	3, 3, -3, 8, -5, 11, -8, 5, -1, -6, 12, -2, 0, -8, -6, -13, -13, -8, -11, -8, \
	-4, 1, -6, -9, 7, 5, -4, 12, 7, 2, 11, 5, -4, 9, -7, 5, 6, 6, -10, 1, -2, -12, \
	-13, 1, -10, -13, 5, -2, 9, 1, -8, -4, 11, 6, 4, -5, -5, -3, -12, -2, -13, 0, -3, \
	-13, -8, -11, -2, 9, -3, -13, 6, 12, -11, -3, 11, 11, -5, 12, -8, 1, -12, -2, \
	5, -1, 7, 5, 0, 12, -8, 11, -3, -10, 1, -11, -13, -13, -10, -8, -6, 12, 2, -13, \
	-13, 9, 3, 1, 2, -10, -13, -12, 2, 6, 8, 10, -9, -13, -7, -2, 2, -5, -9, -1, -1, \
	0, -11, -4, -6, 7, 12, 0, -1, 3, 8, -6, -9, 7, -6, 5, -3, 0, 4, -6, 0, 8, 9, -4, \
	4, 3, -7, 0, -6
#define _kBrief256Pattern31BX_ \
	9, 7, -8, 12, 2, 1, -2, -11, -12, 11, -8, -9, 12, -3, -12, -7, 12, -2, -4, 12, 5, \
	10, 6, -6, -1, -8, -5, -3, -6, 6, 7, 4, 11, 4, 4, -2, -7, 9, 1, -8, -2, -4, 10, \
	1, 11, -11, 12, -6, 12, -8, -8, 7, 10, 1, 5, 3, -13, -12, -11, -4, 12, -7, 0, \
	-7, 8, -4, -1, 5, -5, 0, 5, -4, -9, -8, 12, 12, -6, -3, 12, -5, -12, -2, 12, -11, \
	12, 3, -2, 1, 8, 3, 12, -1, -10, 10, 12, 7, 6, 2, 4, 12, 10, -7, -4, 2, 7, 3, \
	11, 8, 9, -6, -5, -3, -9, 12, 6, -8, 6, -2, -5, 10, -8, -5, 9, -9, 1, 9, -1, 12, \
	-6, 7, 10, 2, -5, 2, 1, 7, 6, -8, -3, -3, 8, -6, -5, 3, 8, 2, 12, 0, 9, -3, -1, \
	12, 5, -9, 8, 7, -7, -7, -12, 3, 12, -6, 9, 2, -10, -7, -10, 11, -1, 0, -12, -10, \
	-2, 3, -4, -3, -2, -4, 6, -5, 12, 12, 0, -3, -6, -8, -6, -6, -4, -8, 5, 10, 10, \
	10, 1, -6, 1, -8, 10, 3, 12, -5, -8, 8, 8, -3, 10, 5, -4, 3, -6, 4, -10, 12, \
	-6, 3, 11, 8, -6, -3, -1, -3, -8, 12, 3, 11, 7, 12, -3, 4, 2, -8, -11, -11, 11, \
	1, -9, -6, -8, 8, 3, -1, 11, 12, 3, 0, 4, -10, 12, 9, 8, -10, 12, 10, 12, 0
#define _kBrief256Pattern31BY_ \
	5, -12, 2, -13, 12, 6, -4, -8, -9, 9, -9, 12, 6, 0, -3, 5, -1, 12, -8, -8, 1, -3, \
	12, -2, -10, 10, -3, 7, 11, -7, -1, -5, -13, 12, 4, 7, -10, 12, -13, 2, 3, -9, \
	7, 3, -10, 0, 1, 12, -4, -12, -4, 8, -7, -12, 6, -10, 5, 12, 8, 7, 8, -6, 12, 5, \
	-13, 5, -7, -11, -13, -1, 2, 12, 6, -4, -3, 12, 5, 4, 2, 1, 5, -6, -7, -12, 12, \
	0, -13, 9, -6, 12, 6, 3, 5, 12, 9, 11, 10, 3, -6, -13, 3, 9, -6, -8, -4, -2, 0, \
	-8, 3, -4, 10, 12, 0, -6, -11, 7, 7, 12, 2, 12, -8, -2, -13, 0, -2, 1, -4, -11, \
	4, 12, 8, 8, -13, 12, 7, -9, -8, 9, -3, -12, 0, 12, -2, 10, -4, -13, 12, -6, 3, \
	-5, 1, -11, -7, -5, 6, 6, 1, -8, -8, 9, 3, 7, -8, 8, 3, -9, -5, 8, 12, 9, -5, \
	11, -13, 2, 0, -10, -7, 9, 11, 5, 6, -2, 7, -2, 7, -13, -8, -9, 5, 10, -13, -13, \
	-1, -9, -13, 2, 12, -10, -6, -6, -9, -7, -13, 5, -13, -3, -12, -1, 3, -9, 1, -8, \
	9, 12, -5, 7, -8, -12, 5, 9, 5, 4, 3, 12, 11, -13, 12, 4, 6, 12, 1, 1, 1, -13, \
	-13, 4, -2, -3, -2, 10, -9, -1, -2, -8, 5, 10, 5, 5, 11, -6, -12, 9, 4, -2, -2, -11

COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AX[256] = { _kBrief256Pattern31AX_ };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AY[256] = { _kBrief256Pattern31AY_ };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BX[256] = { _kBrief256Pattern31BX_ };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BY[256] = { _kBrief256Pattern31BY_ };
#if COMPV_FEATURE_DESC_ORB_FXPQ15
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AXInt16[256] = { _kBrief256Pattern31AX_ };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AYInt16[256] = { _kBrief256Pattern31AY_ };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BXInt16[256] = { _kBrief256Pattern31BX_ };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BYInt16[256] = { _kBrief256Pattern31BY_ };
#endif

COMPV_NAMESPACE_BEGIN()

// Default values from the detector
extern int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT;
extern int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT;
extern bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP;
extern int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
extern float COMPV_FEATURE_DETE_ORB_PYRAMID_SF;
extern int COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER;
extern int COMPV_FEATURE_DETE_ORB_PATCH_BITS;
extern COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE;

static const int COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE = 7;
static const float COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA = 2.f;

#define COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD	(20*10) // number of interestPoints

static void Brief256_31_Float32_C(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out);
#if COMPV_FEATURE_DESC_ORB_FXPQ15
static void Brief256_31_Fxpq15_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out);
#endif

CompVFeatureDescORB::CompVFeatureDescORB()
    : CompVFeatureDesc(COMPV_ORB_ID)
	, m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
	, m_nPatchBits(COMPV_FEATURE_DETE_ORB_PATCH_BITS)
	, m_funBrief256_31_Float32(Brief256_31_Float32_C)
#if COMPV_FEATURE_DESC_ORB_FXPQ15
	, m_funBrief256_31_Fxpq15(Brief256_31_Fxpq15_C)
#endif
{

}

CompVFeatureDescORB::~CompVFeatureDescORB()
{
}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDescORB::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case -1:
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

COMPV_ERROR_CODE CompVFeatureDescORB::convlt(CompVPtr<CompVImageScalePyramid * > pPyramid, int level)
{
	// apply gaussianblur filter on the pyramid
	CompVPtr<CompVImage*> imageAtLevelN;
	COMPV_CHECK_CODE_RETURN(pPyramid->getImage(level, &imageAtLevelN));
	// The out is the image itself to avoid allocating temp buffer. This means the images in the pyramid are modified
	// and any subsequent call must take care
	COMPV_CHECK_CODE_RETURN(m_convlt->convlt1((uint8_t*)imageAtLevelN->getDataPtr(), imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), m_kern->getDataPtr(), m_kern->getDataPtr(), COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, (uint8_t*)imageAtLevelN->getDataPtr()));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFeatureDescORB::describe(CompVPtr<CompVImageScalePyramid * > pPyramid, const CompVInterestPoint* begin, const CompVInterestPoint* end, uint8_t* desc)
{
	float fx, fy, angleInRad, sf, fcos, fsin;
	int32_t xi, yi;
	const CompVInterestPoint* point;
	CompVPtr<CompVImage*> imageAtLevelN;
	const int nFeaturesBytes = (m_nPatchBits >> 3);
	const int nPatchRadius = (m_nPatchDiameter >> 1);
	const uint8_t* img_center;
	int32_t stride;

	for (point = begin; point < end; ++point) {
		// Get image at level N
		COMPV_CHECK_CODE_RETURN(pPyramid->getImage(point->level, &imageAtLevelN));
		stride = imageAtLevelN->getStride();
		// Scale
		sf = pPyramid->getScaleFactor(point->level);
		fx = (point->x * sf);
		fy = (point->y * sf);
		// Convert the angle from degree to radian
		angleInRad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(point->orient);
		// Get angle's cos and sin
		fcos = ::cos(angleInRad);
		fsin = ::sin(angleInRad);
		// Round the point
		xi = COMPV_MATH_ROUNDFU_2_INT(fx, int32_t);
		yi = COMPV_MATH_ROUNDFU_2_INT(fy, int32_t);
		// Compute description
		{
			// Check if the keypoint is too close to the border
			if ((xi - nPatchRadius) < 0 || (xi + nPatchRadius) >= imageAtLevelN->getWidth() || (yi - nPatchRadius) < 0 || (yi + nPatchRadius) >= imageAtLevelN->getHeight()) {
				// Must never happen....unless you are using keypoints from another implementation (e.g. OpenCV)
				COMPV_DEBUG_ERROR("Keypoint too close to the border");
				memset(desc, 0, nFeaturesBytes);
			}
			else {
				img_center = ((const uint8_t*)imageAtLevelN->getDataPtr()) + ((yi * stride) + xi); // Translate the image to have the keypoint at the center. This is required before applying the rotated patch.
#if COMPV_FEATURE_DESC_ORB_FXPQ15 // Disable fixed-point version until we found faster implementation
				if (CompVEngine::isMathFixedPoint()) {
					// cosT and sinT are within [0-1] which means we can just mulb 0x7fff
					int16_t cosTQ15 = COMPV_MATH_ROUNDF_2_INT((cosT * 0x7fff), int16_t);
					int16_t sinTQ15 = COMPV_MATH_ROUNDF_2_INT((sinT * 0x7fff), int16_t);
					m_funBrief256_31_Fxpq15(img_center, imageAtLevelN->getStride(), &cosTQ15, &sinTQ15, desc);
				}
				else
#endif
				{
					m_funBrief256_31_Float32(img_center, stride, &fcos, &fsin, desc);
				}
				desc += nFeaturesBytes;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// override CompVFeatureDesc::process
COMPV_ERROR_CODE CompVFeatureDescORB::process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVFeatureDescriptions*>* descriptions)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE || !descriptions || !interestPoints || interestPoints->empty(),
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// For now only Brief256_31 is supported
	COMPV_CHECK_EXP_RETURN(m_nPatchBits != 256 || m_nPatchDiameter != 31, COMPV_ERROR_CODE_E_INVALID_CALL);
	
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVPtr<CompVFeatureDescriptions*> _descriptions;
    CompVPtr<CompVImageScalePyramid * > _pyramid;
    CompVPtr<CompVImage*> imageAtLevelN;
    CompVPtr<CompVFeatureDete*> attachedDete = getAttachedDete();
    uint8_t* _descriptionsPtr = NULL;
	static const bool size_of_float_is4 = (sizeof(float) == 4); // ASM and INTRIN code require it
	CompVPtr<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
	int threadsCount = 1;
	CompVPtr<CompVFeatureDescORB* >This = this;

    // return COMPV_ERROR_CODE_S_OK;

	const int nFeatures = (int)interestPoints->size();
	const int nFeaturesBits = m_nPatchBits;
    const int nFeaturesBytes = nFeaturesBits >> 3;
    static size_t COMPV_FEATURE_DESC_ORB_SIMD_ELMT_COUNT_ALIGNED = (size_t)CompVMem::alignForward(COMPV_FEATURE_DESC_ORB_SIMD_ELMT_COUNT, COMPV_SIMD_ALIGNV_DEFAULT /* should be 32, we don't need best alignment (64)*/);
    COMPV_CHECK_CODE_RETURN(err_ = CompVFeatureDescriptions::newObj(nFeatures, nFeaturesBits, &_descriptions)); // TODO(dmi): realloc instead of alloc()
    _descriptionsPtr = (uint8_t*)_descriptions->getDataPtr();

    // Get the pyramid from the detector or use or own pyramid
    if ((attachedDete = getAttachedDete())) {
        switch (attachedDete->getId()) {
        case COMPV_ORB_ID: {
            const void* valuePtr = NULL;
            COMPV_CHECK_CODE_RETURN(err_ = attachedDete->get(COMPV_FEATURE_GET_PTR_PYRAMID, valuePtr, sizeof(CompVImageScalePyramid)));
            _pyramid = (CompVImageScalePyramid*)(valuePtr);
            break;
        }
        }
    }
    if (!_pyramid) {
        // This code is called when we fail to get a pyramid from the attached detector or when none is attached.
        // The pyramid should come from the detector. Attach a detector to this descriptor to give it access to the pyramid.
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
        COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image)); // multithreaded function
        _pyramid = m_pyramid;
    }

	// Compute number of threads
	if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
		threadsCount = threadDip->getThreadsCount();
	}

	// apply gaussianblur filter on the pyramid
	if (threadsCount > 1) {
		uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
		// levelStart is used to make sure we won't schedule more than "threadsCount"
		int levelStart, level, levelMax;
		for (levelStart = 0, levelMax = threadsCount; levelStart < _pyramid->getLevels(); levelStart += threadsCount, levelMax += threadsCount) {
			for (level = levelStart; level < _pyramid->getLevels() && level < levelMax; ++level) {
				COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + level), COMPV_TOKENIDX0, CompVFeatureDescORB::convlt_AsynExec,
					COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *_pyramid, level),
					COMPV_ASYNCTASK_SET_PARAM_NULL()));
			}
			for (level = levelStart; level < _pyramid->getLevels() && level < levelMax; ++level) {
				COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + level), COMPV_TOKENIDX0));
			}
		}
	}
	else {
		for (int level = 0; level < _pyramid->getLevels(); ++level) {
			COMPV_CHECK_CODE_RETURN(err_ = convlt(_pyramid, level));
		}
	}    
	
	/* Init "m_funBrief256_31" using current CPU flags */
#if COMPV_FEATURE_DESC_ORB_FXPQ15
	if (CompVEngine::isMathFixedPoint()) {
		if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Fxpq15 = Brief256_31_Fxpq15_Intrin_SSE2);
		}
	}
	else
#endif
	{
		if (size_of_float_is4) {
			if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
				COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Float32 = Brief256_31_Intrin_SSE2);
			}
			if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
				COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_SSE41);
				COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_SSE41);
			}
			if (compv::CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
				COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Float32 = Brief256_31_Intrin_AVX2);
				COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_AVX2);
				COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_AVX2);
				if (compv::CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
					COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_FMA3_AVX2);
					COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_FMA3_AVX2);
				}
			}
		}
	}

	// Describe the points
	int32_t threadsCountDescribe = 1;
	if (threadsCount > 1) {
		threadsCountDescribe = (int32_t)(interestPoints->size() / COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD);
		threadsCountDescribe = COMPV_MATH_CLIP3(0, threadsCountDescribe, threadsCount);
	}
	if (threadsCountDescribe > 1) {
		uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
		const CompVInterestPoint* begin = interestPoints->begin();
		int32_t total = (int32_t)interestPoints->size();
		int32_t count = total / threadsCountDescribe;
		uint8_t* desc = _descriptionsPtr;
		for (int32_t i = 0; count > 0 && i < threadsCountDescribe; ++i) {
			COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX0, describe_AsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *_pyramid, begin, begin + count, desc),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
			begin += count;
			desc += (count * nFeaturesBytes);
			total -= count;
			if (i == (threadsCountDescribe - 2)) {
				count = (total); // the remaining
			}
		}
		for (int32_t i = 0; i < threadsCountDescribe; ++i) {
			COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX0));
		}
	}
	else {
		COMPV_CHECK_CODE_RETURN(err_ = describe(_pyramid, interestPoints->begin(), interestPoints->end(), _descriptionsPtr));
	}

    *descriptions = _descriptions;

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDescORB::newObj(CompVPtr<CompVFeatureDesc* >* orb)
{
    COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVImageScalePyramid * > pyramid_;
	CompVPtr<CompVArray<float>* > kern_;
	CompVPtr<CompVConvlt<float>* > convlt_;
    // Create Gauss kernel values
	COMPV_CHECK_CODE_RETURN(CompVGaussKern<float>::buildKern1(&kern_, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA));
    // Create convolution context
	COMPV_CHECK_CODE_RETURN(CompVConvlt<float>::newObj(&convlt_));
    // Create the pyramid
    COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE, &pyramid_));

    CompVPtr<CompVFeatureDescORB* >_orb = new CompVFeatureDescORB();
    if (!_orb) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    _orb->m_pyramid = pyramid_;
    _orb->m_kern = kern_;
    _orb->m_convlt = convlt_;

    *orb = *_orb;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFeatureDescORB::convlt_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
	CompVPtr<CompVFeatureDescORB* >  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVFeatureDescORB*);
	CompVPtr<CompVImageScalePyramid* > pyramid = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImageScalePyramid*);
	int level = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, int);
	return This->convlt(pyramid, level);
}

COMPV_ERROR_CODE CompVFeatureDescORB::describe_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
	CompVPtr<CompVFeatureDescORB* >  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVFeatureDescORB*);
	CompVPtr<CompVImageScalePyramid* > pyramid = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImageScalePyramid*);
	const CompVInterestPoint* begin = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, const CompVInterestPoint*);
	const CompVInterestPoint* end = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, const CompVInterestPoint*);
	uint8_t* desc = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, uint8_t*);
	return This->describe(pyramid, begin, end, desc);
}

static void Brief256_31_Float32_C(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	static const uint64_t u64_1 = 1;
	uint64_t* _out = (uint64_t*)out;
	int i, j, x, y;
	uint8_t a, b;
	float xf, yf, cosT = *cos1, sinT = *sin1;

	// 256bits = 32Bytes = 4 uint64
	_out[0] = _out[1] = _out[2] = _out[3] = 0;

	// Applying rotation matrix to each (x, y) point in the patch gives us:
	// xr = x*cosT - y*sinT and yr = x*sinT + y*cosT
	for (i = 0, j = 0; i < 256; ++i) {
		xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
		yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
		x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		a = img_center[(y * img_stride) + x];

		xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
		yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
		x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		b = img_center[(y * img_stride) + x];

		_out[0] |= (a < b) ? (u64_1 << j) : 0;
		if (++j == 64) {
			++_out;
			j = 0;
		}
	}
}

#if COMPV_FEATURE_DESC_ORB_FXPQ15
static void Brief256_31_Fxpq15_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	static const uint64_t u64_1 = 1;
	uint64_t* _out = (uint64_t*)out;
	int i, j, x, y;
	uint8_t a, b;
	int cosT = *cos1;
	int sinT = *sin1;

	// 256bits = 32Bytes = 4 uint64
	_out[0] = _out[1] = _out[2] = _out[3] = 0;

	// Applying rotation matrix to each (x, y) point in the patch gives us:
	// xr = x*cosT - y*sinT and yr = x*sinT + y*cosT
	for (i = 0, j = 0; i < 256; ++i) {
		x = (kBrief256Pattern31AXInt16[i] * cosT - kBrief256Pattern31AYInt16[i] * sinT) >> 15;
		y = (kBrief256Pattern31AXInt16[i] * sinT + kBrief256Pattern31AYInt16[i] * cosT) >> 15;
		a = img_center[(y * img_stride) + x];

		x = (kBrief256Pattern31BXInt16[i] * cosT - kBrief256Pattern31BYInt16[i] * sinT) >> 15;
		y = (kBrief256Pattern31BXInt16[i] * sinT + kBrief256Pattern31BYInt16[i] * cosT) >> 15;
		b = img_center[(y * img_stride) + x];

		_out[0] |= (a < b) ? (u64_1 << j) : 0;
		if (++j == 64) {
			++_out;
			j = 0;
		}
	}
}
#endif /* COMPV_FEATURE_DESC_ORB_FXPQ15 */

COMPV_NAMESPACE_END()
