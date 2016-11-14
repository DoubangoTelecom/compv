/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MAT_H_)
#define _COMPV_BASE_MAT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_buffer.h"
#include "compv/base/compv_mem.h"
#include "compv/base/image/compv_image_utils.h"

COMPV_NAMESPACE_BEGIN()
COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Warray-bounds")

class CompVMat;
typedef CompVPtr<CompVMat* > CompVMatPtr;
typedef CompVMatPtr* CompVMatPtrPtr;
 
#if !defined(COMPV_MAT_MAX_COMP_COUNT)
#	define COMPV_MAT_MAX_COMP_COUNT		4
#endif /* COMPV_MAT_MAX_COMP_COUNT */

class COMPV_BASE_API CompVMat : public CompVObj
{
protected:
	CompVMat();
public:
	virtual ~CompVMat();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVMat";
	};

	COMPV_INLINE COMPV_MAT_TYPE type() { 
		return m_eType;
	}

	COMPV_INLINE COMPV_MAT_SUBTYPE subType() {
		return m_eSubType;
	}

	COMPV_INLINE bool isEmpty() {
		return !ptr() || !rows() || !cols();
	}

	COMPV_INLINE size_t compCount() { return static_cast<size_t>(m_nCompCount); }

	template<class ptrType = void>
	COMPV_INLINE const ptrType* ptr(size_t row = 0, size_t col = 0, int32_t compId = 0)const {
		if (compId >= 0 && compId < m_nCompCount) {
			return (row > m_nCompRows[compId] || col > m_nCompCols[compId]) ? NULL : static_cast<const ptrType*>(static_cast<const uint8_t*>(m_pCompPtr[compId]) + (row * m_nCompStrideInBytes[compId]) + (col * m_nElmtInBytes));
		}
		COMPV_DEBUG_ERROR("Invalid parameter");
		return NULL;
	}
	
	COMPV_INLINE size_t rows(int32_t compId = -1)const { // In samples
		if (compId < 0) {
			return m_nRows;
		}
		else if (compId < m_nCompCount) {
			return m_nCompRows[compId];    
		}
		COMPV_DEBUG_ERROR("Invalid parameter");
		return 0;
	}
	
	COMPV_INLINE size_t cols(int32_t compId = -1)const { // In samples
		if (compId < 0) {
			return m_nCols;
		}
		else if (compId < m_nCompCount) {
			return m_nCompCols[compId]; 
		}
		COMPV_DEBUG_ERROR("Invalid parameter");
		return 0;
	}

	COMPV_INLINE size_t elmtInBytes() {
		return m_nElmtInBytes;
	}
	
	COMPV_INLINE size_t rowInBytes(int32_t compId = -1)const { // in bytes
		if (compId < 0) {
			return m_nCols * m_nElmtInBytes;
		}
		else if (compId < m_nCompCount) {
			return m_nCompCols[compId] * m_nElmtInBytes;
		}
		COMPV_DEBUG_ERROR("Invalid parameter");
		return 0;
	}
	
	COMPV_INLINE size_t strideInBytes(int32_t compId = -1)const { // in bytes
		if (compId < 0) {
			return m_nStrideInBytes;
		}
		else if (compId < m_nCompCount) {
			return m_nCompStrideInBytes[compId];
		}
		COMPV_DEBUG_ERROR("Invalid parameter");
		return 0;  
	}
	
	COMPV_INLINE size_t stride(int32_t compId = -1)const { // in samples
		if (compId < 0) {
			return m_nStrideInElts;
		}
		else if (compId < m_nCompCount) {
			return m_nCompStrideInElts[compId];
		}
		COMPV_DEBUG_ERROR("Invalid parameter");
		return 0;
	}

	template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_MAT_SUBTYPE dataSubType = COMPV_MAT_SUBTYPE_RAW>
	static COMPV_ERROR_CODE newObj(CompVMatPtrPtr mat, size_t rows, size_t cols, size_t alignv, size_t stride = 0)
	{
		COMPV_CHECK_EXP_RETURN(!mat || !alignv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMatPtr mat_ = *mat;
		if (!mat_) {
			mat_ = new CompVMat();
			COMPV_CHECK_EXP_RETURN(!mat_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			*mat = mat_;
		}
		COMPV_CHECK_CODE_RETURN((mat_->alloc<elmType, dataType, dataSubType>(rows, cols, alignv, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

	template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_MAT_SUBTYPE dataSubType = COMPV_MAT_SUBTYPE_RAW>
	static COMPV_ERROR_CODE newObjStrideless(CompVMatPtrPtr mat, size_t rows, size_t cols)
	{
		return CompVMat::newObj<elmType, dataType, dataSubType>(mat, rows, cols, 1);
	}

	template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_MAT_SUBTYPE dataSubType = COMPV_MAT_SUBTYPE_RAW>
	static COMPV_ERROR_CODE newObjAligned(CompVMatPtrPtr mat, size_t rows, size_t cols)
	{
		return CompVMat::newObj<elmType, dataType, dataSubType>(mat, rows, cols, COMPV_SIMD_ALIGNV_DEFAULT);
	}

protected:
	template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_MAT_SUBTYPE dataSubType = COMPV_MAT_SUBTYPE_RAW>
	COMPV_ERROR_CODE alloc(size_t rows, size_t cols, size_t alignv = 1, size_t stride = 0)
	{
		COMPV_CHECK_EXP_RETURN(!alignv || (stride && stride < cols), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

		size_t nNewDataSize;
		size_t nElmtInBytes;
		bool bCompInterleaved;
		size_t nCompCount;
		size_t nBestStrideInBytes;
		size_t nCompStrideInBytes[COMPV_MAT_MAX_COMP_COUNT];
		size_t nCompCols[COMPV_MAT_MAX_COMP_COUNT];
		size_t nCompRows[COMPV_MAT_MAX_COMP_COUNT];
		size_t nCompSizeInBytes[COMPV_MAT_MAX_COMP_COUNT];
		void* pMem = NULL;

		nElmtInBytes = sizeof(elmType);
		nBestStrideInBytes = stride ? (stride * nElmtInBytes) : static_cast<size_t>(CompVMem::alignForward((cols * nElmtInBytes), (int)alignv));

		if (dataType == COMPV_MAT_TYPE_RAW) {
			nCompCount = 1;
			bCompInterleaved = false;
			nNewDataSize = nBestStrideInBytes * rows;
			nCompCols[0] = cols;
			nCompRows[0] = rows;
			nCompStrideInBytes[0] = nBestStrideInBytes;
			nCompSizeInBytes[0] = nNewDataSize;
		}
		else if (dataType == COMPV_MAT_TYPE_PIXELS) {
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::getCompCount(static_cast<COMPV_PIXEL_FORMAT>(dataSubType), &nCompCount)); // get number of comps
			COMPV_CHECK_EXP_RETURN(!nCompCount || nCompCount > COMPV_MAT_MAX_COMP_COUNT, COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT); // check the number of comps
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::getCompInterleaved(static_cast<COMPV_PIXEL_FORMAT>(dataSubType), &bCompInterleaved)); // check whether the comps are interleaved
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::getSizeForPixelFormat(static_cast<COMPV_PIXEL_FORMAT>(dataSubType), nBestStrideInBytes, rows, &nNewDataSize)); // get overal size in bytes
			size_t nSumCompSizeInBytes = 0;
			for (size_t compId = 0; compId < nCompCount; ++compId) {
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::getCompSizeForPixelFormat(static_cast<COMPV_PIXEL_FORMAT>(dataSubType), compId, cols, rows, &nCompCols[compId], &nCompRows[compId])); // get 'cols' and 'rows' for the comp
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::getCompSizeForPixelFormat(static_cast<COMPV_PIXEL_FORMAT>(dataSubType), compId, nBestStrideInBytes, rows, &nCompStrideInBytes[compId], &nCompRows[compId])); // get 'stride' and 'rows' for the comp
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::getCompSizeForPixelFormat(static_cast<COMPV_PIXEL_FORMAT>(dataSubType), nCompStrideInBytes[compId], rows, compId, &nCompSizeInBytes[compId])); // get comp size in bytes
				nSumCompSizeInBytes += nCompSizeInBytes[compId];
			}
			// Make sure that sum(comp sizes) = data size
			COMPV_CHECK_EXP_BAIL(nSumCompSizeInBytes != nNewDataSize, COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		}
		else {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		}
		
		if (nNewDataSize > m_nDataCapacity) {
			pMem = CompVMem::malloc(nNewDataSize);
			COMPV_CHECK_EXP_BAIL(!pMem, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
			m_nDataCapacity = nNewDataSize;
			// realloc()
			if (m_bOweMem) {
				CompVMem::free((void**)&m_pDataPtr);
			}
			m_pDataPtr = pMem;
			m_bOweMem = true;
		}

		//!\\ Do not update capacity unless shorter (see above)

		m_eType = dataType;
		m_eSubType = dataSubType;
		m_nCols = cols;
		m_nRows = rows;
		m_nStrideInBytes = nBestStrideInBytes;
		m_nStrideInElts = (nBestStrideInBytes / nElmtInBytes);
		m_nDataSize = nNewDataSize;
		m_nAlignV = alignv;
		m_nElmtInBytes = nElmtInBytes;
		m_bCompInterleaved = bCompInterleaved;
		m_nCompCount = static_cast<int32_t>(nCompCount);
		m_pCompPtr[0] = m_pDataPtr;
		m_nCompSizeInBytes[0] = nCompSizeInBytes[0];
		m_nCompCols[0] = nCompCols[0];
		m_nCompRows[0] = nCompRows[0];
		m_nCompStrideInBytes[0] = nCompStrideInBytes[0];
		m_nCompStrideInElts[0] = (nCompStrideInBytes[0] / nElmtInBytes);
		m_bCompStrideInEltsIsIntegral[0] = !(nCompStrideInBytes[0] % nElmtInBytes);
		for (size_t compId = 1; compId < nCompCount; ++compId) {
			if (bCompInterleaved) {
				m_pCompPtr[compId] = m_pCompPtr[0]; // use byte-0
			}
			else {
				m_pCompPtr[compId] = static_cast<const uint8_t*>(m_pCompPtr[compId - 1]) + nCompSizeInBytes[compId - 1];
			}
			m_nCompSizeInBytes[compId] = nCompSizeInBytes[compId];
			m_nCompCols[compId] = nCompCols[compId];
			m_nCompRows[compId] = nCompRows[compId];
			m_nCompStrideInBytes[compId] = nCompStrideInBytes[compId];
			m_nCompStrideInElts[compId] = (nCompStrideInBytes[compId] / nElmtInBytes);
			m_bCompStrideInEltsIsIntegral[compId] = !(nCompStrideInBytes[compId] % nElmtInBytes);
		}
		
	bail:
		if (COMPV_ERROR_CODE_IS_NOK(err_)) {
			CompVMem::free((void**)&pMem);
		}
		return err_;
	}

private:
	void* m_pDataPtr;
	int32_t m_nCompCount;
	bool m_bCompInterleaved;
	size_t m_nCols;
	size_t m_nRows;
	size_t m_nStrideInBytes;
	size_t m_nStrideInElts;
	size_t m_nCompCols[COMPV_MAT_MAX_COMP_COUNT];
	size_t m_nCompRows[COMPV_MAT_MAX_COMP_COUNT];
	const void* m_pCompPtr[COMPV_MAT_MAX_COMP_COUNT];
	size_t m_nCompSizeInBytes[COMPV_MAT_MAX_COMP_COUNT];
	size_t m_nCompStrideInBytes[COMPV_MAT_MAX_COMP_COUNT];
	size_t m_nCompStrideInElts[COMPV_MAT_MAX_COMP_COUNT];
	bool m_bCompStrideInEltsIsIntegral[COMPV_MAT_MAX_COMP_COUNT];
	size_t m_nElmtInBytes;
	size_t m_nAlignV;
	size_t m_nDataSize;
	size_t m_nDataCapacity;
	bool m_bOweMem;
	COMPV_MAT_TYPE m_eType;
	COMPV_MAT_SUBTYPE m_eSubType;
};

COMPV_GCC_DISABLE_WARNINGS_END()
COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MAT_H_ */
