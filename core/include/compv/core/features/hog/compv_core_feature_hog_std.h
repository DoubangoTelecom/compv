/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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
	const compv_uscalar_t dirStride,
	const void* bilinearFastLUT COMPV_DEFAULT(nullptr));

struct CompVHogStdBilinearLUTIdx {
	compv_float32_t diff;
	int32_t binIdx;
	int32_t binIdxNext;
};

struct CompVHogStdBilinearLUTData {
public:
	COMPV_INLINE const CompVHogStdBilinearLUTIdx* lut()const {
		return m_ptrLUT ? m_ptrLUT->ptr<const CompVHogStdBilinearLUTIdx>() : nullptr;
	}
	COMPV_ERROR_CODE update(const size_t& nbins, const int& interp, const bool& gradientSigned)
	{
		if (interp != COMPV_HOG_INTERPOLATION_BILINEAR_LUT) {
			m_nbins = 0;
			m_ptrLUT = nullptr;
			return COMPV_ERROR_CODE_S_OK;
		}
		if (m_bGradientSigned == gradientSigned && m_nbins == nbins && m_ptrLUT) {
			return COMPV_ERROR_CODE_S_OK;
		}
		const compv_hog_floattype_t thetaMax = gradientSigned ? 360.f : 180.f;
		const int32_t binWidth = static_cast<int32_t>(thetaMax / nbins);
		const compv_hog_floattype_t scaleBinWidth = 1.f / static_cast<compv_hog_floattype_t>(binWidth);
		const int32_t binIdxMax = static_cast<int32_t>(nbins - 1);
		
		const size_t lut_count = static_cast<size_t>((thetaMax + 1) * 10); // Resolution = 0.1 degree (*10), (+1) because we start at Zero
		CompVMatPtr lut;
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<CompVHogStdBilinearLUTIdx, COMPV_MAT_TYPE_STRUCT>(&lut, 1, lut_count)));
		CompVHogStdBilinearLUTIdx* lut_ptr = lut->ptr<CompVHogStdBilinearLUTIdx>();
		for (compv_hog_floattype_t theta = 0.f; theta <= thetaMax + 1; theta += 0.1f, ++lut_ptr) {
			const int32_t binIdx = static_cast<int32_t>((theta * scaleBinWidth) - 0.5f);
			const compv_float32_t diff = ((theta - (binIdx * binWidth)) * scaleBinWidth) - 0.5f;
			const int32_t binIdxNext = binIdx + ((diff >= 0) ? 1 : -1);
			
			lut_ptr->binIdx = binIdx;
			lut_ptr->binIdxNext = binIdxNext < 0 ? binIdxMax : (binIdxNext > binIdxMax ? 0 : binIdxNext);
			lut_ptr->diff = diff;
#			if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
			COMPV_ASSERT(lut_ptr->binIdx >= 0 && lut_ptr->binIdx <= binIdxMax);
			COMPV_ASSERT(lut_ptr->binIdxNext >= 0 && lut_ptr->binIdxNext <= binIdxMax);
#			endif
		}
		COMPV_ASSERT(lut_ptr == lut->ptr<CompVHogStdBilinearLUTIdx>() + lut_count);

		m_bGradientSigned = gradientSigned;
		m_nbins = nbins;
		m_ptrLUT = lut;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	bool m_bGradientSigned = false;
	size_t m_nbins = 0;
	CompVMatPtr m_ptrLUT = nullptr;
};

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
	COMPV_ERROR_CODE updateBilinearFastData(const size_t& nbins, const int& interp, const bool& gradientSigned);
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
	CompVHogStdBilinearLUTData m_BilinearLUT;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOG_STD_H_ */
