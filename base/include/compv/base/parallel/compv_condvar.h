/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PRALLEL_CONDVAR_H_)
#define _COMPV_BASE_PRALLEL_CONDVAR_H_

#include "compv/base/compv_config.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVCondvar : public CompVObj
{
protected:
    CompVCondvar();
public:
    virtual ~CompVCondvar();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVCondvar";
    };

    COMPV_ERROR_CODE wait(uint64_t millis = 0);
    COMPV_ERROR_CODE signal();
    COMPV_ERROR_CODE broadcast();

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVCondvar*>* condvar);

private:
    COMPV_ERROR_CODE waitWithoutTimeout();
    COMPV_ERROR_CODE waitWithTimeout(uint64_t millis);

private:
    void* m_pHandle;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PRALLEL_CONDVAR_H_ */

