/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_KERNEL_H_)
#define _COMPV_BASE_KERNEL_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVKernel
{
public:
	static COMPV_ERROR_CODE mean(const size_t& blockSize, CompVMatPtrPtr kernel);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_KERNEL_H_ */
