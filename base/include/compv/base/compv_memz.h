/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEMZ_H_)
#define _COMPV_BASE_MEMZ_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Helper class to avoid calling malloc + memset which is slooow
// https://vorpus.org/blog/why-does-calloc-exist/
template<class T>
class CompVMemZero : public CompVObj
{
protected:
	CompVMemZero(size_t rows, size_t cols, size_t stride = 0, bool useLegacyCalloc = false) : m_nCols(cols), m_nRows(rows) {
		m_bIsTbbMallocEnabled = !useLegacyCalloc && CompVMem::isTbbMallocEnabled();
#if 0 // MSER faster even if memset is called
		if (m_bIsTbbMallocEnabled) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Intel tbbMalloc calls memset(0) which is not good for optimization");
		}
#endif
		const size_t strideInBytes = (stride > cols) ? (stride * sizeof(T)) : CompVMem::alignForward(cols * sizeof(T), CompVMem::bestAlignment());
		m_nDataSize = ((strideInBytes * rows)) + CompVMem::bestAlignment();
		m_pMem = m_bIsTbbMallocEnabled
			? reinterpret_cast<uint8_t*>(CompVMem::callocAligned(m_nDataSize, sizeof(uint8_t), 8)) // use alignment equal to "8" to avoid calling malloc folowed by memset(0)
			: reinterpret_cast<uint8_t*>(::calloc(m_nDataSize, sizeof(uint8_t)));
		m_pPtr = reinterpret_cast<T*>(CompVMem::alignForward(reinterpret_cast<uintptr_t>(m_pMem)));
		m_nStride = strideInBytes / sizeof(T);
	}
public:
	virtual ~CompVMemZero() {
		if (m_pMem) {
			if (m_bIsTbbMallocEnabled) {
				CompVMem::freeAligned(reinterpret_cast<void**>(&m_pMem));
			}
			else {
				::free(m_pMem);
			}
		}
	}
	COMPV_OBJECT_GET_ID(CompVMemZero);
	COMPV_INLINE T* ptr(size_t row = 0, size_t col = 0) {
		return (m_pPtr + (row * m_nStride)) + col;
	}
	COMPV_INLINE size_t cols() {
		return m_nCols;
	}
	COMPV_INLINE size_t rows() {
		return m_nRows;
	}
	COMPV_INLINE size_t stride() {
		return m_nStride;
	}
	COMPV_INLINE size_t dataSize() {
		return m_nDataSize;
	}
	static COMPV_ERROR_CODE newObj(CompVPtr<CompVMemZero<T> *>* memz, size_t rows, size_t cols, size_t stride = 0, bool useLegacyCalloc = false) {
		COMPV_CHECK_EXP_RETURN(!memz || !rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVPtr<CompVMemZero<T> *> memz_ = new CompVMemZero<T>(rows, cols, stride, useLegacyCalloc);
		COMPV_CHECK_EXP_RETURN(!memz_, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_EXP_RETURN(!memz_->m_pPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		*memz = memz_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	T* m_pPtr;
	size_t m_nCols;
	size_t m_nRows;
	size_t m_nStride;
	size_t m_nDataSize;
	uint8_t* m_pMem;
	bool m_bIsTbbMallocEnabled;
};

typedef CompVMemZero<int32_t> CompVMemZero32s;
typedef CompVMemZero<int16_t> CompVMemZero16s;
typedef CompVMemZero<uint8_t> CompVMemZero8u;

typedef CompVPtr<CompVMemZero32s *> CompVMemZero32sPtr;
typedef CompVPtr<CompVMemZero16s *> CompVMemZero16sPtr;
typedef CompVPtr<CompVMemZero8u *> CompVMemZero8uPtr;

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MEMZ_H_ */
