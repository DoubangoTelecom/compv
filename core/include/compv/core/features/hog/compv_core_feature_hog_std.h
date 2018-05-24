/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_HOG_STD_H_)
#define _COMPV_CORE_FEATURES_HOG_STD_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/core/features/hog/compv_core_feature_hog_common_norm.h"
#include "compv/base/compv_memz.h"
#include "compv/base/compv_allocators.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

typedef compv_float32_t compv_hog_floattype_t;
typedef std::vector<compv_hog_floattype_t> compv_hog_vector_t;
typedef void(*CompVHogStdBuildMapHistForSingleCell_32f32s)(
	const compv_float32_t* magPtr,
	const compv_float32_t* dirPtr,
	compv_float32_t* mapHistPtr,
	const compv_float32_t* thetaMax1,
	const compv_float32_t* scaleBinWidth1,
	const int32_t* binWidth1,
	const int32_t* binIdxMax1,
	const compv_uscalar_t cellWidth,
	const compv_uscalar_t cellHeight,
	const compv_uscalar_t magStride,
	const compv_uscalar_t dirStride);

COMPV_OBJECT_DECLARE_PTRS(HogStd)

class CompVHogStd : public CompVHOG
{
protected:
	CompVHogStd(const CompVSizeSz& blockSize,
		const CompVSizeSz& blockStride,
		const CompVSizeSz& cellSize,
		const size_t nbins,
		const int blockNorm,
		const bool gradientSigned,
		const int interp);
public:
	virtual ~CompVHogStd();
	COMPV_OBJECT_GET_ID(CompVHogStd);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& input, CompVMatPtrPtr output) override /*Overrides(CompVHOG)*/;
	static COMPV_ERROR_CODE newObj(
		CompVHOGPtrPtr hog,
		const CompVSizeSz& blockSize = CompVSizeSz(16, 16),
		const CompVSizeSz& blockStride = CompVSizeSz(8, 8),
		const CompVSizeSz& cellSize = CompVSizeSz(8, 8),
		const size_t nbins = 9,
		const int blockNorm = COMPV_HOG_BLOCK_NORM_L2HYS,
		const bool gradientSigned = true,
		const int interp = COMPV_HOG_INTERPOLATION_BILINEAR);

private:
	COMPV_ERROR_CODE updateFptrBinning(const int& nInterp, const CompVSizeSz& szCellSize);
	static COMPV_ERROR_CODE buildOutputForSingleBlock(
		const compv_hog_floattype_t* mapHistPtr, compv_hog_floattype_t* outputPtr,
		const size_t& numCellsPerBlockY, const size_t& numBinsPerBlockX, const size_t& mapHistStride
	);

private:
	CompVSizeSz m_szBlockSize;
	CompVSizeSz m_szBlockStride;
	CompVSizeSz m_szCellSize;
	size_t m_nbins;
	int m_nBlockNorm;
	bool m_bGradientSigned;
	int m_nInterp;
	CompVHogStdBuildMapHistForSingleCell_32f32s m_fptrBinning;
	struct {
		void(*L1)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL1_32f_C;
		void(*L1_9)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL1_32f_C;
		void(*L1Sqrt)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL1Sqrt_32f_C;
		void(*L1Sqrt_9)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL1Sqrt_32f_C;
		void(*L2)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL2_32f_C;
		void(*L2_9)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL2_32f_C;
		void(*L2Hys)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL2Hys_32f_C;
		void(*L2Hys_9)(compv_hog_floattype_t* inOutPtr, const compv_hog_floattype_t* eps1, const compv_uscalar_t count) = CompVHogCommonNormL2Hys_32f_C;
	} fptrs_norm;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOG_STD_H_ */
