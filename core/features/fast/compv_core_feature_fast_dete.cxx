/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/* @description
This class implement FAST (Features from Accelerated Segment Test) algorithm.
Some literature about FAST:
- http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/AV1011/AV1FeaturefromAcceleratedSegmentTest.pdf
- https://en.wikipedia.org/wiki/Features_from_accelerated_segment_test
- http://www.edwardrosten.com/work/fast.html
- http://web.eecs.umich.edu/~silvio/teaching/EECS598_2010/slides/11_16_Hao.pdf
*/

#include "compv/core/features/fast/compv_core_feature_fast_dete.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math_utils.h"

#include "compv/core/features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_sse2.h"
#include "compv/core/features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_avx2.h"
#include "compv/core/features/fast/intrin/arm/compv_core_feature_fast_dete_intrin_neon.h"

// This is the only file where we include plein intrinsic because there are no asm implementations 'CompVFastBuildInterestPoints'
#if COMPV_ARCH_X64 || defined(__SSE2__) // SSE2 is mandatory on x64
#	if defined(_MSC_VER)
#		include <intrin.h>
#	elif defined(__GNUC__)
#		include <x86intrin.h>
#	endif
#endif

#define COMPV_THIS_CLASSNAME	"CompVCornerDeteFAST"

COMPV_NAMESPACE_BEGIN()
#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVFast9DataRow_Asm_X86_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFast12DataRow_Asm_X86_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFastNmsGather_Asm_X86_SSE2(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVFastNmsApply_Asm_X86_SSE2(COMPV_ALIGNED(SSE) uint8_t* pcStrengthsMap, COMPV_ALIGNED(SSE) uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVFast9DataRow_Asm_X86_AVX2(const uint8_t* IP, COMPV_ALIGNED(AVX) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFast12DataRow_Asm_X86_AVX2(const uint8_t* IP, COMPV_ALIGNED(AVX) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFastNmsGather_Asm_X86_AVX2(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVFastNmsApply_Asm_X86_AVX2(COMPV_ALIGNED(AVX) uint8_t* pcStrengthsMap, COMPV_ALIGNED(AVX) uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVFast9DataRow_Asm_X64_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFast12DataRow_Asm_X64_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFastNmsGather_Asm_X64_SSE2(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVFast9DataRow_Asm_X64_AVX2(const uint8_t* IP, COMPV_ALIGNED(AVX) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFast12DataRow_Asm_X64_AVX2(const uint8_t* IP, COMPV_ALIGNED(AVX) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFastNmsGather_Asm_X64_AVX2(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM
	COMPV_EXTERNC void CompVFastDataRow_Asm_NEON32(const uint8_t* IP, COMPV_ALIGNED(NEON) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
	COMPV_EXTERNC void CompVFastNmsGather_Asm_NEON32(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVFastNmsApply_Asm_NEON32(COMPV_ALIGNED(NEON) uint8_t* pcStrengthsMap, COMPV_ALIGNED(NEON) uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#	endif
#endif /* COMPV_ASM */

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT			20
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP				true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES				2000 // maximum number of features to retain (<0 means all)
#define COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD		(200*250) // number of pixels
#define COMPV_FEATURE_DETE_FAST_NMS_MIN_SAMPLES_PER_THREAD	(80*80) // number of interestPoints

static int32_t COMPV_INLINE __continuousCount(int32_t fasType) {
    switch (fasType) {
    case COMPV_FAST_TYPE_9: return 9;
    case COMPV_FAST_TYPE_12: return 12;
    default:
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid fastType:%d", fasType);
        return 9;
    }
}

static void CompVFastDataRange(RangeFAST* range);
static void CompVFastBuildInterestPoints(RangeFAST* range, std::vector<CompVInterestPoint>& interestPoints);
static void CompVFastNmsGatherRange(RangeFAST* range);
static void CompVFastNmsApplyRangeAndBuildInterestPoints(RangeFAST* range, std::vector<CompVInterestPoint>& interestPoints);
static void CompVFastDataRow_C(const uint8_t* IP,  compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
static void CompVFastNmsGather_C(const uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride);
static void CompVFastNmsApply_C(uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride);
static COMPV_ERROR_CODE FastRangesAlloc(size_t nRanges, RangeFAST** ppRanges, size_t stride);
static COMPV_ERROR_CODE FastRangesFree(size_t nRanges, RangeFAST** ppRanges);

CompVCornerDeteFAST::CompVCornerDeteFAST()
    : CompVCornerDete(COMPV_FAST_ID)
    , m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
    , m_iType(COMPV_FAST_TYPE_9)
    , m_iNumContinuous(__continuousCount(COMPV_FAST_TYPE_9))
	, m_iMaxFeatures(COMPV_FEATURE_DETE_FAST_MAX_FEATURTES)
    , m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nStride(0)
    , m_pRanges(NULL)
    , m_nRanges(0)
    , m_pStrengthsMap(NULL)
	, m_pNmsMap(NULL)
{

}

CompVCornerDeteFAST::~CompVCornerDeteFAST()
{
    FastRangesFree(m_nRanges, &m_pRanges);
    CompVMem::free(reinterpret_cast<void**>(&m_pStrengthsMap));
	CompVMem::free(reinterpret_cast<void**>(&m_pNmsMap));
}

COMPV_ERROR_CODE CompVCornerDeteFAST::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
    COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_FAST_SET_INT_THRESHOLD: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int threshold = *reinterpret_cast<const int*>(valuePtr);
        m_iThreshold = COMPV_MATH_CLIP3(0, 255, threshold);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_INT_MAX_FEATURES: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_iMaxFeatures = *reinterpret_cast<const int*>(valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_INT_FAST_TYPE: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        int iType = *reinterpret_cast<const int*>(valuePtr);
        COMPV_CHECK_EXP_RETURN(iType != COMPV_FAST_TYPE_9 && iType != COMPV_FAST_TYPE_12, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_iType = iType;
        m_iNumContinuous = __continuousCount(iType);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_bNonMaximaSupp = *reinterpret_cast<const bool*>(valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
    }
}

// overrides CompVCornerDete::process
COMPV_ERROR_CODE CompVCornerDeteFAST::process(const CompVMatPtr& image, std::vector<CompVInterestPoint>& interestPoints) /*Overrides(CompVCornerDete)*/
{
    COMPV_CHECK_EXP_RETURN(!image || image->isEmpty() || image->subType() != COMPV_SUBTYPE_PIXELS_Y,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	const uint8_t* dataPtr = image->ptr<const uint8_t>();
    size_t width = image->cols();
	size_t height = image->rows();
	size_t stride = image->stride();
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
    size_t threadsCountRange = 1;

    COMPV_CHECK_EXP_RETURN(width < 4 || height < 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid size (too short)");

    // Free ranges memory if stride increase
    if (m_nStride < stride) {
        FastRangesFree(m_nRanges, &m_pRanges);
        m_nRanges = 0;
    }

    // Even if newStride < oldStride, free the strenghtMap to cleanup old values.
    // FastData() function will cleanup only new matching positions.
    if (m_nStride != stride) {
        CompVMem::free(reinterpret_cast<void**>(&m_pStrengthsMap));
		CompVMem::free(reinterpret_cast<void**>(&m_pNmsMap));
    }

    // Alloc strenghts map if not already done
    if (!m_pStrengthsMap || !m_pNmsMap) {
        size_t nMapSize = CompVMem::alignForward((3 + stride + 3) * (3 + height + 3)); // +3 for the borders, alignForward() for the SIMD functions
		// Must use calloc to fill the strengths with null values
		if (!m_pStrengthsMap) {
			m_pStrengthsMap = reinterpret_cast<uint8_t*>(CompVMem::calloc(nMapSize, sizeof(uint8_t)));
			COMPV_CHECK_EXP_RETURN(!m_pStrengthsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate strengths map");
		}
		if (!m_pNmsMap) {
			m_pNmsMap = reinterpret_cast<uint8_t*>(CompVMem::calloc(nMapSize, sizeof(uint8_t)));
			COMPV_CHECK_EXP_RETURN(!m_pNmsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate nms map");
		}
    }

    // Update width and height
    m_nWidth = width;
    m_nHeight = height;
    m_nStride = stride;

	interestPoints.clear();

	const compv_scalar_t strideScalarSigned = static_cast<compv_scalar_t>(stride);
    COMPV_ALIGN_DEFAULT() const compv_scalar_t pixels16[16] = {
        -(strideScalarSigned * 3) + 0, // 1
        -(strideScalarSigned * 3) + 1, // 2
        -(strideScalarSigned * 2) + 2, // 3
        -(strideScalarSigned * 1) + 3, // 4
        +(strideScalarSigned * 0) + 3, // 5
        +(strideScalarSigned * 1) + 3, // 6
        +(strideScalarSigned * 2) + 2, // 7
        +(strideScalarSigned * 3) + 1, // 8
        +(strideScalarSigned * 3) + 0, // 9
        +(strideScalarSigned * 3) - 1, // 10
        +(strideScalarSigned * 2) - 2, // 11
        +(strideScalarSigned * 1) - 3, // 12
        +(strideScalarSigned * 0) - 3, // 13
        -(strideScalarSigned * 1) - 3, // 14
        -(strideScalarSigned * 2) - 2, // 15
        -(strideScalarSigned * 3) - 1, // 16
    };

    // Compute number of threads
	threadsCountRange = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD)
		: 1;

    // Alloc ranges
    if (m_nRanges < threadsCountRange) {
        m_nRanges = 0;
        COMPV_CHECK_CODE_RETURN(FastRangesAlloc(threadsCountRange, &m_pRanges, m_nStride));
        m_nRanges = threadsCountRange;
    }

    if (threadsCountRange > 1) {
		size_t rowStart = 0;
		size_t heights = (height / threadsCountRange);
		size_t lastHeight = height - ((threadsCountRange - 1) * heights);
        RangeFAST* pRange;
        CompVAsyncTaskIds taskIds;
		std::vector<std::vector<CompVInterestPoint> >interestPointsList;
		if (!m_bNonMaximaSupp) {
			interestPointsList.resize(threadsCountRange);
		}
        taskIds.reserve(threadsCountRange);
        auto funcPtr = [&](RangeFAST* pRange, size_t idx) -> void {
            CompVFastDataRange(pRange);
			if (interestPointsList.size() > idx) { // when nomax is enabled then, building the points is done while applying the nomax (mt-friendly)
				CompVFastBuildInterestPoints(pRange, interestPointsList[idx]);
			}
        };
        for (size_t i = 0; i < threadsCountRange; ++i) {
            pRange = &m_pRanges[i];
            pRange->IP = dataPtr;
            pRange->rowStart = rowStart;
            pRange->rowEnd = (rowStart + (i == (threadsCountRange - 1) ? lastHeight : heights));
            pRange->rowCount = height;
            pRange->width = width;
            pRange->stride = stride;
            pRange->threshold = m_iThreshold;
            pRange->N = m_iNumContinuous;
            pRange->pixels16 = pixels16;
            pRange->strengths = m_pStrengthsMap;
			pRange->nms = m_pNmsMap;
            COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, pRange, i), taskIds));
            rowStart = pRange->rowEnd;
        }
		if (interestPointsList.empty()) {
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		}
		else {
			for (size_t i = 0; i < threadsCountRange; ++i) {
				COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[i]));
				interestPoints.insert(interestPoints.end(), interestPointsList[i].begin(), interestPointsList[i].end());
			}
		}
    } 
	else {
        RangeFAST* pRange = &m_pRanges[0];
        pRange->IP = dataPtr;
        pRange->rowStart = 0;
        pRange->rowEnd = height;
        pRange->rowCount = height;
        pRange->width = width;
        pRange->stride = stride;
        pRange->threshold = m_iThreshold;
        pRange->N = m_iNumContinuous;
        pRange->pixels16 = pixels16;
        pRange->strengths = m_pStrengthsMap;
		pRange->nms = m_pNmsMap;
        CompVFastDataRange(pRange);
		if (!m_bNonMaximaSupp) { // when nomax is enabled then, building the points is done while applying the nomax (mt-friendly)
			CompVFastBuildInterestPoints(pRange, interestPoints);
		}
    }

	// Non Maximal Suppression for removing adjacent corners
	if (m_bNonMaximaSupp) {
		size_t threadsCountNMS = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
			? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_FEATURE_DETE_FAST_NMS_MIN_SAMPLES_PER_THREAD)
			: 1;
		threadsCountNMS = COMPV_MATH_MIN(threadsCountRange, threadsCountNMS); // only 'threadsCountRange' values were allocated

		if (threadsCountNMS > 1) {
			size_t threadIdx, rowStart;
			size_t heights = (height / threadsCountNMS);
			size_t lastHeight = height - ((threadsCountNMS - 1) * heights);
			std::vector<std::vector<CompVInterestPoint> >interestPointsList;
			RangeFAST* pRange;
			CompVAsyncTaskIds taskIds;
			taskIds.reserve(threadsCountNMS);
			interestPointsList.resize(threadsCountNMS);
			auto funcPtrNmsGather = [&](RangeFAST* pRange) -> void {
				CompVFastNmsGatherRange(pRange);
			};
			auto funcPtrNmsApplyRangeAndBuildInterestPoints = [&](RangeFAST* pRange, size_t idx) -> void {
				CompVFastNmsApplyRangeAndBuildInterestPoints(pRange, interestPointsList[idx]);
			};
			// NMS gathering
			for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountNMS; ++threadIdx) {
				pRange = &m_pRanges[threadIdx];
				pRange->rowStart = rowStart;
				pRange->rowEnd = (rowStart + (threadIdx == (threadsCountNMS - 1) ? lastHeight : heights));
				pRange->rowCount = height;
				pRange->width = width;
				pRange->stride = stride;
				pRange->strengths = m_pStrengthsMap;
				pRange->nms = m_pNmsMap;
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsGather, pRange), taskIds));
				rowStart = pRange->rowEnd;
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
			// NMS-apply
			taskIds.clear();
			for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountNMS; ++threadIdx) {
				pRange = &m_pRanges[threadIdx];
				pRange->rowStart = rowStart;
				pRange->rowEnd = (rowStart + (threadIdx == (threadsCountNMS - 1) ? lastHeight : heights));
				pRange->rowCount = height;
				pRange->width = width;
				pRange->stride = stride;
				pRange->strengths = m_pStrengthsMap;
				pRange->nms = m_pNmsMap;
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsApplyRangeAndBuildInterestPoints, pRange, threadIdx), taskIds));
				rowStart = pRange->rowEnd;
			}
			for (threadIdx = 0; threadIdx < threadsCountNMS; ++threadIdx) {
				COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
				interestPoints.insert(interestPoints.end(), interestPointsList[threadIdx].begin(), interestPointsList[threadIdx].end());
			}
		}
		else {
			RangeFAST* pRange = &m_pRanges[0];
			pRange->rowStart = 0;
			pRange->rowEnd = height;
			pRange->rowCount = height;
			pRange->width = width;
			pRange->stride = stride;
			pRange->strengths = m_pStrengthsMap;
			pRange->nms = m_pNmsMap;

			CompVFastNmsGatherRange(pRange);
			CompVFastNmsApplyRangeAndBuildInterestPoints(pRange, interestPoints);
		}
	}

    // Retain best "m_iMaxFeatures" features
    if (m_iMaxFeatures > 1 && static_cast<int32_t>(interestPoints.size()) > m_iMaxFeatures) {
		CompVInterestPoint::selectBest(interestPoints, static_cast<size_t>(m_iMaxFeatures));
    }
    return err_;
}

COMPV_ERROR_CODE CompVCornerDeteFAST::newObj(CompVCornerDetePtrPtr fast)
{
    COMPV_CHECK_EXP_RETURN(fast == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVCornerDeteFASTPtr _fast = new CompVCornerDeteFAST();
	COMPV_CHECK_EXP_RETURN(!_fast, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *fast = *_fast;
    return COMPV_ERROR_CODE_S_OK;
}

static void CompVFastDataRange(RangeFAST* range)
{
    const uint8_t* IP;
    int32_t j, kalign, kextra, align = 1, minj, maxj, rowstart, k;
    uint8_t *strengths, *extra;
    void(*FastDataRow)(const uint8_t* IP, compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths) 
		= CompVFastDataRow_C;
#if COMPV_ARCH_X86
    if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((FastDataRow = CompVFastDataRow_Intrin_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
		COMPV_EXEC_IFDEF_ASM_X86((FastDataRow = range->N == 9 ? CompVFast9DataRow_Asm_X86_SSE2 : CompVFast12DataRow_Asm_X86_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
        COMPV_EXEC_IFDEF_ASM_X64((FastDataRow = range->N == 9 ? CompVFast9DataRow_Asm_X64_SSE2 : CompVFast12DataRow_Asm_X64_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
    }
    if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((FastDataRow = CompVFastDataRow_Intrin_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
		COMPV_EXEC_IFDEF_ASM_X86((FastDataRow = range->N == 9 ? CompVFast9DataRow_Asm_X86_AVX2 : CompVFast12DataRow_Asm_X86_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
		COMPV_EXEC_IFDEF_ASM_X64((FastDataRow = range->N == 9 ? CompVFast9DataRow_Asm_X64_AVX2 : CompVFast12DataRow_Asm_X64_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
    }
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagNone)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((FastDataRow = CompVFastDataRow_Intrin_NEON, align = COMPV_SIMD_ALIGNV_NEON));
		//COMPV_EXEC_IFDEF_ASM_ARM((FastDataRow = CompVFastDataRow_Asm_NEON32, align = COMPV_SIMD_ALIGNV_NEON));
	}
#endif

    // Number of pixels to process (multiple of align)
    kalign = static_cast<int32_t>(CompVMem::alignForward((-3 + range->width - 3), align));
    if (kalign > static_cast<int32_t>(range->stride - 3)) { // must never happen
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Unexpected code called. k16=%d, stride=%zu", kalign, range->stride);
        COMPV_ASSERT(false);
        return;
    }
    // Number of pixels to ignore
    kextra = kalign - (-3 + static_cast<int32_t>(range->width) - 3);

    rowstart = static_cast<int32_t>(range->rowStart);
    minj = (rowstart == 0 ? 3 : 0);
    maxj = static_cast<int32_t>((range->rowEnd - rowstart) - ((range->rowCount - range->rowEnd) <= 3 ? 3 - (range->rowCount - range->rowEnd) : 0));
    IP = range->IP + ((rowstart + minj) * range->stride) + 3;
    strengths = range->strengths + ((rowstart + minj) * range->stride) + 3;

    for (j = minj; j < maxj; ++j) {
		FastDataRow(IP, kalign, range->pixels16, range->N, range->threshold, strengths);
        // remove extra samples
        extra = &strengths[kalign - 1];
        for (k = 0; k < kextra; ++k) {
            *extra-- = 0;
        }
        IP += range->stride;
        strengths += range->stride;
    } // for (j)
}

static void CompVFastBuildInterestPoints(RangeFAST* range, std::vector<CompVInterestPoint>& interestPoints)
{
	// TODO(dmi): Performance isssues! Not cache-friendly
	size_t rowStart = range->rowStart > 3 ? range->rowStart - 3 : range->rowStart;
	size_t rowEnd = COMPV_MATH_CLIP3(0, range->rowCount, (range->rowEnd + 3));
	size_t rowSride = range->stride;
	size_t rowWidth = range->width;
	uint8_t *strengths = range->strengths + ((rowStart + 3) * rowSride);
	const uint8_t thresholdMinus1 = static_cast<uint8_t>(range->threshold - 1);

	// There is no asm implementation for this function and this is why we use intrinsics regardless 'COMPV_INTRINSIC'

#if COMPV_ARCH_X86
#	if COMPV_INTRINSIC
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(strengths)) {
		CompVFastBuildInterestPoints_Intrin_AVX2(strengths, interestPoints, thresholdMinus1, (rowStart + 3), (rowEnd - 3), rowWidth, rowSride);
		return;
	}
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(strengths)) {
		CompVFastBuildInterestPoints_Intrin_SSE2(strengths, interestPoints, thresholdMinus1, (rowStart + 3), (rowEnd - 3), rowWidth, rowSride);
		return;
	}
#endif
#	if COMPV_ARCH_X64 || defined(__SSE2__) // SSE2 is mandatory on x64
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(strengths)) {
		int mask;
#define COMPV_PUSH_SSE2(ii) \
		if (mask & (1 << ii)) { \
			interestPoints.push_back(CompVInterestPoint( \
				static_cast<compv_float32_t>(i + ii), \
				static_cast<compv_float32_t>(j), \
				static_cast<compv_float32_t>(strengths[i + ii] + thresholdMinus1))); \
		}
		static const __m128i vecZero = _mm_setzero_si128();
		static const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
		for (size_t j = (rowStart + 3); j < (rowEnd - 3); ++j) {
			for (size_t i = 0; i < rowWidth; i += 16) {
				if ((mask = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&strengths[i])), vecZero), vec0xFF)))) {
					COMPV_PUSH_SSE2(0); COMPV_PUSH_SSE2(1); COMPV_PUSH_SSE2(2); COMPV_PUSH_SSE2(3); COMPV_PUSH_SSE2(4); COMPV_PUSH_SSE2(5); COMPV_PUSH_SSE2(6); COMPV_PUSH_SSE2(7);
					COMPV_PUSH_SSE2(8); COMPV_PUSH_SSE2(9); COMPV_PUSH_SSE2(10); COMPV_PUSH_SSE2(11); COMPV_PUSH_SSE2(12); COMPV_PUSH_SSE2(13); COMPV_PUSH_SSE2(14); COMPV_PUSH_SSE2(15);
				}
			}
			strengths += rowSride;
		}
#undef COMPV_PUSH_SSE2
		return;
	}
#	endif /* COMPV_ARCH_X64 || defined(__SSE2__) */
#elif COMPV_ARCH_ARM && 0 // C++ code faster
#	if COMPV_INTRINSIC
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(strengths)) {
		CompVFastBuildInterestPoints_Intrin_NEON(strengths, interestPoints, thresholdMinus1, (rowStart + 3), (rowEnd - 3), rowWidth, rowSride);
		return;
	}
#endif
#endif /* COMPV_ARCH_X86 */

#if 0 // Not a big deal
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
#endif

#define COMPV_PUSH1() if (*begin1) { *begin1 += thresholdMinus1; interestPoints.push_back(CompVInterestPoint(static_cast<compv_float32_t>(begin1 - strengths), static_cast<compv_float32_t>(j), static_cast<compv_float32_t>(*begin1))); } ++begin1;
#define COMPV_PUSH4() COMPV_PUSH1() COMPV_PUSH1() COMPV_PUSH1() COMPV_PUSH1()
#define COMPV_PUSH8() COMPV_PUSH4() COMPV_PUSH4()
	uint8_t *begin1;
	if (COMPV_IS_ALIGNED(rowSride, 64) && COMPV_IS_ALIGNED(CompVCpu::cache1LineSize(), 64)) {
		uint64_t *begin8, *end8;
		size_t width_div8 = rowWidth >> 3;
		for (size_t j = (rowStart + 3); j < rowEnd - 3; ++j) {
			begin8 = reinterpret_cast<uint64_t*>(strengths + 0); // i can start at +3 but we prefer +0 because strengths[0] is cacheline-aligned
			end8 = (begin8 + width_div8);
			do {
				if (*begin8) {
					begin1 = reinterpret_cast<uint8_t*>(begin8);
					COMPV_PUSH8();
				}
			} while (begin8++ < end8);
			strengths += rowSride;
		}
	}
	else {
		uint32_t *begin4, *end4;
		size_t width_div4 = rowWidth >> 2;
		for (size_t j = (rowStart + 3); j < rowEnd - 3; ++j) {
			begin4 = reinterpret_cast<uint32_t*>(strengths + 0); // i can start at +3 but we prefer +0 because strengths[0] is cacheline-aligned
			end4 = (begin4 + width_div4);
			do {
				if (*begin4) {
					begin1 = reinterpret_cast<uint8_t*>(begin4);
					COMPV_PUSH4();
				}
			} while (begin4++ < end4);
			strengths += rowSride;
		}
	}
}

void CompVFastNmsGatherRange(RangeFAST* range)
{
	void(*CompVFastNmsGather)(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
		= CompVFastNmsGather_C;

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(range->stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVFastNmsGather = CompVFastNmsGather_Intrin_SSE2));
		COMPV_EXEC_IFDEF_ASM_X86((CompVFastNmsGather = CompVFastNmsGather_Asm_X86_SSE2));
		COMPV_EXEC_IFDEF_ASM_X64((CompVFastNmsGather = CompVFastNmsGather_Asm_X64_SSE2));
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(range->stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVFastNmsGather = CompVFastNmsGather_Intrin_AVX2));
		COMPV_EXEC_IFDEF_ASM_X86((CompVFastNmsGather = CompVFastNmsGather_Asm_X86_AVX2));
		COMPV_EXEC_IFDEF_ASM_X64((CompVFastNmsGather = CompVFastNmsGather_Asm_X64_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(range->stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVFastNmsGather = CompVFastNmsGather_Intrin_NEON));
		COMPV_EXEC_IFDEF_ASM_ARM((CompVFastNmsGather = CompVFastNmsGather_Asm_NEON32));
	}
#endif

	size_t rowStart = range->rowStart > 3 ? range->rowStart - 3 : range->rowStart;
	size_t rowEnd = COMPV_MATH_CLIP3(0, range->rowCount, (range->rowEnd + 3));
	CompVFastNmsGather(
		range->strengths + (range->stride * rowStart),
		range->nms + (range->stride * rowStart),
		range->width,
		(rowEnd - rowStart),
		range->stride
	);
}

void CompVFastNmsApplyRangeAndBuildInterestPoints(RangeFAST* range, std::vector<CompVInterestPoint>& interestPoints)
{
	void(*CompVFastNmsApply)(uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
		= CompVFastNmsApply_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(range->stride) && COMPV_IS_ALIGNED_SSE(range->strengths) && COMPV_IS_ALIGNED_SSE(range->nms)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVFastNmsApply = CompVFastNmsApply_Intrin_SSE2));
		COMPV_EXEC_IFDEF_ASM_X86((CompVFastNmsApply = CompVFastNmsApply_Asm_X86_SSE2));
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(range->stride) && COMPV_IS_ALIGNED_AVX2(range->strengths) && COMPV_IS_ALIGNED_AVX2(range->nms)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVFastNmsApply = CompVFastNmsApply_Intrin_AVX2));
		COMPV_EXEC_IFDEF_ASM_X86((CompVFastNmsApply = CompVFastNmsApply_Asm_X86_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(range->stride) && COMPV_IS_ALIGNED_NEON(range->strengths) && COMPV_IS_ALIGNED_NEON(range->nms)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVFastNmsApply = CompVFastNmsApply_Intrin_NEON));
		COMPV_EXEC_IFDEF_ASM_ARM((CompVFastNmsApply = CompVFastNmsApply_Asm_NEON32));
	}
#endif
	size_t rowStart = range->rowStart > 3 ? range->rowStart - 3 : range->rowStart;
	size_t rowEnd = COMPV_MATH_CLIP3(0, range->rowCount, (range->rowEnd + 3));

	// Apply nms (suppress the nonmax points)
	CompVFastNmsApply(
		range->strengths + (range->stride * rowStart),
		range->nms + (range->stride * rowStart),
		range->width,
		(rowEnd - rowStart),
		range->stride
	);

	// Build interest points
	CompVFastBuildInterestPoints(range, interestPoints);
}

static void CompVFastDataRow_C(const uint8_t* IP, compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths)
{
	// Code not intended to be fast but just readable, real code is implemented in SSE, AVX and NEON.
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	static const uint16_t kCompVFast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
	static const uint16_t kCompVFast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };
	int sumb, sumd, sb, sd;
	uint8_t threshold_ = static_cast<uint8_t>(threshold); // using int16_t to avoid clipping (useless for SIMD with support for saturated sub and add)
	uint8_t neighborhoods16[16], strength, t0, t1, brighter, darker;
	const uint16_t(&FastXFlags)[16] = N == 9 ? kCompVFast9Flags : kCompVFast12Flags;
	compv_uscalar_t flags;
	const int minsum = (N == 12 ? 3 : 2);
	int n = static_cast<int>(N);
	compv_uscalar_t i, j, k, arcStart;
	const uint8_t* circle[16] = {
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};

#define _cpp_fast_check(a, b, c, d) \
		t0 = circle[a][i], t1 = circle[b][i]; \
		sd = (t0 < darker) + (t1 < darker), sb = (t0 > brighter) + (t1 > brighter); \
		if (!(sd || sb)) goto Next; \
		sumd += sd, sumb += sb; \
		t0 = circle[c][i], t1 = circle[d][i]; \
		sd = (t0 < darker) + (t1 < darker), sb = (t0 > brighter) + (t1 > brighter); \
		if (!(sd || sb)) goto Next; \
		sumd += sd, sumb += sb; \
		if (sumd < minsum && sumb < minsum) goto Next

#define _cpp_fast_strenght() \
	flags = \
		(neighborhoods16[0] ? (1 << 0) : 0) \
		| (neighborhoods16[1] ? (1 << 1) : 0) \
		| (neighborhoods16[2] ? (1 << 2) : 0) \
		| (neighborhoods16[3] ? (1 << 3) : 0) \
		| (neighborhoods16[4] ? (1 << 4) : 0) \
		| (neighborhoods16[5] ? (1 << 5) : 0) \
		| (neighborhoods16[6] ? (1 << 6) : 0) \
		| (neighborhoods16[7] ? (1 << 7) : 0) \
		| (neighborhoods16[8] ? (1 << 8) : 0) \
		| (neighborhoods16[9] ? (1 << 9) : 0) \
		| (neighborhoods16[10] ? (1 << 10) : 0) \
		| (neighborhoods16[11] ? (1 << 11) : 0) \
		| (neighborhoods16[12] ? (1 << 12) : 0) \
		| (neighborhoods16[13] ? (1 << 13) : 0) \
		| (neighborhoods16[14] ? (1 << 14) : 0) \
		| (neighborhoods16[15] ? (1 << 15) : 0); \
		for (arcStart = 0; arcStart < 16; ++arcStart) { \
			if ((flags & FastXFlags[arcStart]) == FastXFlags[arcStart]) { \
				t0 = 0xff; \
				for (j = arcStart, k = 0; k < N; ++j, ++k) { \
					t0 = std::min(neighborhoods16[j & 15], t0); \
				} \
				strength = std::max(strength, t0); \
			} \
		}


	for (i = 0; i < width; ++i, ++IP, ++strengths) {
		brighter = CompVMathUtils::clampPixel8(IP[0] + threshold_); // SSE: paddusb
		darker = CompVMathUtils::clampPixel8(IP[0] - threshold_); // SSE: psubusb
		strength = sumb = sumd = 0;
		
		_cpp_fast_check(0, 8, 4, 12);
		_cpp_fast_check(1, 9, 5, 13);
		_cpp_fast_check(2, 10, 6, 14);
		_cpp_fast_check(3, 11, 7, 15);
		
		if (sumd >= n) {
			t0 = circle[0][i], neighborhoods16[0] = (darker > t0) ? (darker - t0) : 0; // SSE: psubusb
			t0 = circle[1][i], neighborhoods16[1] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[2][i], neighborhoods16[2] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[3][i], neighborhoods16[3] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[4][i], neighborhoods16[4] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[5][i], neighborhoods16[5] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[6][i], neighborhoods16[6] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[7][i], neighborhoods16[7] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[8][i], neighborhoods16[8] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[9][i], neighborhoods16[9] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[10][i], neighborhoods16[10] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[11][i], neighborhoods16[11] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[12][i], neighborhoods16[12] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[13][i], neighborhoods16[13] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[14][i], neighborhoods16[14] = (darker > t0) ? (darker - t0) : 0;
			t0 = circle[15][i], neighborhoods16[15] = (darker > t0) ? (darker - t0) : 0;
			_cpp_fast_strenght();
		}
		else if (sumb >= n) { // else -> cannot be both darker and brighter and and (16 - N) < N, for N = 9, 12...
			t0 = circle[0][i], neighborhoods16[0] = (t0 > brighter) ? (t0 - brighter) : 0; // SSE: psubusb
			t0 = circle[1][i], neighborhoods16[1] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[2][i], neighborhoods16[2] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[3][i], neighborhoods16[3] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[4][i], neighborhoods16[4] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[5][i], neighborhoods16[5] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[6][i], neighborhoods16[6] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[7][i], neighborhoods16[7] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[8][i], neighborhoods16[8] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[9][i], neighborhoods16[9] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[10][i], neighborhoods16[10] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[11][i], neighborhoods16[11] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[12][i], neighborhoods16[12] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[13][i], neighborhoods16[13] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[14][i], neighborhoods16[14] = (t0 > brighter) ? (t0 - brighter) : 0;
			t0 = circle[15][i], neighborhoods16[15] = (t0 > brighter) ? (t0 - brighter) : 0;
			_cpp_fast_strenght();
		}
		
Next:
		*strengths = strength;
	} // for (i ....width)	
}

static void CompVFastNmsGather_C(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	compv_uscalar_t i, j;
	uint8_t strength;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 3; i < width - 3; ++i) {
			if ((strength = pcStrengthsMap[i])) { 
				// If-Else faster than a single if(|||||||)
				if (pcStrengthsMap[i - 1] >= strength) { // left
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + 1] >= strength) { // right
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i - stride - 1] >= strength) { // left-top
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i - stride] >= strength) { // top
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i - stride + 1] >= strength) { // right-top
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + stride - 1] >= strength) { // left-bottom
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + stride] >= strength) { // bottom
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + stride + 1] >= strength) { // right-bottom
					pNMS[i] = 0xff; continue;
				}
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

static void CompVFastNmsApply_C(uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	compv_uscalar_t i, j;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 3; i < width - 3; ++i) { // SIMD: start at #zero index to have aligned memory
			if (pNMS[i]) {
				pNMS[i] = 0; // must, for next frame
				pcStrengthsMap[i] = 0; // suppress
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

static COMPV_ERROR_CODE FastRangesAlloc(size_t nRanges, RangeFAST** ppRanges, size_t stride)
{
    /* COMPV_DEBUG_INFO("FAST: alloc %d ranges", nRanges); */

    COMPV_CHECK_EXP_RETURN(nRanges <= 0 || !ppRanges, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_CHECK_CODE_RETURN(FastRangesFree(nRanges, ppRanges));

    *ppRanges = reinterpret_cast<RangeFAST*>(CompVMem::calloc(nRanges, sizeof(RangeFAST)));
    COMPV_CHECK_EXP_RETURN(!*ppRanges, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
#if 0
    RangeFAST* pRanges = *ppRanges;
    for (int32_t i = 0; i < nRanges; ++i) {

        pRanges[i].me = (compv_uscalar_t*)CompVMem::malloc(stride * 1 * sizeof(compv_uscalar_t));
        COMPV_CHECK_EXP_RETURN(!pRanges[i].me, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
#endif
    return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE FastRangesFree(size_t nRanges, RangeFAST** ppRanges)
{
    if (ppRanges && *ppRanges) {
#if 0
        RangeFAST* pRanges = *ppRanges;
        for (int32_t i = 0; i < nRanges; ++i) {
            CompVMem::free((void**)&pRanges[i].me);
        }
#endif
        CompVMem::free(reinterpret_cast<void**>(ppRanges));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
