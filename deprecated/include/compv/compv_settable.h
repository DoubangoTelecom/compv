/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_SETTABLE_H_)
#define _COMPV_SETTABLE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"

COMPV_NAMESPACE_BEGIN()

enum {
    COMPV_SETTABLE_DATA_TYPE_INT32,
    COMPV_SETTABLE_DATA_TYPE_OBJ,
};


class COMPV_API CompVSettable
{
protected:
    CompVSettable();

public:
    virtual ~CompVSettable();
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    virtual COMPV_ERROR_CODE get(int id, const void*& valuePtr, size_t valueSize);

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_SETTABLE_H_ */
