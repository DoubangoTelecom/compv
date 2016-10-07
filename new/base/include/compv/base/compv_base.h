/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BASE_H_)
#define _COMPV_BASE_BASE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVBase : public CompVObj
{
protected:
	CompVBase();
public:
	virtual ~CompVBase();
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();

private:
	static bool s_bInitialized;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BASE_H_ */
