/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_FILEUTILS_H_)
#define _COMPV_BASE_FILEUTILS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_buffer.h"
#include "compv/base/compv_common.h"

#define COMPV_PATH_FROM_NAME(name) (CompVFileUtils::getFullPathFromFileName((name)).c_str())

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVFileUtils
{
private:
    CompVFileUtils();
public:
    virtual ~CompVFileUtils();
    static std::string getCurrentDirectory();
    static std::string getFullPathFromFileName(const char* filename);
	static std::string getFileNameFromFullPath(const char* fullpath);
	static std::string getFolderPathFromFullPath(const char* fullpath);
	static std::string patchFullPath(const char* fullpath);
    static bool exists(const char* pcPath);
    static bool empty(const char* pcPath);
    static size_t getSize(const char* pcPath);
    static std::string getExt(const char* pcPath);
    static COMPV_IMAGE_FORMAT getImageFormat(const char* pcPath);
    static COMPV_ERROR_CODE read(const char* pcPath, CompVBufferPtrPtr buffer, bool quiet = false);
    static FILE* open(const char* fname, const char* mode);
	static COMPV_ERROR_CODE close(FILE** file);
	static COMPV_ERROR_CODE write(const char* pcPath, const void* data, size_t count);
	static COMPV_ERROR_CODE getFilesInDir(const char* dir, std::vector<std::string>& paths);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_FILEUTILS_H_ */
