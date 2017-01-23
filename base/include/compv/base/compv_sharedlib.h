/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_SHAREDLIB_H_)
#define _COMPV_BASE_SHAREDLIB_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(SharedLib)

class COMPV_BASE_API CompVSharedLib : public CompVObj
{
protected:
    CompVSharedLib(void* pHandle = NULL);
public:
    virtual ~CompVSharedLib();
    COMPV_OBJECT_GET_ID(CompVSharedLib);
    COMPV_INLINE void* handle()const {
        return m_pHandle;
    }

    void* sym(const char* name);

    static COMPV_ERROR_CODE open(const char* filePath, void** handle);
    static COMPV_ERROR_CODE close(void* handle);
    static void* sym(void* handle, const char* name);

    static COMPV_ERROR_CODE newObj(CompVSharedLibPtrPtr sharedlib, const char* filePath);
    static COMPV_ERROR_CODE newObj(CompVSharedLibPtrPtr sharedlib, void* handle);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    void* m_pHandle;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_SHAREDLIB_H_ */
