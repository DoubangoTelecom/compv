/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/compv_core_feature_houghkht.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghkht_intrin_sse2.h"
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghkht_intrin_avx.h"
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghkht_intrin_avx2.h"
#include "compv/core/features/hough/intrin/arm/compv_core_feature_houghkht_intrin_neon.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"

#include <algorithm> /* std::reverse */
#include <float.h> /* DBL_MAX */
#include <iterator> /* std::back_inserter */

#define COMPV_THIS_CLASSNAME	"CompVHoughKht"

#define COMPV_FEATURE_HOUGHKHT_VOTES_COUNT_MIN_SAMPLES_PER_THREAD (40)
#define COMPV_FEATURE_HOUGHKHT_CLUSTERS_FIND_MIN_SAMPLES_PER_THREAD (40 * 1)
#define COMPV_FEATURE_HOUGHKHT_PEAKS_VOTES_MIN_SAMPLES_PER_THREAD (40 * 1)

// Documentation:
//	- http://www2.ic.uff.br/~laffernandes/projects/kht/
//	- https://www.academia.edu/11637890/Real-time_line_detection_through_an_improved_Hough_transform_voting_scheme
//	- https://en.wikipedia.org/wiki/Hough_transform#Kernel-based_Hough_transform_.28KHT.29
//	- http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5388&rep=rep1&type=pdf

#define COMPV_HOUGHKHT_CLUSTER_MIN_DEVIATION		2.0
#define COMPV_HOUGHKHT_CLUSTER_MIN_SIZE				10
#define COMPV_HOUGHKHT_KERNEL_MIN_HEIGTH			0.002

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVHoughKhtKernelHeight_2mpq_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(SSE) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(SSE) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(SSE) const compv_float64_t* n_scale,
		COMPV_ALIGNED(SSE) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(SSE) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(SSE) compv_float64_t* m2, COMPV_ALIGNED(SSE) compv_float64_t* sigma_theta_square,
		COMPV_ALIGNED(SSE) compv_float64_t* height, COMPV_ALIGNED(SSE) compv_float64_t* heightMax1, COMPV_ALIGNED(SSE) compv_uscalar_t count);
	COMPV_EXTERNC void CompVHoughKhtKernelHeight_4mpq_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(AVX) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(AVX) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(AVX) const compv_float64_t* n_scale,
		COMPV_ALIGNED(AVX) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(AVX) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(AVX) compv_float64_t* m2, COMPV_ALIGNED(AVX) compv_float64_t* sigma_theta_square,
		COMPV_ALIGNED(AVX) compv_float64_t* height, COMPV_ALIGNED(AVX) compv_float64_t* heightMax1, COMPV_ALIGNED(AVX) compv_uscalar_t count);
	COMPV_EXTERNC void CompVHoughKhtKernelHeight_4mpq_Asm_X64_FMA3_AVX(COMPV_ALIGNED(AVX) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(AVX) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(AVX) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(AVX) const compv_float64_t* n_scale,
		COMPV_ALIGNED(AVX) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(AVX) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(AVX) compv_float64_t* m2, COMPV_ALIGNED(AVX) compv_float64_t* sigma_theta_square,
		COMPV_ALIGNED(AVX) compv_float64_t* height, COMPV_ALIGNED(AVX) compv_float64_t* heightMax1, COMPV_ALIGNED(AVX) compv_uscalar_t count);
#   elif COMPV_ARCH_ARM32
    COMPV_EXTERNC void CompVHoughKhtKernelHeight_4mpq_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(NEON) const compv_float64_t* n_scale,
        COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(AVX) compv_float64_t* m2, COMPV_ALIGNED(NEON) compv_float64_t* sigma_theta_square,
        COMPV_ALIGNED(NEON) compv_float64_t* height, COMPV_ALIGNED(NEON) compv_float64_t* heightMax1, COMPV_ALIGNED(NEON) compv_uscalar_t count);
    COMPV_EXTERNC void CompVHoughKhtKernelHeight_4mpq_Asm_VFPV4_NEON32(COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(NEON) const compv_float64_t* n_scale,
       COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(AVX) compv_float64_t* m2, COMPV_ALIGNED(NEON) compv_float64_t* sigma_theta_square,
       COMPV_ALIGNED(NEON) compv_float64_t* height, COMPV_ALIGNED(NEON) compv_float64_t* heightMax1, COMPV_ALIGNED(NEON) compv_uscalar_t count);
#   elif COMPV_ARCH_ARM64
    COMPV_EXTERNC void CompVHoughKhtKernelHeight_4mpq_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(NEON) const compv_float64_t* n_scale,
        COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(AVX) compv_float64_t* m2, COMPV_ALIGNED(NEON) compv_float64_t* sigma_theta_square,
        COMPV_ALIGNED(NEON) compv_float64_t* height, COMPV_ALIGNED(NEON) compv_float64_t* heightMax1, COMPV_ALIGNED(NEON) compv_uscalar_t count);
#	endif /* COMPV_ARCH_X64 */
#endif /* COMPV_ASM */

static const KHT_TYP KHT_TYP_ZERO = static_cast<KHT_TYP>(0);
static const KHT_TYP KHT_TYP_HALF = static_cast<KHT_TYP>(0.5);
static const KHT_TYP KHT_TYP_ONE = static_cast<KHT_TYP>(1);
static const KHT_TYP KHT_TYP_TWO = static_cast<KHT_TYP>(2);
static const KHT_TYP KHT_TYP_TWOPI = static_cast<KHT_TYP>(2.0 * COMPV_MATH_PI);
static const KHT_TYP KHT_TYP_FOUR = static_cast<KHT_TYP>(4);
static const KHT_TYP KHT_TYP_RAD_TO_DEG_SCALE = static_cast<KHT_TYP>(180.0 / COMPV_MATH_PI);

// Fast exp function for small numbers
// https://en.wikipedia.org/wiki/Taylor_series#Exponential_function
COMPV_ALWAYS_INLINE KHT_TYP __compv_math_exp_fast_small(KHT_TYP x) {
#if 1
	static const KHT_TYP scale = static_cast<KHT_TYP>(1.0 / 1024.0);
	x = KHT_TYP_ONE + (x * scale);
	x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x;
	return x;
#else
	return std::exp(x);
#endif
}

// https://www.beyond3d.com/content/articles/8/
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
COMPV_ALWAYS_INLINE compv_float64_t __compv_math_rsqrt_fast(compv_float64_t x) {
	compv_float64_t xhalf = 0.5 * x;
	int64_t i = *reinterpret_cast<int64_t*>(&x);
	i = 0x5fe6eb50c7b537a9 - (i >> 1);
	x = *reinterpret_cast<compv_float64_t*>(&i);
	x = x*(1.5 - xhalf*x*x);
	return x;
}
#if COMPV_ARCH_X64 && COMPV_INTRINSIC // X64 always support SSE2
static const __m128d vec0 = _mm_set_sd(0.);
#define __compv_math_sqrt_fast(x) _mm_cvtsd_f64(_mm_sqrt_sd(vec0, _mm_set_sd(x)))
COMPV_ALWAYS_INLINE KHT_TYP __compv_math_sqrt_fast2(const KHT_TYP x, const KHT_TYP y) {
	const __m128d vecsqrt = _mm_sqrt_pd(_mm_set_pd(x, y));
	return _mm_cvtsd_f64(_mm_mul_sd(vecsqrt, _mm_shuffle_pd(vecsqrt, vecsqrt, 0x1)));
}
#elif defined(__GNUC__)
#define __compv_math_sqrt_fast(x)		__builtin_sqrt(x)
#define __compv_math_sqrt_fast2(x, y)	(__builtin_sqrt((x)) * __builtin_sqrt((y)))
#else
#define __compv_math_sqrt_fast(x)		std::sqrt(x)
#define __compv_math_sqrt_fast2(x, y)	(std::sqrt((x)) * std::sqrt((y)))
#endif

static void CompVHoughKhtPeaks_Section3_4_VotesCount_C(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes);

CompVHoughKht::CompVHoughKht(float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(1.f), size_t threshold COMPV_DEFAULT(1))
	:CompVHough(COMPV_HOUGHKHT_ID)
	, m_dRho(static_cast<KHT_TYP>(rho * 1.f))
	, m_dTheta_rad(static_cast<KHT_TYP>(theta * kfMathTrigPiOver180))
	, m_cluster_min_deviation(static_cast<KHT_TYP>(COMPV_HOUGHKHT_CLUSTER_MIN_DEVIATION))
	, m_cluster_min_size(COMPV_HOUGHKHT_CLUSTER_MIN_SIZE)
	, m_kernel_min_heigth(static_cast<KHT_TYP>(COMPV_HOUGHKHT_KERNEL_MIN_HEIGTH))
	, m_dGS(KHT_TYP_ONE)
	, m_nThreshold(threshold)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nMaxLines(INT_MAX)
	, m_bOverrideInputEdges(false)
{
    m_poss.reserve(500);
    m_strings.reserve(500);
}

CompVHoughKht:: ~CompVHoughKht()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVHoughKht::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_HOUGH_SET_FLT32_RHO: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f || *reinterpret_cast<const compv_float32_t*>(valuePtr) > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fRho = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(static_cast<KHT_TYP>(fRho), m_dTheta_rad, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_FLT32_THETA: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fTheta = *reinterpret_cast<const compv_float32_t*>(valuePtr) * kfMathTrigPiOver180;
		COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, static_cast<KHT_TYP>(fTheta), m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int nThreshold = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, m_dTheta_rad, static_cast<size_t>(nThreshold), m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_MAXLINES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nMaxLines = static_cast<size_t>(*reinterpret_cast<const int*>(valuePtr) <= 0 ? INT_MAX : *reinterpret_cast<const int*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_cluster_min_deviation = static_cast<KHT_TYP>(*reinterpret_cast<const compv_float32_t*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_cluster_min_size = static_cast<size_t>(*reinterpret_cast<const int*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) < 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_kernel_min_heigth = static_cast<KHT_TYP>(*reinterpret_cast<const compv_float32_t*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGHKHT_SET_BOOL_OVERRIDE_INPUT_EDGES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_bOverrideInputEdges = *reinterpret_cast<const bool*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	default: {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	}
}

COMPV_ERROR_CODE CompVHoughKht::get(int id, const void** valuePtrPtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtrPtr || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_HOUGHKHT_GET_FLT64_GS: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float64_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		**((compv_float64_t**)valuePtrPtr) = m_dGS;
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return CompVCaps::get(id, valuePtrPtr, valueSize);
	}
}

COMPV_ERROR_CODE CompVHoughKht::process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions COMPV_DEFAULT(NULL)) /*Overrides(CompVHough)*/
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Edges null or empty");
	COMPV_CHECK_EXP_RETURN(edges->elmtInBytes() != sizeof(uint8_t) || edges->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Edges must be 8U_1D (e.g. grayscale image)");
    
	lines.clear();
	m_strings.clear();
	m_poss.clear();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	size_t threadsCountClusters = 1, threadsCountPeaks = 1, threadsCountVotes = 1;
	CompVAsyncTaskIds taskIds;
	size_t threadIdx, countAny, countLast;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVHoughKhtClusters clusters_all;
	CompVHoughKhtKernels kernels_all;
	CompVHoughKhtVotes votes_all;
    COMPV_ALIGN_DEFAULT() KHT_TYP hmax = 0.0; // for backward compatibility, make hmax aligned
    KHT_TYP Gmin = static_cast<KHT_TYP>(std::is_same<KHT_TYP, compv_float64_t>::value ? DBL_MAX : FLT_MAX);

	if (maxThreads > 1) {
		std::vector<KHT_TYP > hmax_mt;
		std::vector<KHT_TYP > Gmin_mt;
		std::vector<CompVHoughKhtKernels > kernels_mt;
		auto funcPtrClone = [&]() -> COMPV_ERROR_CODE {
			if (m_bOverrideInputEdges) {
				m_edges = edges;
			}
			else {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("If you're not reusing the edges you should let us know");
#if 0
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Multiple memcpy");
				COMPV_CHECK_CODE_RETURN(edges->clone(&m_edges));
#else
				COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&m_edges, COMPV_SUBTYPE_PIXELS_Y, edges->cols(), edges->rows(), edges->stride()));
				COMPV_CHECK_EXP_RETURN(m_edges->dataSizeInBytes() != edges->dataSizeInBytes(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
				COMPV_CHECK_CODE_RETURN(CompVMem::copy(m_edges->ptr<void>(), edges->ptr<const void*>(), edges->dataSizeInBytes()));
#endif
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrInitCoordsAndClearMaps = [&]() -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, m_dTheta_rad, m_nThreshold, edges->cols(), edges->rows())); // Should be
			COMPV_CHECK_CODE_RETURN(m_count->zero_rows()); // required before calling 'voting_Algorithm2_Count'
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrVotingHmax = [&](size_t index, CompVHoughKhtStrings::const_iterator strings_begin, CompVHoughKhtStrings::const_iterator strings_end) -> COMPV_ERROR_CODE {
			CompVHoughKhtClusters clusters_mt;
			/* Clusters */
			COMPV_CHECK_CODE_RETURN(clusters_find(clusters_mt, strings_begin, strings_end));
			/* Voting (build kernels and compute hmax) */
			COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Kernels(clusters_mt, kernels_mt[index], hmax_mt[index]));
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrDiscardShortKernelsAndGmin = [&](size_t index)  -> COMPV_ERROR_CODE {
			/* Voting (Erase short kernels) */
			COMPV_CHECK_CODE_RETURN(voting_Algorithm2_DiscardShortKernels(kernels_mt[index], hmax));
			/* Voting (Compute Gmin) */
			COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Gmin(kernels_mt[index], Gmin_mt[index]));
			return COMPV_ERROR_CODE_S_OK;
		};
		
		/* Clone the edges (the linking procedure modifify the data) */
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrClone), taskIds));
		/* Init coords (sine and cosine tables) */
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrInitCoordsAndClearMaps), taskIds));
		// Wait for the tasks to complete
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));

		/* Appendix A. Linking procedure ( not thread-safe) */
		COMPV_CHECK_CODE_RETURN(linking_AppendixA(m_edges, m_strings));
		if (m_strings.empty()) {
			return COMPV_ERROR_CODE_S_OK;
		}

		/* MT (Clusters and Voting for hmax and kernels) */
		threadsCountClusters = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, m_strings.size(), maxThreads, COMPV_FEATURE_HOUGHKHT_CLUSTERS_FIND_MIN_SAMPLES_PER_THREAD);
		if (threadsCountClusters > 1) {
			countAny = (m_strings.size() / threadsCountClusters);
			countLast = countAny + (m_strings.size() % threadsCountClusters);
			taskIds.clear();
			hmax_mt.resize(threadsCountClusters);
			Gmin_mt.resize(threadsCountClusters);
			kernels_mt.resize(threadsCountClusters);
			CompVHoughKhtStrings::const_iterator strings_begin = m_strings.begin();
			for (threadIdx = 0; threadIdx < threadsCountClusters - 1; ++threadIdx, strings_begin += countAny) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrVotingHmax, threadIdx, strings_begin, (strings_begin + countAny)), taskIds));
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrVotingHmax, (threadsCountClusters - 1), strings_begin, (strings_begin + countLast)), taskIds));
			for (threadIdx = 0; threadIdx < threadsCountClusters; ++threadIdx) {
				COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
				if (hmax < hmax_mt[threadIdx]) {
					hmax = hmax_mt[threadIdx];
				}
			}

			/* Voting (Erase short kernels) + (Compute Gmin) */
			taskIds.clear();
			for (threadIdx = 0; threadIdx < threadsCountClusters; ++threadIdx) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrDiscardShortKernelsAndGmin, threadIdx), taskIds));
			}
			for (threadIdx = 0; threadIdx < threadsCountClusters; ++threadIdx) {
				COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
				if (!kernels_mt[threadIdx].empty()) {
					if (Gmin_mt[threadIdx] < Gmin) {
						Gmin = Gmin_mt[threadIdx];
					}
					kernels_all.insert(kernels_all.end(), kernels_mt[threadIdx].begin(), kernels_mt[threadIdx].end());
				}
			}
			if (kernels_all.empty()) {
				return COMPV_ERROR_CODE_S_OK;
			}
		}		
	}
	else {
		/* Clone the edges (the linking procedure modifify the data) */
		if (m_bOverrideInputEdges) {
			m_edges = edges;
		}
		else {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("If you're not reusing the edges you should let us know");
#if 0
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Multiple memcpy");
			COMPV_CHECK_CODE_RETURN(edges->clone(&m_edges));
#else
			COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&m_edges, COMPV_SUBTYPE_PIXELS_Y, edges->cols(), edges->rows(), edges->stride()));
			COMPV_CHECK_EXP_RETURN(m_edges->dataSizeInBytes() != edges->dataSizeInBytes(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
			COMPV_CHECK_CODE_RETURN(CompVMem::copy(m_edges->ptr<void>(), edges->ptr<const void*>(), edges->dataSizeInBytes()));
#endif
		}

		/* Init coords (sine and cosine tables) */
		COMPV_CHECK_CODE_RETURN(initCoords(m_dRho, m_dTheta_rad, m_nThreshold, edges->cols(), edges->rows()));

		/* Appendix A. Linking procedure */
		COMPV_CHECK_CODE_RETURN(linking_AppendixA(m_edges, m_strings));
		if (m_strings.empty()) {
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	
	if (threadsCountClusters <= 1) {
		/* Clusters */
		COMPV_CHECK_CODE_RETURN(clusters_find(clusters_all, m_strings.begin(), m_strings.end()));
		if (clusters_all.empty()) {
			return COMPV_ERROR_CODE_S_OK;
		}

		/* Voting (build kernels and compute hmax) */
		COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Kernels(clusters_all, kernels_all, hmax)); // IS thread-safe
		if (kernels_all.empty()) {
			return COMPV_ERROR_CODE_S_OK;
		}

		/* Voting (Erase short kernels) */
		COMPV_CHECK_CODE_RETURN(voting_Algorithm2_DiscardShortKernels(kernels_all, hmax)); // IS thread-safe
		if (kernels_all.empty()) {
			return COMPV_ERROR_CODE_S_OK;
		}

		/* Voting (Compute Gmin) */
		COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Gmin(kernels_all, Gmin)); // IS thread-safe
	}

	/* {Scale factor for integer votes} */
	m_dGS = (Gmin == KHT_TYP_ZERO) ? KHT_TYP_ONE : std::max((KHT_TYP_ONE / Gmin), KHT_TYP_ONE);
	
	/* Voting (Count votes) */
	if (maxThreads <= 1) { // If MT enabled then, zeroing is calling asynchronously (see funcPtrInitCoordsAndClearMaps)
		COMPV_CHECK_CODE_NOP(m_count->zero_all()); // required before calling 'voting_Algorithm2_Count'
	}
	
#if 0 // Doesn't make sense and is slooo
	threadsCountVotes = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, kernels_all.size(), maxThreads, 1); // voting_Algorithm2_Count calls Algorithm4 which is very CPU intensive -> use all available threads
	if (threadsCountVotes > 1) {
		countAny = (kernels_all.size() / threadsCountVotes);
		countLast = countAny + (kernels_all.size() % threadsCountVotes);
		std::vector<CompVMemZeroInt32Ptr > voting_counts_mt;
		voting_counts_mt.resize(threadsCountVotes - 1);
		taskIds.clear();
		auto funcPtrVotingCount = [&](size_t index, CompVHoughKhtKernels::const_iterator kernels_begin, CompVHoughKhtKernels::const_iterator kernels_end, const KHT_TYP Gs) -> COMPV_ERROR_CODE {
			if (index) {
				CompVMemZeroInt32Ptr& counts = voting_counts_mt[index - 1];
				COMPV_CHECK_CODE_RETURN(CompVMemZeroInt32::newObj(&counts, m_count->rows(), m_count->cols(), m_count->stride()));
				COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Count(counts->ptr(), counts->stride(), kernels_begin, kernels_end, m_dGS));
			}
			else {
				COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Count(m_count->ptr<int32_t>(), m_count->stride(), kernels_begin, kernels_end, m_dGS));
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		CompVHoughKhtKernels::const_iterator kernels_begin = kernels_all.begin();
		for (threadIdx = 0; threadIdx < threadsCountVotes - 1; ++threadIdx, kernels_begin += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrVotingCount, threadIdx, kernels_begin, (kernels_begin + countAny), m_dGS), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrVotingCount, (threadsCountVotes - 1), kernels_begin, (kernels_begin + countLast), m_dGS), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[0])); // wait for m_count
		for (threadIdx = 1; threadIdx < threadsCountVotes; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum2<int32_t, int32_t>(m_count->ptr<const int32_t>(), voting_counts_mt[threadIdx - 1]->ptr(), m_count->ptr<int32_t>(),
				m_count->cols(), m_count->rows(), m_count->stride())));
		}
	}
	else
#endif
	{
		COMPV_CHECK_CODE_RETURN(voting_Algorithm2_Count(m_count->ptr<int32_t>(), m_count->stride(), kernels_all.begin(), kernels_all.end(), m_dGS));
	}

	/* Peaks detection */
	threadsCountPeaks = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(m_rho->cols(), (m_theta->cols() - 1), maxThreads, COMPV_FEATURE_HOUGHKHT_PEAKS_VOTES_MIN_SAMPLES_PER_THREAD);
	if (threadsCountVotes > 1) {
		size_t index_start;
		std::vector<CompVHoughKhtVotes > votes_mt;
		countAny = ((m_theta->cols() - 1) / threadsCountPeaks);
		countLast = countAny + ((m_theta->cols() - 1) % threadsCountPeaks);
		taskIds.clear();
		votes_mt.resize(threadsCountPeaks);
		auto funcPtrVotesCountAndClearVisitedMap = [&](size_t index, size_t theta_index_start, size_t theta_index_end)  -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(peaks_Section3_4_VotesCountAndClearVisitedMap(votes_mt[index], theta_index_start, theta_index_end)); // IS thread-safe
			return COMPV_ERROR_CODE_S_OK;
		};
		for (threadIdx = 0, index_start = 1; threadIdx < threadsCountPeaks - 1; ++threadIdx, index_start += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrVotesCountAndClearVisitedMap, threadIdx, index_start, (index_start + countAny)), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrVotesCountAndClearVisitedMap, (threadsCountPeaks - 1), index_start, (index_start + countLast)), taskIds));
		for (threadIdx = 0; threadIdx < threadsCountPeaks; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			if (!votes_mt[threadIdx].empty()) {
				votes_all.insert(votes_all.end(), votes_mt[threadIdx].begin(), votes_mt[threadIdx].end());
			}
		}
	}
	else {
		COMPV_CHECK_CODE_RETURN(peaks_Section3_4_VotesCountAndClearVisitedMap(votes_all, 1, m_theta->cols())); // IS thread-safe
	}
	COMPV_CHECK_CODE_RETURN(peaks_Section3_4_VotesSort(votes_all)); // NOT thread-safe
	COMPV_CHECK_CODE_RETURN(peaks_Section3_4_Lines(lines, votes_all)); // NOT thread-safe

	/* Retain best lines */
	const size_t maxLines = COMPV_MATH_MIN(lines.size(), m_nMaxLines);
	lines.resize(maxLines); // already sorted in 'peaks_Section3_4'
	
	return err;
}

COMPV_ERROR_CODE CompVHoughKht::toCartesian(const size_t imageWidth, const size_t imageHeight, const CompVHoughLineVector& polar, CompVLineFloat32Vector& cartesian)
{
	COMPV_CHECK_EXP_RETURN(!imageWidth || !imageHeight, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	cartesian.clear();
	if (polar.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	cartesian.resize(polar.size());
	size_t k = 0;
	const compv_float32_t widthF = static_cast<compv_float32_t>(imageWidth);
	const compv_float32_t heightF = static_cast<compv_float32_t>(imageHeight);
	const compv_float32_t half_widthF = widthF * 0.5f;
	const compv_float32_t half_heightF = heightF * 0.5f;
	for (CompVHoughLineVector::const_iterator i = polar.begin(); i < polar.end(); ++i) {
		const compv_float32_t rho = i->rho;
		const compv_float32_t theta = i->theta;
		const compv_float32_t a = std::cos(theta), b = 1.f / std::sin(theta);
		CompVLineFloat32& cline = cartesian[k++];
		cline.a.x = 0;
		cline.a.y = ((rho + (half_widthF * a)) * b) + half_heightF;
		cline.b.x = widthF;
		cline.b.y = ((rho - (half_widthF * a)) * b) + half_heightF;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughKht::newObj(CompVHoughPtrPtr hough, float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
{
	COMPV_CHECK_EXP_RETURN(!hough || rho <= 0 || rho > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVHoughPtr hough_ = new CompVHoughKht(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughKht::initCoords(KHT_TYP dRho, KHT_TYP dTheta, size_t nThreshold, size_t nWidth COMPV_DEFAULT(0), size_t nHeight COMPV_DEFAULT(0))
{
	nWidth = !nWidth ? m_nWidth : nWidth;
	nHeight = !nHeight ? m_nHeight : nHeight;
	if (m_dRho != dRho || m_dTheta_rad != dTheta || m_nWidth != nWidth || m_nHeight != nHeight) {
		KHT_TYP r0, *ptr0;
		const KHT_TYP dTheta_deg = static_cast<KHT_TYP>(COMPV_MATH_RADIAN_TO_DEGREE(m_dTheta_rad));
		const KHT_TYP r = __compv_math_sqrt_fast(static_cast<KHT_TYP>((nWidth * nWidth) + (nHeight * nHeight)));
		const size_t maxRhoCount = static_cast<size_t>(((r + KHT_TYP_ONE) / dRho));
		const size_t maxThetaCount = static_cast<size_t>(180.0 / dTheta_deg);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<KHT_TYP>(&m_rho, 1, maxRhoCount));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<KHT_TYP>(&m_theta, 1, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&m_count, (maxThetaCount + 2), (maxRhoCount + 2)));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&m_visited, (maxThetaCount + 2), (maxRhoCount + 2)));
		
		// rho filling
		r0 = -(r * KHT_TYP_HALF);
		ptr0 = m_rho->ptr<KHT_TYP>();
		for (size_t i = 1; i < maxRhoCount; ++i, r0 += dRho) {
			ptr0[i] = r0;
		}

		// theta filling
		r0 = KHT_TYP_ZERO;
		ptr0 = m_theta->ptr<KHT_TYP>();
		for (size_t i = 1; i < maxThetaCount; ++i, r0 += dTheta_deg) {
			ptr0[i] = r0;
		}

		COMPV_CHECK_CODE_RETURN(m_count->zero_all());
		COMPV_CHECK_CODE_RETURN(m_visited->zero_all());

		m_dRho = dRho;
		m_dTheta_rad = dTheta;
		m_dTheta_deg = dTheta_deg;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}
	m_nThreshold = nThreshold;
	return COMPV_ERROR_CODE_S_OK;
}

// Appendix A. Linking procedure
COMPV_ERROR_CODE CompVHoughKht::linking_AppendixA(CompVMatPtr& edges, CompVHoughKhtStrings& strings)
{
	uint8_t* edgesPtr = edges->ptr<uint8_t>(1);
	const size_t edgesWidth = edges->cols();
	const size_t edgesHeight = edges->rows();
	const size_t edgesStride = edges->strideInBytes();
	const int maxi = static_cast<int>(edges->cols() - 1);
	const int maxi64 = maxi - 8;
	const int maxj = static_cast<int>(edges->rows() - 1);
	int x_ref, y_ref; // both 'x_ref' and 'y_ref' start at 1

#define KHT_LINK_A5(x) if (edgesPtr[x]) linking_link_Algorithm5(&edgesPtr[x], edgesWidth, edgesHeight, edgesStride, strings, x, y_ref)

#if COMPV_INTRINSIC && COMPV_ARCH_X86 /* To avoid AVX/SSE transition issues (file not built with /AVX flag) do not add support for AVX (anyways, tested and not faster) */
	if (edges->cols() >= 16 && CompVCpu::isEnabled(kCpuFlagSSE2) && edges->isAlignedSSE()) {
		const int maxi128 = maxi - 15;

		__compv_builtin_prefetch_read(&edgesPtr[COMPV_CACHE1_LINE_SIZE * 0]);
		__compv_builtin_prefetch_read(&edgesPtr[COMPV_CACHE1_LINE_SIZE * 1]);
		__compv_builtin_prefetch_read(&edgesPtr[COMPV_CACHE1_LINE_SIZE * 2]);

		for (y_ref = 1; y_ref < maxj; ++y_ref) {
			if (_mm_movemask_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&edgesPtr[0]))) & 0xfffe) { // 0xfffe to ignore index #0
				/*KHT_LINK_A5(0);*/ KHT_LINK_A5(1); KHT_LINK_A5(2); KHT_LINK_A5(3);
				KHT_LINK_A5(4); KHT_LINK_A5(5); KHT_LINK_A5(6); KHT_LINK_A5(7);
				KHT_LINK_A5(8); KHT_LINK_A5(9); KHT_LINK_A5(10); KHT_LINK_A5(11);
				KHT_LINK_A5(12); KHT_LINK_A5(13); KHT_LINK_A5(14); KHT_LINK_A5(15);
			}
			for (x_ref = 16; x_ref < maxi128; x_ref += 16) {
				__compv_builtin_prefetch_read(&edgesPtr[x_ref + (COMPV_CACHE1_LINE_SIZE * 3)]);
				if (_mm_movemask_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&edgesPtr[x_ref])))) {
					KHT_LINK_A5(x_ref + 0); KHT_LINK_A5(x_ref + 1); KHT_LINK_A5(x_ref + 2); KHT_LINK_A5(x_ref + 3);
					KHT_LINK_A5(x_ref + 4); KHT_LINK_A5(x_ref + 5); KHT_LINK_A5(x_ref + 6); KHT_LINK_A5(x_ref + 7);
					KHT_LINK_A5(x_ref + 8); KHT_LINK_A5(x_ref + 9); KHT_LINK_A5(x_ref + 10); KHT_LINK_A5(x_ref + 11);
					KHT_LINK_A5(x_ref + 12); KHT_LINK_A5(x_ref + 13); KHT_LINK_A5(x_ref + 14); KHT_LINK_A5(x_ref + 15);
				}
			}
			__compv_builtin_prefetch_read(&edgesPtr[x_ref + (COMPV_CACHE1_LINE_SIZE * 3)]);
			for (; x_ref < maxi; ++x_ref) {
				KHT_LINK_A5(x_ref);
			}
			edgesPtr += edges->strideInBytes();
		}
		return COMPV_ERROR_CODE_S_OK;
	}
#endif /* COMPV_INTRINSIC && COMPV_ARCH_X86 */

#if COMPV_INTRINSIC && COMPV_ARCH_ARM
	if (edges->cols() >= 16 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && edges->isAlignedNEON()) {
		const int maxi128 = maxi - 15;
		uint8x16_t vec;
		
		edgesPtr = reinterpret_cast<uint8_t*>(__compv_builtin_assume_aligned(edgesPtr, 16));

		__compv_builtin_prefetch_read(&edgesPtr[COMPV_CACHE1_LINE_SIZE * 0]);
		__compv_builtin_prefetch_read(&edgesPtr[COMPV_CACHE1_LINE_SIZE * 1]);
		__compv_builtin_prefetch_read(&edgesPtr[COMPV_CACHE1_LINE_SIZE * 2]);

		for (y_ref = 1; y_ref < maxj; ++y_ref) {
			vec = vld1q_u8(&edgesPtr[0]); // read starting #0 for aligned
			if (COMPV_ARM_NEON_NEQ_ZERO(vec)) {
				/*KHT_LINK_A5(0);*/ KHT_LINK_A5(1); KHT_LINK_A5(2); KHT_LINK_A5(3);
				KHT_LINK_A5(4); KHT_LINK_A5(5); KHT_LINK_A5(6); KHT_LINK_A5(7);
				KHT_LINK_A5(8); KHT_LINK_A5(9); KHT_LINK_A5(10); KHT_LINK_A5(11);
				KHT_LINK_A5(12); KHT_LINK_A5(13); KHT_LINK_A5(14); KHT_LINK_A5(15);
			}
			for (x_ref = 16; x_ref < maxi128; x_ref += 16) {
				__compv_builtin_prefetch_read(&edgesPtr[x_ref + (COMPV_CACHE1_LINE_SIZE * 3)]);
				vec = vld1q_u8(&edgesPtr[x_ref]);
				if (COMPV_ARM_NEON_NEQ_ZERO(vec)) {
					KHT_LINK_A5(x_ref + 0); KHT_LINK_A5(x_ref + 1); KHT_LINK_A5(x_ref + 2); KHT_LINK_A5(x_ref + 3);
					KHT_LINK_A5(x_ref + 4); KHT_LINK_A5(x_ref + 5); KHT_LINK_A5(x_ref + 6); KHT_LINK_A5(x_ref + 7);
					KHT_LINK_A5(x_ref + 8); KHT_LINK_A5(x_ref + 9); KHT_LINK_A5(x_ref + 10); KHT_LINK_A5(x_ref + 11);
					KHT_LINK_A5(x_ref + 12); KHT_LINK_A5(x_ref + 13); KHT_LINK_A5(x_ref + 14); KHT_LINK_A5(x_ref + 15);
				}
			}
			__compv_builtin_prefetch_read(&edgesPtr[x_ref + (COMPV_CACHE1_LINE_SIZE * 3)]);
			for (; x_ref < maxi; ++x_ref) {
				KHT_LINK_A5(x_ref);
			}
			edgesPtr += edges->strideInBytes();
		}
		return COMPV_ERROR_CODE_S_OK;
	}
#endif

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	if (maxi64 > 8) {
		for (y_ref = 1; y_ref < maxj; ++y_ref) {
			// Align on #64 Bytes
			if (*reinterpret_cast<uint64_t*>(&edgesPtr[0]) << 8) { // "<<8" to ignore 'x_ref = 0' 
				KHT_LINK_A5(1); KHT_LINK_A5(2); KHT_LINK_A5(3); KHT_LINK_A5(4); KHT_LINK_A5(5); KHT_LINK_A5(6); KHT_LINK_A5(7);
			}
			for (x_ref = 8; x_ref < maxi64; x_ref += 8) {
				if (*reinterpret_cast<uint64_t*>(&edgesPtr[x_ref])) {
					KHT_LINK_A5(x_ref + 0); KHT_LINK_A5(x_ref + 1); KHT_LINK_A5(x_ref + 2); KHT_LINK_A5(x_ref + 3);
					KHT_LINK_A5(x_ref + 4); KHT_LINK_A5(x_ref + 5); KHT_LINK_A5(x_ref + 6); KHT_LINK_A5(x_ref + 7);
				}
			}
			for (; x_ref < maxi; ++x_ref) {
				KHT_LINK_A5(x_ref);
			}
			edgesPtr += edges->strideInBytes();
		}
	}
	else {
		for (y_ref = 1; y_ref < maxj; ++y_ref) {
			for (x_ref = 1; x_ref < maxi; ++x_ref) {
				KHT_LINK_A5(x_ref);
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Algorithm 6 - Function Next(). It complements the linking procedure(Algorithm 5).
#define __linking_next_Algorithm6() { \
	left_avail = x_seed > 0; \
	right_avail = (x_seed + 1) < edgesWidthInt; \
	/* == top == */ \
	if (y_seed > 0) { \
		top = next_seed - edgesStride; \
		if (left_avail && top[-1]) { /* top-left */ \
			--x_seed, --y_seed; next_seed = &top[-1]; continue; \
		} \
		else if (*top) { /* top-center */ \
			--y_seed; next_seed = top; continue; \
		} \
		else if (right_avail && top[1]) { /* top-right */ \
			++x_seed, --y_seed; next_seed = &top[1]; continue; \
		} \
	} \
	/* == center == */ \
	if (left_avail && next_seed[-1]) { /* center-left */ \
		--x_seed; --next_seed; continue; \
	} \
	else if (right_avail && next_seed[1]) { /* center-right */ \
		++x_seed; ++next_seed; continue; \
	} \
	/* == bottom == */ \
	else if ((y_seed + 1) < edgesHeightInt) { \
		bottom = next_seed + edgesStride; \
		if (left_avail && bottom[-1]) { /* bottom-left */ \
			--x_seed, ++y_seed;	next_seed = &bottom[-1]; continue; \
		} \
		else if (*bottom) { /* bottom-center */ \
			++y_seed; next_seed = bottom; continue; \
		} \
		else if (right_avail && bottom[1]) { /* bottom-right */ \
			++x_seed, ++y_seed; next_seed = &bottom[1]; continue; \
		} \
	} \
	next_seed = nullptr; break; \
}

// Algorithm 5 - Linking of neighboring edge pixels into strings
void CompVHoughKht::linking_link_Algorithm5(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, CompVHoughKhtStrings& strings, const int x_ref, const int y_ref)
{
	int x_seed, y_seed;
	const int edgesWidthInt = static_cast<int>(edgesWidth);
	const int edgesHeightInt = static_cast<int>(edgesHeight);
	uint8_t *next_seed, *top, *bottom;
	bool left_avail, right_avail;
    const KHT_TYP edgesWidthDiv2 = static_cast<KHT_TYP>(edgesWidth) * KHT_TYP_HALF;
    const KHT_TYP edgesHeightDiv2 = static_cast<KHT_TYP>(edgesHeight) * KHT_TYP_HALF;

	const size_t begin_size = m_poss.size();
    
    //!\\ Important: It's faster to compute 'cx' and 'cy' while setting 'x' and 'y'
    // than doing it later using a for_each loop

	// {Find and add feature pixels to the end of the string}
	x_seed = x_ref;
	y_seed = y_ref;
	next_seed = edgesPtr;
	do {
        m_poss.push_back(CompVHoughKhtPos(y_seed, x_seed, edgesHeightDiv2, edgesWidthDiv2));
		*next_seed = 0x00; // !! edges are modified here (not thread-safe) !!
		__linking_next_Algorithm6();
	} while(1);

	const size_t reverse_size = m_poss.size();

	// {Find and add feature pixels to the begin of the string}
	x_seed = x_ref;
	y_seed = y_ref;
	next_seed = edgesPtr;
	do {
		__linking_next_Algorithm6();
	} while (0);
	if (next_seed) {
		do {
            m_poss.push_back(CompVHoughKhtPos(y_seed, x_seed, edgesHeightDiv2, edgesWidthDiv2));
			*next_seed = 0x00; // !! edges are modified here (not thread-safe) !!
			__linking_next_Algorithm6();
		} while (1);
	}

	const size_t end_size = m_poss.size();
	
	if ((end_size - begin_size) >= m_cluster_min_size) {
        CompVHoughKhtPoss::iterator a = m_poss.begin() + begin_size;
		if (reverse_size > begin_size) {
			std::reverse(a, a + (reverse_size - begin_size));
		}
		strings.push_back(CompVHoughKhtString(begin_size, end_size));
	}
	else {
		m_poss.resize(begin_size);
	}
}

COMPV_ERROR_CODE CompVHoughKht::clusters_find(CompVHoughKhtClusters& clusters, CompVHoughKhtStrings::const_iterator strings_begin, CompVHoughKhtStrings::const_iterator strings_end)
{
	clusters.clear();
	for (CompVHoughKhtStrings::const_iterator it = strings_begin; it < strings_end; ++it) {
		clusters_subdivision(clusters, *it, 0, (it->end - it->begin) - 1);
	}
	return COMPV_ERROR_CODE_S_OK;
}

// (i) cluster approximately collinear feature pixels
KHT_TYP CompVHoughKht::clusters_subdivision(CompVHoughKhtClusters& clusters, const CompVHoughKhtString& string, const size_t start_index, const size_t end_index)
{
	// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5388&rep=rep1&type=pdf
	//  Three-Dimensional Object Recognition from Single Two - Dimensional Images
	// Section 4.6 - Segmentation of linked points into straight line segments

	const size_t num_clusters_without_subs = clusters.size();

    CompVHoughKhtPoss::const_iterator begin = m_poss.begin() + string.begin;
    CompVHoughKhtPoss::const_iterator start = begin + start_index;
	CompVHoughKhtPoss::const_iterator end = begin + end_index;
	CompVHoughKhtPoss::const_iterator current = start + 1;
	const int diffx = start->x - end->x;
	const int diffy = start->y - end->y;
	const KHT_TYP length = __compv_math_sqrt_fast(static_cast<KHT_TYP>((diffx * diffx) + (diffy * diffy)));

	// The significance of a straight line fit to a list of points can be estimated by calculating
	// 	the ratio of the length of the line segment divided by the maximum deviation of
	//	any point from the line(the maximum deviation is always assumed to be at least two
	//		pixels in size to account for limitations on measurement accuracy)
	
	size_t max_index = start_index, i = start_index + 1;
	int deviation, max_deviation = 0;
	for (; i < end_index; ++i, ++current) { // No need to include 'start_index' and 'end_index' -> max_deviation is equal to zero at theses indexes
		deviation = std::abs((((start->x - current->x) * diffy) - ((start->y - current->y) * diffx)));
		if (deviation > max_deviation) {
			max_index = i;
			max_deviation = deviation;
		}
	}	
	const KHT_TYP ratio = length / std::max((static_cast<KHT_TYP>(max_deviation) / length), m_cluster_min_deviation);

	// A segment is recursively subdivided at the
	// point with maximum deviation from a line connecting its endpoints(Figure 7 (b, c)).
	
	// This process is repeated until each segment is no more than 4 pixels in length, producing
	// a binary tree of possible subdivisions.

	//	If the maximum
	//	significance of any of the subsegments is greater than the significance of the complete
	//	segment, then the subsegments are returned.Otherwise the single segment is returned.

	const bool left_have_more_than_minsize = ((max_index - start_index + 1) >= m_cluster_min_size);
	if (left_have_more_than_minsize) {
		const bool right_have_more_than_minsize = ((end_index - max_index + 1) >= m_cluster_min_size);
		if (right_have_more_than_minsize) {
			const KHT_TYP ratio_left = clusters_subdivision(clusters, string, start_index, max_index);
			const KHT_TYP ratio_right = clusters_subdivision(clusters, string, max_index, end_index);
			if ((ratio_left > ratio) || (ratio_right > ratio)) {
				return (ratio_left > ratio_right) ? ratio_left : ratio_right;
			}
		}
	}
	
	// remove sub-clusters
	clusters.resize(num_clusters_without_subs);
	// push current cluster
	clusters.push_back(CompVHoughKhtCluster((string.begin + start_index), (string.begin + end_index + 1)));

	return ratio;
}

static COMPV_INLINE KHT_TYP __gauss_Eq15(const KHT_TYP rho, const KHT_TYP theta, const CompVHoughKhtKernel& kernel) {
#if 0 // https://github.com/DoubangoTelecom/compv/issues/137
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implemention found");
#endif
	const KHT_TYP sigma_rho_times_sigma_theta = __compv_math_sqrt_fast2(kernel.sigma_rho_square, kernel.sigma_theta_square); // sqrt(sigma_rho_square) * sqrt(sigma_theta_square)
	const KHT_TYP sigma_rho_times_sigma_theta_scale = KHT_TYP_ONE / sigma_rho_times_sigma_theta;
	const KHT_TYP r = (kernel.sigma_rho_times_theta * sigma_rho_times_sigma_theta_scale);
	const KHT_TYP one_minus_r_square = KHT_TYP_ONE - (r * r);
	const KHT_TYP x = KHT_TYP_ONE / (KHT_TYP_TWOPI * sigma_rho_times_sigma_theta * __compv_math_sqrt_fast(one_minus_r_square));
	const KHT_TYP y = KHT_TYP_ONE / (KHT_TYP_TWO * (one_minus_r_square));
	const KHT_TYP z = ((rho * rho) / kernel.sigma_rho_square) - (((r * KHT_TYP_TWO) * rho * theta) * sigma_rho_times_sigma_theta_scale) + ((theta * theta) / kernel.sigma_theta_square);
	return x * __compv_math_exp_fast_small(-z * y);
}

// Computes kernel's height (Gauss Eq15) and hmax
static COMPV_INLINE void CompVHoughKhtKernelHeight_1mpq_C(
	const KHT_TYP* M_Eq14_r0, const KHT_TYP* M_Eq14_0, const KHT_TYP* M_Eq14_2, const KHT_TYP* n_scale,
	KHT_TYP* sigma_rho_square, KHT_TYP* sigma_rho_times_theta, KHT_TYP* m2, KHT_TYP* sigma_theta_square,
	KHT_TYP* height, KHT_TYP* heightMax1, compv_uscalar_t count) 
{
	if (count > 3) { // otherwise SIMD will be used
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implemention found");
	}
	static const KHT_TYP KHT_TYP_ZERO_DOT_ONE = static_cast<KHT_TYP>(0.1);
	KHT_TYP r, r0, r1, r2, sigma_rho_times_sigma_theta, one_minus_r_square;
	for (compv_uscalar_t i = 0; i < count; ++i) {
		// No need to check if 'M_Eq14_r0' is equal to zero or not:
		// -> see http://www2.ic.uff.br/~laffernandes/projects/kht/ "Clarification - 01/14/2008"
		r0 = (KHT_TYP_ONE / M_Eq14_r0[i]);
		r1 = (M_Eq14_0[i] * r0);
		r2 = (M_Eq14_2[i] * r0);
		sigma_rho_square[i] = r1 * M_Eq14_0[i] + n_scale[i];
		sigma_rho_times_theta[i] = r1 * M_Eq14_2[i];
		m2[i] = r2 * M_Eq14_0[i];
		sigma_theta_square[i] = r2 * M_Eq14_2[i];
		// line 22
		if (sigma_theta_square[i] == 0.0) {
			sigma_theta_square[i] = KHT_TYP_ZERO_DOT_ONE; // (2^2 * 0.1)
		}
		sigma_rho_square[i] *= KHT_TYP_FOUR; // * (2^2)
		sigma_theta_square[i] *= KHT_TYP_FOUR; // * (2^2)
		sigma_rho_times_sigma_theta = __compv_math_sqrt_fast2(sigma_rho_square[i], sigma_theta_square[i]); // sqrt(sigma_rho_square) * sqrt(sigma_theta_square)
		r = (sigma_rho_times_theta[i] / sigma_rho_times_sigma_theta);
		one_minus_r_square = KHT_TYP_ONE - (r * r);
		height[i] = KHT_TYP_ONE / (KHT_TYP_TWOPI * sigma_rho_times_sigma_theta * __compv_math_sqrt_fast(one_minus_r_square));
		*heightMax1 = std::max(*heightMax1, height[i]);
	}
}

// Algorithm 2: Computation of the Gaussian kernel parameters
// Is thread-safe
COMPV_ERROR_CODE CompVHoughKht::voting_Algorithm2_Kernels(const CompVHoughKhtClusters& clusters, CompVHoughKhtKernels& kernels, KHT_TYP& hmax)
{
	hmax = 0.0;
	if (!clusters.empty()) {
		void(*CompVHoughKhtKernelHeight)(const KHT_TYP* M_Eq14_r0, const KHT_TYP* M_Eq14_0, const KHT_TYP* M_Eq14_2, const KHT_TYP* n_scale,
			KHT_TYP* sigma_rho_square, KHT_TYP* sigma_rho_times_theta, KHT_TYP* m2, KHT_TYP* sigma_theta_square,
			KHT_TYP* height, KHT_TYP* heightMax1, compv_uscalar_t count) = CompVHoughKhtKernelHeight_1mpq_C;
        CompVHoughKhtPoss::const_iterator posc, posb, pose; // position, current, begin and end
		size_t n;
		KHT_TYP mean_cx, mean_cy, cx, cy, cxx, cyy, cxy;
		KHT_TYP n_scale; // 1.0 / (number of pixels in Sk) = (1.0 / n)
		KHT_TYP ux, uy; // eigenvector in V for the biggest eigenvalue
		KHT_TYP vx, vy; // eigenvector in V for the smaller eigenvalue
		KHT_TYP sqrt_one_minus_vx2; // sqrt(1 - (vx^2)) - Eq14
		KHT_TYP matrix[2 * 2];
		KHT_TYP eigenVectors[2 * 2], eigenValues[2 * 2];
		KHT_TYP r0, r1;
		CompVMatPtr M_Eq15; // Gauss Eq15.0
		KHT_TYP *M_Eq14_r0, *M_Eq14_0, *M_Eq14_2, *M_Eq15_sigma_rho_square, *M_Eq15_sigma_rho_times_theta, *M_Eq15_sigma_theta_square, *M_Eq15_m2, *M_Eq15_height, *M_Eq15_n_scale;
		size_t M_Eq15_index;
		int M_Eq15_minpack = 1; // CompVHoughKhtKernelHeight_1mpq_C

		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<KHT_TYP>(&M_Eq15, 9, clusters.size()));
		M_Eq14_r0 = M_Eq15->ptr<KHT_TYP>(0);
		M_Eq14_0 = M_Eq15->ptr<KHT_TYP>(1);
		M_Eq14_2 = M_Eq15->ptr<KHT_TYP>(2);
		M_Eq15_sigma_rho_square = M_Eq15->ptr<KHT_TYP>(3);
		M_Eq15_sigma_rho_times_theta = M_Eq15->ptr<KHT_TYP>(4);
		M_Eq15_sigma_theta_square = M_Eq15->ptr<KHT_TYP>(5);
		M_Eq15_m2 = M_Eq15->ptr<KHT_TYP>(6);
		M_Eq15_height = M_Eq15->ptr<KHT_TYP>(7);
		M_Eq15_n_scale = M_Eq15->ptr<KHT_TYP>(8);

		kernels.resize(clusters.size());
		CompVHoughKhtKernels::iterator kernel = kernels.begin();

#if COMPV_ARCH_X86
		if (M_Eq15->cols() >= 2 && CompVCpu::isEnabled(kCpuFlagSSE2) && M_Eq15->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_2mpq_Intrin_SSE2, M_Eq15_minpack = 2));
			COMPV_EXEC_IFDEF_ASM_X64((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_2mpq_Asm_X64_SSE2, M_Eq15_minpack = 2));
		}
		if (M_Eq15->cols() >= 4 && CompVCpu::isEnabled(kCpuFlagAVX) && M_Eq15->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_4mpq_Intrin_AVX, M_Eq15_minpack = 4));
			COMPV_EXEC_IFDEF_ASM_X64((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_4mpq_Asm_X64_AVX, M_Eq15_minpack = 4));
			if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_4mpq_Asm_X64_FMA3_AVX, M_Eq15_minpack = 4));
			}
		}
#elif COMPV_ARCH_ARM
		if (M_Eq15->cols() >= 2 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && M_Eq15->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM64((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_2mpq_Intrin_NEON64, M_Eq15_minpack = 2));
            COMPV_EXEC_IFDEF_ASM_ARM64((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_4mpq_Asm_NEON64, M_Eq15_minpack = 2));
            COMPV_EXEC_IFDEF_ASM_ARM32((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_4mpq_Asm_NEON32, M_Eq15_minpack = 1));
            if (CompVCpu::isEnabled(kCpuFlagARM_VFPv4)) {
                COMPV_EXEC_IFDEF_ASM_ARM32((CompVHoughKhtKernelHeight = CompVHoughKhtKernelHeight_4mpq_Asm_VFPV4_NEON32, M_Eq15_minpack = 1));
            }
		}
#endif
		// for each group of pixels Sk
		M_Eq15_index = 0;
		for (CompVHoughKhtClusters::const_iterator cluster = clusters.begin(); cluster < clusters.end(); ++cluster, ++kernel, ++M_Eq15_index) {
			/* {Alternative reference system definition} */
			// computing the centroid
			mean_cx = mean_cy = 0;
			n = (cluster->end - cluster->begin);
			posb = m_poss.begin() + cluster->begin;
			pose = posb + n;
			n_scale = KHT_TYP_ONE / static_cast<KHT_TYP>(n);
			for (posc = posb; posc < pose; ++posc) {
				mean_cx += posc->cx;
				mean_cy += posc->cy;
			}
			mean_cx *= n_scale; // p_hat.x
			mean_cy *= n_scale; // p_hat.y

			/* {Eigen-decomposition} */
			cxx = cyy = cxy = 0.0;
			for (posc = posb; posc < pose; ++posc) {
				cx = (posc->cx - mean_cx); // (p.x - p_hat.x)
				cy = (posc->cy - mean_cy); // (p.y - p_hat.y)
				cxx += (cx * cx);
				cyy += (cy * cy);
				cxy += (cx * cy);
			}
			matrix[0] = cxx, matrix[1] = cxy, matrix[2] = cxy, matrix[3] = cyy;		
			COMPV_CHECK_CODE_RETURN(CompVMathEigen<KHT_TYP>::find2x2(matrix, eigenValues, eigenVectors)); // Eigen values and vectors are already sorted (highest to lowest)
			ux = eigenVectors[0], uy = eigenVectors[2]; // eigenvector in V for the biggest eigenvalue (first eigen vector)
			vx = eigenVectors[1], vy = eigenVectors[3]; // eigenvector in V for the smaller eigenvalue (second eigen vector)

			/* {yv < 0 condition verification} */
			if (vy < 0.0) {
				vx = -vx, vy = -vy;
			}

			/* {Normal equation parameters computation(Eq3)} */
			kernel->rho = (vx * mean_cx) + (vy * mean_cy);
			kernel->theta = std::acos(vx) * KHT_TYP_RAD_TO_DEG_SCALE;

			/* {substituting Eq5 in Eq10} */
			// Eq.14
			sqrt_one_minus_vx2 = __compv_math_sqrt_fast(KHT_TYP_ONE - (vx * vx));
			M_Eq14_0[M_Eq15_index] = -(ux * mean_cx) - (uy * mean_cy);
			M_Eq14_2[M_Eq15_index] = (sqrt_one_minus_vx2 == KHT_TYP_ZERO) ? KHT_TYP_ZERO : ((ux / sqrt_one_minus_vx2) * KHT_TYP_RAD_TO_DEG_SCALE);
			// Compute M
			r0 = 0.0;
			for (posc = posb; posc < pose; ++posc) {
				r1 = (ux * (posc->cx - mean_cx)) + (uy * (posc->cy - mean_cy));
				r0 += (r1 * r1);
			}

			M_Eq14_r0[M_Eq15_index] = r0;
			M_Eq15_n_scale[M_Eq15_index] = n_scale;
		} // end-of-for (CompVHoughKhtClusters::const_iterator cluster

		// Compute kernel's height and hmax
		const int cols = static_cast<int>(M_Eq15->cols());
		const int colsAlignedBackward = (cols & -M_Eq15_minpack);
		CompVHoughKhtKernelHeight(M_Eq14_r0, M_Eq14_0, M_Eq14_2, M_Eq15_n_scale,
			M_Eq15_sigma_rho_square, M_Eq15_sigma_rho_times_theta, M_Eq15_m2, M_Eq15_sigma_theta_square,
			M_Eq15_height, &hmax, static_cast<compv_uscalar_t>(colsAlignedBackward));
		if (M_Eq15_minpack > 1) {
			if (cols != colsAlignedBackward) {
				CompVHoughKhtKernelHeight_1mpq_C(&M_Eq14_r0[colsAlignedBackward], &M_Eq14_0[colsAlignedBackward], &M_Eq14_2[colsAlignedBackward], &M_Eq15_n_scale[colsAlignedBackward],
					&M_Eq15_sigma_rho_square[colsAlignedBackward], &M_Eq15_sigma_rho_times_theta[colsAlignedBackward], &M_Eq15_m2[colsAlignedBackward], &M_Eq15_sigma_theta_square[colsAlignedBackward],
					&M_Eq15_height[colsAlignedBackward], &hmax, static_cast<compv_uscalar_t>(cols - colsAlignedBackward));
			}
		}

		M_Eq15_index = 0;
		for (CompVHoughKhtKernels::iterator k = kernels.begin(); k < kernels.end(); ++k, ++M_Eq15_index) {
			k->sigma_rho_square = M_Eq15_sigma_rho_square[M_Eq15_index];
			k->sigma_rho_times_theta = M_Eq15_sigma_rho_times_theta[M_Eq15_index];
			k->m2 = M_Eq15_m2[M_Eq15_index];
			k->sigma_theta_square = M_Eq15_sigma_theta_square[M_Eq15_index];
			k->h = M_Eq15_height[M_Eq15_index];
		}

		/* Algorithm 3. Proposed voting process.The Vote() procedure is in Algorithm 4. */
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Is thread-safe
COMPV_ERROR_CODE CompVHoughKht::voting_Algorithm2_DiscardShortKernels(CompVHoughKhtKernels& kernels, const KHT_TYP hmax)
{
	/* {Discard groups with very short kernels} */
	if (!kernels.empty()) {
		const KHT_TYP hmax_scale = KHT_TYP_ONE / hmax;
		const KHT_TYP kernel_min_heigth = m_kernel_min_heigth;
		auto fncShortKernels = std::remove_if(kernels.begin(), kernels.end(), [kernel_min_heigth, hmax_scale](const CompVHoughKhtKernel& k) {
			return (k.h * hmax_scale) < kernel_min_heigth;
		});
		kernels.erase(fncShortKernels, kernels.end());
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Is thread-safe
COMPV_ERROR_CODE CompVHoughKht::voting_Algorithm2_Gmin(const CompVHoughKhtKernels& kernels, KHT_TYP &Gmin)
{
	Gmin = static_cast<KHT_TYP>(std::is_same<KHT_TYP, compv_float64_t>::value ? DBL_MAX : FLT_MAX); // r0 = Gmin // http://www2.ic.uff.br/~laffernandes/projects/kht/ -> Errata - 01/14/2008
	if (!kernels.empty()) {
		KHT_TYP r1, r2, eigenVectors[2 * 2], eigenValues[2 * 2];
		/* {Find the gmin threshold. Gk function in Eq15} */
		for (CompVHoughKhtKernels::const_iterator k = kernels.begin(); k < kernels.end(); ++k) {
			const KHT_TYP M[4] = { k->sigma_rho_square , k->sigma_rho_times_theta , k->m2 , k->sigma_theta_square };
			COMPV_CHECK_CODE_RETURN(CompVMathEigen<KHT_TYP>::find2x2(M, eigenValues, eigenVectors));
			// Compute Gk (gauss function)
			r1 = __compv_math_sqrt_fast(eigenValues[3]); // sqrt(smallest eigenvalue -> lambda_w)
			r2 = __gauss_Eq15(eigenVectors[1] * r1, eigenVectors[3] * r1, *k);
			if (r2 < Gmin) {
				Gmin = r2;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Not thread-safe
COMPV_ERROR_CODE CompVHoughKht::voting_Algorithm2_Count(int32_t* countsPtr, const size_t countsStride, CompVHoughKhtKernels::const_iterator kernels_begin, CompVHoughKhtKernels::const_iterator kernels_end, const KHT_TYP Gs)
{
	if (kernels_begin < kernels_end) {
		const KHT_TYP rho_scale = KHT_TYP_ONE / m_dRho;
		const KHT_TYP theta_scale = KHT_TYP_ONE / m_dTheta_deg;
		size_t rho_index, theta_index;
		const KHT_TYP rho_max_neg = *m_rho->ptr<const KHT_TYP>(0, 1);
		/* {Vote for each selected kernel} */
		for (CompVHoughKhtKernels::const_iterator k = kernels_begin; k < kernels_end; ++k) {
			// 3.2. Voting using a Gaussian distribution
			rho_index = COMPV_MATH_ROUNDFU_2_NEAREST_INT(std::abs((k->rho - rho_max_neg) * rho_scale), size_t) + 1;
			theta_index = COMPV_MATH_ROUNDFU_2_NEAREST_INT(std::abs(k->theta * theta_scale), size_t) + 1;
			// The four quadrants
			vote_Algorithm4(countsPtr, countsStride, rho_index, theta_index, 0.0, 0.0, 1, 1, Gs, *k);
			vote_Algorithm4(countsPtr, countsStride, rho_index, theta_index - 1, 0.0, -m_dTheta_deg, 1, -1, Gs, *k);
			vote_Algorithm4(countsPtr, countsStride, rho_index - 1, theta_index, -m_dRho, 0.0, -1, 1, Gs, *k);
			vote_Algorithm4(countsPtr, countsStride, rho_index - 1, theta_index - 1, -m_dRho, -m_dTheta_deg, -1, -1, Gs, *k);
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

void CompVHoughKht::vote_Algorithm4(int32_t* countsPtr, const size_t countsStride, size_t rho_start_index, const size_t theta_start_index, const KHT_TYP rho_start, const KHT_TYP theta_start, int inc_rho_index, const int inc_theta_index, const KHT_TYP scale, const CompVHoughKhtKernel& kernel)
{
	int32_t* pcount;
	const size_t rho_size = m_rho->cols(), theta_size = m_theta->cols();
	const KHT_TYP inc_rho = m_dRho * inc_rho_index;
	const KHT_TYP inc_theta = m_dTheta_deg * inc_theta_index;
	const KHT_TYP sigma_rho_square_scale = KHT_TYP_ONE / kernel.sigma_rho_square;
	const KHT_TYP sigma_theta_square_scale = KHT_TYP_ONE / kernel.sigma_theta_square;
	const KHT_TYP sigma_rho_times_sigma_theta = __compv_math_sqrt_fast2(kernel.sigma_rho_square, kernel.sigma_theta_square);
	const KHT_TYP sigma_rho_times_sigma_theta_scale = KHT_TYP_ONE / sigma_rho_times_sigma_theta;
	const KHT_TYP r = (kernel.sigma_rho_times_theta * sigma_rho_times_sigma_theta_scale);
	const KHT_TYP one_minus_r_square = KHT_TYP_ONE - (r * r);
	const KHT_TYP r_times_2 = r * KHT_TYP_TWO;
	const KHT_TYP x = KHT_TYP_ONE / (KHT_TYP_TWOPI * sigma_rho_times_sigma_theta * __compv_math_sqrt_fast(one_minus_r_square));
	const KHT_TYP y = KHT_TYP_ONE / (KHT_TYP_TWO * one_minus_r_square);

	KHT_TYP rho, theta;
	KHT_TYP k, krho, ki, z, w;
	int32_t votes;
	size_t rho_index, theta_index, theta_count;

	/* {Loop for the  coordinates of the parameter space} */
	theta_index = theta_start_index;
	theta = theta_start;
	theta_count = 0;
	do {
		/* {Test if the kernel exceeds the parameter space limits} */
		if (!theta_index || theta_index > theta_size) {
			rho_start_index = (rho_size - rho_start_index) + 1;
			theta_index = theta_index ? 1 : theta_size;
			inc_rho_index = -inc_rho_index;
		}

		if (rho_start_index >= 1) {
			/* {Loop for the  coordinates of the parameter space} */
			pcount = countsPtr + (theta_index * countsStride);
			rho_index = rho_start_index;
			rho = rho_start;
			w = ((theta * theta) * sigma_theta_square_scale);
			k = (r_times_2 * theta * sigma_rho_times_sigma_theta_scale);
			krho = (k * rho);
			ki = (k * inc_rho);
			z = ((rho * rho) * sigma_rho_square_scale) - (krho) + w;
			while (((rho_index <= rho_size) && (votes = COMPV_MATH_ROUNDFU_2_NEAREST_INT(((x * __compv_math_exp_fast_small(-z * y)) * scale), int32_t)) > 0)) {
				pcount[rho_index] += votes;
				rho_index += inc_rho_index;
				rho += inc_rho;
				krho += ki;
				z = ((rho * rho) * sigma_rho_square_scale) - (krho) + w;
			}
			theta_index += inc_theta_index;
			theta += inc_theta;
		}
		else break;
	} while ((rho != rho_start) && (++theta_count < theta_size));
}

// IS thread-safe
COMPV_ERROR_CODE CompVHoughKht::peaks_Section3_4_VotesCountAndClearVisitedMap(CompVHoughKhtVotes& votes, const size_t theta_index_start, const size_t theta_index_end)
{
	const size_t pcount_stride = m_count->stride();
	const int32_t *pcount = m_count->ptr<const int32_t>(theta_index_start);
	const size_t rho_count = m_rho->cols();
	size_t theta_index;
	int xmpd = 1;

	int32_t nThreshold = static_cast<int32_t>(m_nThreshold);

	void(*CompVHoughKhtPeaks_Section3_4_VotesCount)(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes)
		= CompVHoughKhtPeaks_Section3_4_VotesCount_C;

#if COMPV_ARCH_X86
	if (rho_count > 4 && CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVHoughKhtPeaks_Section3_4_VotesCount = CompVHoughKhtPeaks_Section3_4_VotesCount_4mpd_Intrin_SSE2, xmpd = 4));
	}
#	if 0 // TODO(dmi): SSE faster than AVX (too much memory read and few math)
	if (rho_count > 8 && CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVHoughKhtPeaks_Section3_4_VotesCount = CompVHoughKhtPeaks_Section3_4_VotesCount_8mpd_Intrin_AVX2, xmpd = 8));
	}
#	endif
#elif COMPV_ARCH_ARM
	if (rho_count > 4 && CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVHoughKhtPeaks_Section3_4_VotesCount = CompVHoughKhtPeaks_Section3_4_VotesCount_4mpd_Intrin_NEON, xmpd = 4));
	}
#endif

	// "1" not multiple of "2" and cannot used for backward align
	const size_t xmpd_consumed = ((xmpd == 1) ? rho_count : rho_count & (-xmpd)) + 1; // +1 because 'rho_index' starts at 1
	const size_t xmpd_remains = (rho_count > xmpd_consumed) ? (rho_count - xmpd_consumed) : 0;

	for (theta_index = theta_index_start; theta_index < theta_index_end; ++theta_index) {
		CompVHoughKhtPeaks_Section3_4_VotesCount(pcount, pcount_stride, theta_index, rho_count, nThreshold, votes);
		if (xmpd_remains) {
			CompVHoughKhtPeaks_Section3_4_VotesCount_C(&pcount[xmpd_consumed], pcount_stride, theta_index, xmpd_remains, nThreshold, votes);
		}
		COMPV_CHECK_CODE_RETURN(m_visited->zero_row(theta_index));
		pcount += pcount_stride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// NOT thread-safe
COMPV_ERROR_CODE CompVHoughKht::peaks_Section3_4_VotesSort(CompVHoughKhtVotes& votes)
{
	// Then, this list is sorted in descending
	//	order according to the result of the convolution of the cells with
	//	a 3 × 3 Gaussian kernel.
	std::sort(votes.begin(), votes.end(), [](const CompVHoughKhtVote &vote1, const CompVHoughKhtVote &vote2) {
		return (vote1.count > vote2.count);
	});
	return COMPV_ERROR_CODE_S_OK;
}

// NOT thread-safe
COMPV_ERROR_CODE CompVHoughKht::peaks_Section3_4_Lines(CompVHoughLineVector& lines, const CompVHoughKhtVotes& votes)
{
	bool bvisited;
	uint8_t* pvisited;
	const uint8_t *pvisited_top, *pvisited_bottom;
	const size_t pvisited_stride = m_visited->strideInBytes();
	const KHT_TYP *rho = m_rho->ptr<const KHT_TYP>();
	const KHT_TYP *theta = m_theta->ptr<const KHT_TYP>();
	CompVHoughKhtVotes::const_iterator votes_begin = votes.begin(), votes_end = votes.end();

	// After the sorting step, we use a sweep plane that visits each
	// cell of the list.By treating the parameter space as a heightfield
	// image, the sweeping plane gradually moves from each
	// peak toward the zero height.For each visited cell, we check if
	// any of its eight neighbors has already been visited.If so, the
	// current cell should be a smaller peak next to a taller one.In
	// such a case, we mark the current cell as visited and move the
	// sweeping plane to the next cell in the list.
	// Otherwise, we add
	// the current cell to the list of detected peaks, mark it as visited,
	// and then proceed with the sweeping plane scan to the next cell
	// in the list.The resulting group of detected peaks contains the
	// most significant lines identified in the image, already sorted by
	// number of votes.
	for (CompVHoughKhtVotes::const_iterator i = votes_begin; i < votes_end; ++i) {
		pvisited = m_visited->ptr<uint8_t>(i->theta_index, i->rho_index);
		pvisited_top = (pvisited - pvisited_stride);
		pvisited_bottom = (pvisited + pvisited_stride);
		bvisited = pvisited_top[-1] || pvisited_top[0] || pvisited_top[1] ||
			pvisited[-1] || pvisited[1] ||
			pvisited_bottom[-1] || pvisited_bottom[0] || pvisited_bottom[1];
		if (!bvisited) {
			lines.push_back(CompVHoughLine(
				static_cast<compv_float32_t>(rho[i->rho_index]),
				static_cast<compv_float32_t>(COMPV_MATH_DEGREE_TO_RADIAN(theta[i->theta_index])),
				i->count));
		}
		*pvisited = 0xff;
	}

	return COMPV_ERROR_CODE_S_OK;
}

static void CompVHoughKhtPeaks_Section3_4_VotesCount_C(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes)
{
	// Given a voting map, first we create a list with all cells that
	//	receive at least one vote.

	if (rho_count > 7) { // Otherwise, it's just minpack can't handle all data
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	}
	compv_uscalar_t rho_index;
	const int32_t *pcount_top, *pcount_bottom, *pcount_center;
	int32_t vote_count;
	// pcount.cols() have #2 more samples than rho_count which means no OutOfIndex issue for 'pcount_center[rho_index + 1]' (aka 'right') even for the last rho_index
	for (rho_index = 1; rho_index < rho_count; ++rho_index) {
		if (pcount[rho_index]) {
			pcount_center = &pcount[rho_index];
			pcount_top = (pcount_center - pcount_stride);
			pcount_bottom = (pcount_center + pcount_stride);
			vote_count = /* convolution of the cells with a 3 × 3 Gaussian kernel */
				pcount_top[-1] + (*pcount_top << 1) + pcount_top[1]
				+ pcount_bottom[-1] + ((*pcount_bottom) << 1) + pcount_bottom[1]
				+ (pcount_center[-1] << 1) + ((*pcount_center) << 2) + (pcount_center[1] << 1);
			if (vote_count >= nThreshold) { // do not add votes with less than threshold's values in count
				votes.push_back(CompVHoughKhtVote(rho_index, theta_index, vote_count));
			}
		}
	}
}

COMPV_NAMESPACE_END()
