/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_ASM_ARMv7_H_)
#define _COMPV_BASE_PRALLEL_ASM_ARMv7_H_

#include "compv/base/parallel/compv_semaphore.h"

#if COMPV_ASM && COMPV_ARCH_ARM32

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(SemaphoreAsmARMv7)
class COMPV_BASE_API CompVSemaphoreAsmARMv7 : public CompVSemaphore
{
	friend class CompVSemaphore;
private:
	CompVSemaphoreAsmARMv7();
public:
	virtual ~CompVSemaphoreAsmARMv7();
	COMPV_OBJECT_GET_ID(CompVRunnable);

	virtual COMPV_ERROR_CODE init(int initialVal = 0) override;
	virtual COMPV_ERROR_CODE increment() override;
	virtual COMPV_ERROR_CODE decrement() override;

private:
	intptr_t m_pHandle;
};

COMPV_NAMESPACE_END()

#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#endif /* _COMPV_BASE_PRALLEL_ASM_ARMv7_H_ */

