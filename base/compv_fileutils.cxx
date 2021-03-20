/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_fileutils.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"
#include "compv/base/android/compv_android_fileutils.h"
#include "compv/base/compv_errno.h"

#if COMPV_OS_WINDOWS
#	include <Shlwapi.h>
#	include <direct.h> // _getcwd
#	define getcwd _getcwd
#	define COMPV_MAX_PATH	MAX_PATH
#else
#	include <unistd.h> // getcwd
#	define COMPV_MAX_PATH	PATH_MAX
#endif /* COMPV_OS_WINDOWS */

#if COMPV_OS_IPHONE
#   import <Foundation/Foundation.h>
#endif /* COMPV_OS_IPHONE */

#if COMPV_OS_ANDROID
AAssetManager* __compv_android_assetmgr = nullptr; // defined as extern in "compv/base/android/compv_android_fileutils.h"
#endif /* COMPV_OS_ANDROID */

#include <algorithm> // std::transform(), ...
#include <sys/stat.h>
#include <dirent.h>

COMPV_NAMESPACE_BEGIN()

#define kModuleNameFileUtils "FileUtils"

CompVFileUtils::CompVFileUtils()
{

}

CompVFileUtils::~CompVFileUtils()
{

}

std::string CompVFileUtils::getCurrentDirectory()
{
    char path[COMPV_MAX_PATH] = { '\0' };
    char* ret = getcwd(path, sizeof(path));
    if (!ret) {
        COMPV_DEBUG_ERROR("getcwd failed");
        return std::string(".");
    }
    return std::string(ret);
}

std::string CompVFileUtils::getFullPathFromFileName(const char* filename)
{
    if (!filename) {
        COMPV_DEBUG_ERROR("Invalid parameter");
        return std::string("");
    }

#if COMPV_OS_WINDOWS
#	if defined(_MSC_VER)
    if (IsDebuggerPresent()) {
        DWORD length;
        char dir[COMPV_MAX_PATH] = { '\0' };
        if ((length = GetModuleFileNameA(NULL, dir, sizeof(dir)))) {
            if (!PathRemoveFileSpecA(dir)) {
                COMPV_DEBUG_ERROR("PathRemoveFileSpecA(%s) failed: %d", dir, GetLastError());
                return std::string("");
            }
            else {
                char path[COMPV_MAX_PATH] = { '\0' };
                char* ret = PathCombineA(path, dir, filename);
                if (!ret) {
                    COMPV_DEBUG_ERROR("PathCombineA failed");
                    return std::string("");
                }
                return std::string(ret);
            }
        }
        else {
            COMPV_DEBUG_ERROR("GetModuleFileNameA failed: %d", GetLastError());
            return std::string("");
        }
    }
#	endif /* _MSC_VER */
    std::string currDir = CompVFileUtils::getCurrentDirectory();
    char path[COMPV_MAX_PATH];
    char* ret = PathCombineA(path, currDir.c_str(), filename);
    if (!ret) {
        COMPV_DEBUG_ERROR("PathCombineA failed");
        return currDir + "/" + std::string(filename);
    }
    return std::string(ret);
#else
#	if COMPV_OS_ANDROID
    if (compv_android_have_assetmgr()) {
        return std::string(filename);
    }
#   endif /* COMPV_OS_ANDROID */
#	if COMPV_OS_IPHONE
    NSString* nsPath = [NSString stringWithUTF8String:filename];
    NSString* nsPathWithoutExt = [nsPath stringByDeletingPathExtension];
    NSString* nsPathExt = [nsPath pathExtension];
    NSString* nsResource = [[NSBundle mainBundle] pathForResource:nsPathWithoutExt ofType:nsPathExt];
    return nsResource ? std::string([nsResource cStringUsingEncoding:NSASCIIStringEncoding]) : std::string(filename); // using bundle
#   else
    return CompVFileUtils::getCurrentDirectory() + "/" + std::string(filename);
#   endif /* !COMPV_OS_IPHONE */
#endif
}

std::string CompVFileUtils::getFileNameFromFullPath(const char* fullpath)
{
	COMPV_ASSERT(fullpath != nullptr);
	std::string fullpath_(fullpath);
	std::replace(fullpath_.begin(), fullpath_.end(), '\\', '/');
	const size_t i = fullpath_.rfind('/', fullpath_.length());
	if (i != std::string::npos) {
		return (fullpath_.substr(i + 1, fullpath_.length() - i));
	}
	return fullpath_;
}

std::string CompVFileUtils::getFolderPathFromFullPath(const char* fullpath)
{
	COMPV_ASSERT(fullpath != nullptr);
	std::string fullpath_(fullpath);
	std::replace(fullpath_.begin(), fullpath_.end(), '\\', '/');
	const size_t i = fullpath_.rfind('/', fullpath_.length());
	if (i != std::string::npos) {
		return (fullpath_.substr(0, i));
	}
	return fullpath_;
}

std::string CompVFileUtils::patchFullPath(const char* fullpath)
{
	COMPV_ASSERT(fullpath != nullptr);
	
	if (CompVFileUtils::exists(fullpath)) {
		return fullpath;
	}

	std::string fileName_ = CompVFileUtils::getFileNameFromFullPath(fullpath);
	if (fileName_.empty()) {
		return fullpath;
	}
	return CompVFileUtils::getFullPathFromFileName(fileName_.c_str()); // Get real full path (Android -> assets, iOS -> Bundle)
}

bool CompVFileUtils::exists(const char* pcPath)
{
    if (!pcPath) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Invalid parameter");
        return false;
    }
#if COMPV_OS_ANDROID
    if (compv_android_have_assetmgr()) {
        return compv_android_asset_fexist(pcPath);
    }
    COMPV_DEBUG_INFO_CODE_ONCE("Not using asset manager");
#endif /* COMPV_OS_ANDROID */
#if COMPV_OS_IPHONE
    return [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithUTF8String:pcPath]];
#else
    struct stat st_;
    return (stat(pcPath, &st_) == 0);
#endif /* !COMPV_OS_IPHONE */
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
    
#if COMPV_OS_ANDROID
    if (compv_android_have_assetmgr()) {
        return compv_android_asset_fsize(pcPath);
    }
    COMPV_DEBUG_INFO_CODE_ONCE("Not using asset manager");
#endif /* COMPV_OS_ANDROID */
    
#if COMPV_OS_IPHONE
    NSData* nsData = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String:pcPath]];
    return nsData ? static_cast<size_t>(nsData.length) : 0;
#else
    struct stat st_;
    if (stat(pcPath, &st_) != 0) {
        return 0;
    }
    return static_cast<size_t>(st_.st_size);
#endif /* !COMPV_OS_IPHONE */
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
#elif COMPV_OS_IPHONE
        NSString* nsPathExt = [[NSString stringWithUTF8String:pcPath] pathExtension];
        return std::string([nsPathExt cStringUsingEncoding:NSASCIIStringEncoding]);
#else
        std::string path_ = std::string(pcPath);
        size_t index_ = path_.find_last_of(".");
        if (index_ != std::string::npos) {
            ext_ = path_.substr(index_ + 1);
        }
#endif
    }
    if (ext_.empty()) {
        COMPV_DEBUG_INFO_EX(kModuleNameFileUtils, "Connot find extension for file path = %s", pcPath);
    }
    else {
        std::transform(ext_.begin(), ext_.end(), ext_.begin(), ::toupper); // UpperCase
    }
    return ext_;
}

COMPV_IMAGE_FORMAT CompVFileUtils::getImageFormat(const char* pcPath)
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

COMPV_ERROR_CODE CompVFileUtils::read(const char* pcPath, CompVBufferPtrPtr buffer)
{
    COMPV_CHECK_EXP_RETURN(!pcPath || !buffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVBufferPtr buffer_ = NULL;
    size_t size_ = CompVFileUtils::getSize(pcPath);
	if (!size_) {
		COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "File at %s is empty or doesn't exist", pcPath);
		return COMPV_ERROR_CODE_E_FAILED_TO_READ_FILE;
	}
	else {
        FILE* file_ = nullptr;
        void* mem_ = nullptr;
        if (!(file_ = CompVFileUtils::open(pcPath, "rb"))) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Can't open %s", pcPath);
            return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
        }
        mem_ = CompVMem::malloc(size_ + 1);
        if (!mem_) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Failed to alloc mem with size = %zu", size_);
            fclose(file_);
            return COMPV_ERROR_CODE_E_OUT_OF_MEMORY;
        }
        size_t read_;
        if (size_ != (read_ = fread(mem_, 1, size_, file_))) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "fread(%s) returned %zu instead of %zu", pcPath, read_, size_);
            fclose(file_);
            CompVMem::free(&mem_);
            return COMPV_ERROR_CODE_E_FAILED_TO_READ_FILE;
        }
		*(reinterpret_cast<char*>(mem_) + size_) = '\0'; //!\ required when reading sources to avoid garbage (e.g OpenCL source *.cl) 
        if (COMPV_ERROR_CODE_IS_NOK(CompVBuffer::newObjAndTakeData(&mem_, size_, &buffer_))) {
            COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Failed to create new CompVBuffer object");
        }
        fclose(file_);
        CompVMem::free(&mem_);
    }
    *buffer = buffer_;
    return COMPV_ERROR_CODE_S_OK;
}

// Unlike standard "fopen", this function can deal with
// Android's assets and iOS' bundles.
FILE* CompVFileUtils::open(const char* fname, const char* mode)
{
    if (!fname || !mode) {
        COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "Invalid parameter (%s, %s)", fname, mode);
        return nullptr;
    }

#if COMPV_OS_ANDROID
	const std::string mode_ = mode;
	const bool openForWriteOperations = 
		(mode_.find('w') != std::string::npos) || (mode_.back() == '+');
	if (openForWriteOperations) {
		// Using standard file operations to write to internal folder (e.g. '/storage/emulated/0/Android/data/org.doubango.ultimateAlpr/files')
		// works fine when the app is called from Java Activity (e.g. ultimateALPR-SDK use case).
		COMPV_DEBUG_INFO_EX(kModuleNameFileUtils, "Asset manager can't handle write operations, using standard file ops");
	}
	else {
		if (compv_android_have_assetmgr()) {
			return compv_android_asset_fopen(fname, mode);
		}
		else {
			COMPV_DEBUG_INFO_CODE_ONCE("Not using asset manager");
		}
	}
#endif /* COMPV_OS_ANDROID */
    return fopen(fname, mode);
}

COMPV_ERROR_CODE CompVFileUtils::close(FILE** file)
{
	if (file && *file) {
		fclose(*file);
		*file = nullptr;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFileUtils::write(const char* pcPath, const void* data, size_t count)
{
	COMPV_CHECK_EXP_RETURN(!pcPath || !data || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	FILE* file = CompVFileUtils::open(pcPath, "wb+");
	if (!file) {
#if COMPV_OS_WINDOWS || COMPV_OS_LINUX || COMPV_OS_PI
		perror("Cannot open/create the file");
#endif
		COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "open(%s) failed", pcPath);
		return COMPV_ERROR_CODE_E_FAILED_TO_OPEN_FILE;
	}
	const size_t wrote = fwrite(data, 1, count, file);
	fclose(file);
	if (wrote != count) {
		COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "fwrite(%s) returned %zu instead of %zu", pcPath, wrote, count);
		return COMPV_ERROR_CODE_E_INVALID_STATE;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Get list of files in the directory
COMPV_ERROR_CODE CompVFileUtils::getFilesInDir(const char* dir, std::vector<std::string>& paths)
{
	COMPV_CHECK_EXP_RETURN(!dir, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	paths.clear();
	// Collect files
	DIR *dir_ = opendir(dir);
	if (!dir_) {
		COMPV_DEBUG_ERROR_EX(kModuleNameFileUtils, "opendir(%s) failed", dir);
		return COMPV_ERROR_CODE_E_FAILED_TO_OPEN_FILE;
	}
	struct dirent *ent;
	std::string dir_path = std::string(dir);
	if (dir_path.back() != '/') {
		dir_path += "/";
	}
	COMPV_DEBUG_INFO_EX(kModuleNameFileUtils, "Loading files in %s ...", dir);
	while ((ent = readdir(dir_))) {
		const std::string filename = std::string(ent->d_name);
		if (filename == "." || filename == "..") {
			continue;
		}
		paths.push_back(dir_path + std::string(filename));
	}
	closedir(dir_);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
