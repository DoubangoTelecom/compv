/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_parallel_asm_armv7.h"

#if COMPV_ASM && COMPV_ARCH_ARM32

#	if COMPV_ARCH_ARM32
	COMPV_EXTERNC void CompVSemaphoreDec_Asm_ARMv7_ARM32(void* semaphore);
	COMPV_EXTERNC void CompVSemaphoreInc_Asm_ARMv7_ARM32(void* semaphore);
	static void(*CompVSemaphoreDec_Asm_ARMv7)(void* semaphore) = CompVSemaphoreDec_Asm_ARMv7_ARM32;
	static void(*CompVSemaphoreInc_Asm_ARMv7)(void* semaphore) = CompVSemaphoreInc_Asm_ARMv7_ARM32;
#	endif /* COMPV_ARCH_ARM32 */

COMPV_NAMESPACE_BEGIN()

CompVSemaphoreAsmARMv7::CompVSemaphoreAsmARMv7()
	: CompVSemaphore()
	, m_pHandle(0)
{

}

CompVSemaphoreAsmARMv7::~CompVSemaphoreAsmARMv7()
{

}

COMPV_ERROR_CODE CompVSemaphoreAsmARMv7::init(int initialVal COMPV_DEFAULT(0)) /*override*/
{
	m_pHandle = static_cast<intptr_t>(initialVal);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSemaphoreAsmARMv7::increment() /*override*/
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("pthread is faster");
	CompVSemaphoreInc_Asm_ARMv7(&m_pHandle);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVSemaphoreAsmARMv7::decrement() /*override*/
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("pthread is faster");
	CompVSemaphoreDec_Asm_ARMv7(&m_pHandle);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* #if COMPV_ASM && COMPV_ARCH_ARM32 */
