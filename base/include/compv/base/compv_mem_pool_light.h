/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEM_POOL_LIGHT_H_)
#define _COMPV_BASE_MEM_POOL_LIGHT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_memz.h"
#include "compv/base/compv_debug.h"


COMPV_NAMESPACE_BEGIN()

//
// T must contains basic primitive types (https://en.wikipedia.org/wiki/Primitive_data_type) not requiring dtor/ctor(s)
//
template<class T>
class CompVMemPoolLightUnstructured : public CompVObj
{
protected:
	CompVMemPoolLightUnstructured(size_t nSizeInSamples)
	: m_nIx(0)
	, m_ptrMem(nullptr)
	, m_nSizeInSamples(nSizeInSamples)
	{

	}
public:
	COMPV_OBJECT_GET_ID(CompVMemPoolLightUnstructured);
	virtual ~CompVMemPoolLightUnstructured() {
		CompVMem::free(reinterpret_cast<void**>(&m_ptrMem));
	}
	COMPV_INLINE T* newItem() {
		return (m_nIx < m_nSizeInSamples)
			? &m_ptrMem[m_nIx++]
			: nullptr;
	}

	static COMPV_ERROR_CODE newObj(CompVPtr<CompVMemPoolLightUnstructured<T >* >* pool, size_t countInSamples, bool zeroMem = false) {
		COMPV_CHECK_EXP_RETURN(!pool || !countInSamples, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVPtr<CompVMemPoolLightUnstructured<T >* > pool_ = new CompVMemPoolLightUnstructured<T >(countInSamples);
		COMPV_CHECK_EXP_RETURN(!pool_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		pool_->m_ptrMem = reinterpret_cast<T*> 
			(
				zeroMem
				? CompVMem::calloc(countInSamples, sizeof(T))
				: CompVMem::malloc(countInSamples * sizeof(T))
			);
		COMPV_CHECK_EXP_RETURN(!pool_->m_ptrMem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		*pool = pool_;
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	T* m_ptrMem;
	size_t m_nIx;
	size_t m_nSizeInSamples;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MEM_POOL_LIGHT_H_ */
