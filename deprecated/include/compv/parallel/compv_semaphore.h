/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PRALLEL_SEMAPHORE_H_)
#define _COMPV_PRALLEL_SEMAPHORE_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVSemaphore : public CompVObj
{
protected:
    CompVSemaphore(int initialVal = 0);
public:
    virtual ~CompVSemaphore();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVSemaphore";
    };

    COMPV_ERROR_CODE increment();
    COMPV_ERROR_CODE decrement();

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVSemaphore*>* sem, int initialVal = 0);

private:
    void* m_pHandle;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PRALLEL_SEMAPHORE_H_ */
