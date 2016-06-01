/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_fileutils.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

#if defined(COMPV_OS_WINDOWS)
#include <Shlwapi.h>
#endif /* COMPV_OS_WINDOWS */

#include <algorithm> // std::transform(), ...
#include <sys/stat.h>

COMPV_NAMESPACE_BEGIN()

#define kModuleNameFileUtils "FileUtils"

CompVFileUtils::CompVFileUtils()
{

}

CompVFileUtils::~CompVFileUtils()
{

}

bool CompVFileUtils::exists(const char* pcPath)
{
    if (!pcPath) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Invalid parameter");
        return false;
    }
    struct stat st_;
    return (stat(pcPath, &st_) == 0);
}

bool CompVFileUtils::empty(const char* pcPath)
{
    return (CompVFileUtils::getSize(pcPath) == 0);
}

size_t CompVFileUtils::getSize(const char* pcPath)
{
    if (!pcPath) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Invalid parameter");
        return 0;
    }
    struct stat st_;
    if (stat(pcPath, &st_) != 0) {
        return 0;
    }
    return st_.st_size;
}

/*
Returns extension in uppercase format
*/
std::string CompVFileUtils::getExt(const char* pcPath)
{
    std::string ext_;
    if (!pcPath) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Invalid parameter");
    }
    else {
#if defined(COMPV_OS_WINDOWS)
        LPSTR strExt_ = PathFindExtensionA(pcPath);
        if (strExt_ && strlen(strExt_) > 1) {
            ext_ = std::string(strExt_ + 1);
        }
#else
        std::string path_ = std::string(pcPath);
        size_t index_ = path_.find_last_of(".");
        if (index_ != std::string::npos) {
            ext_ = path_.substr(index_ + 1);
        }
#endif
    }
    if (ext_.empty()) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Connot find extension for file path = %s", pcPath);
    }
    else {
        std::transform(ext_.begin(), ext_.end(), ext_.begin(), ::toupper); // UpperCase
    }
    return ext_;
}

_COMPV_IMAGE_FORMAT CompVFileUtils::getImageFormat(const char* pcPath)
{
    std::string ext_ = CompVFileUtils::getExt(pcPath); // UpperCase
    if (ext_ == "JPEG" || ext_ == "JPG" || ext_ == "JPE" || ext_ == "JFIF" || ext_ == "JIF") {
        return COMPV_IMAGE_FORMAT_JPEG;
    }
    else if (ext_ == "PNG") {
        return COMPV_IMAGE_FORMAT_PNG;
    }
    else if (ext_ == "BMP") {
        return COMPV_IMAGE_FORMAT_BMP;
    }
    else {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Connot image format from file path = %s", pcPath);
        return COMPV_IMAGE_FORMAT_NONE;
    }
}

COMPV_ERROR_CODE CompVFileUtils::read(const char* pcPath, CompVPtr<CompVBuffer*> *buffer)
{
    CompVPtr<CompVBuffer*> buffer_ = NULL;
    if (!pcPath || !buffer) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Invalid parameter");
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    int32_t size_ = (int32_t)CompVFileUtils::getSize(pcPath);
    if (size_ > 0) {
        FILE* file_ = NULL;
        void* mem_ = NULL;
        if ((file_ = fopen(pcPath, "rb")) == NULL) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Can't open %s", pcPath);
            return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
        }
        mem_ = CompVMem::malloc(size_);
        if (!mem_) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Failed to alloc mem with size = %u", (unsigned)size_);
            fclose(file_);
            return COMPV_ERROR_CODE_E_OUT_OF_MEMORY;
        }
        size_t read_;
        if (size_ != (read_ = fread(mem_, 1, size_, file_))) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "fread(%s) returned %lu instead of %u", pcPath, read_, (unsigned)size_);
            fclose(file_);
            CompVMem::free(&mem_);
            return COMPV_ERROR_CODE_E_FAILED_TO_READ_FILE;
        }
        if (COMPV_ERROR_CODE_IS_NOK(CompVBuffer::newObjAndTakeData(&mem_, size_, &buffer_))) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Failed to create new CompVBuffer object");
        }
        fclose(file_);
        CompVMem::free(&mem_);
    }
    *buffer = buffer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
