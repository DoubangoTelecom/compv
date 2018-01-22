/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_SEMAPHORE_H_)
#define _COMPV_BASE_PRALLEL_SEMAPHORE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(Semaphore)
class COMPV_BASE_API CompVSemaphore : public CompVObj
{
protected:
    CompVSemaphore();
public:
    virtual ~CompVSemaphore();
	COMPV_OBJECT_GET_ID(CompVRunnable);

	virtual COMPV_ERROR_CODE init(int initialVal = 0);
	virtual COMPV_ERROR_CODE increment();
    virtual COMPV_ERROR_CODE decrement();

    static COMPV_ERROR_CODE newObj(CompVSemaphorePtrPtr sem, int initialVal = 0);

private:
    void* m_pHandle;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PRALLEL_SEMAPHORE_H_ */
