/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEM_INTRIN_AVX_H_)
#define _COMPV_BASE_MEM_INTRIN_AVX_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void MemCopy_Intrin_Aligned_AVX(COMPV_ALIGNED(AVX) void* dataDstPtr, COMPV_ALIGNED(AVX) const void* dataSrcPtr, compv_uscalar_t size);
void MemCopyNTA_Intrin_Aligned_AVX(COMPV_ALIGNED(AVX) void* dataDstPtr, COMPV_ALIGNED(AVX) const void* dataSrcPtr, compv_uscalar_t size);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MEM_INTRIN_AVX_H_ */
