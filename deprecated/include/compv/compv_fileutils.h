/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FILEUTILS_H_)
#define _COMPV_FILEUTILS_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_buffer.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVFileUtils
{
private:
    CompVFileUtils();
public:
    virtual ~CompVFileUtils();
    static bool exists(const char* pcPath);
    static bool empty(const char* pcPath);
    static size_t getSize(const char* pcPath);
    static std::string getExt(const char* pcPath);
    static COMPV_IMAGE_FORMAT getImageFormat(const char* pcPath);
    static COMPV_ERROR_CODE read(const char* pcPath, CompVPtr<CompVBuffer*> *buffer);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FILEUTILS_H_ */
