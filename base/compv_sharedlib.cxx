/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_sharedlib.h"
#include "compv/base/compv_fileutils.h"

#if COMPV_OS_WINDOWS
#	include <windows.h>
#else
#	include <dlfcn.h>
#endif

#define COMPV_THIS_CLASSNAME "CompVSharedLib"

COMPV_NAMESPACE_BEGIN()

CompVSharedLib::CompVSharedLib(void* pHandle COMPV_DEFAULT(NULL))
    : m_pHandle(pHandle)
{

}

CompVSharedLib::~CompVSharedLib()
{
    COMPV_CHECK_CODE_ASSERT(CompVSharedLib::close(m_pHandle));
}

void* CompVSharedLib::sym(const char* name)
{
    return CompVSharedLib::sym(m_pHandle, name);
}

COMPV_ERROR_CODE CompVSharedLib::open(const char* filePath, void** handle, bool quiet COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!filePath || !handle, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Loading sharded library from %s", filePath);
    void* handle_ = nullptr;
#if COMPV_OS_WINDOWS
#	if COMPV_OS_WINDOWS_RT
    wchar_t* szPath = (wchar_t*)tsk_calloc(tsk_strlen(path) + 1, sizeof(wchar_t));
    swprintf(szPath, tsk_strlen(path) * sizeof(wchar_t), szFormat, path);
    handle_ = LoadPackagedLibrary(szPath, 0x00000000);
    TSK_FREE(szPath);
#	else /* Windows desktop/CE */
#if COMPV_OS_WINDOWS_CE
    wchar_t* szPath = (wchar_t*)tsk_calloc(tsk_strlen(path) + 1, sizeof(wchar_t));
    swprintf_s(szPath, tsk_strlen(path) * sizeof(wchar_t), szFormat, path);
    handle_ = LoadLibrary(szPath);
    TSK_FREE(szPath);
#else
    UINT currErrMode = SetErrorMode(SEM_FAILCRITICALERRORS); // save current ErrorMode. GetErrorMode() not supported on XP.
    SetErrorMode(currErrMode | SEM_FAILCRITICALERRORS);
    handle_ = static_cast<void*>(LoadLibraryA(filePath));
    SetErrorMode(currErrMode); // restore ErrorMode
#endif /* !COMPV_OS_WINDOWS_CE */
#	endif /*end-of-else-COMPV_OS_WINDOWS_RT*/
#else
    handle_ = dlopen(filePath, RTLD_LAZY);
#endif

    if (!handle_) {
		if (quiet) {
			return COMPV_ERROR_CODE_E_NOT_FOUND;
		}
#if COMPV_OS_WINDOWS
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to load library with path=%s, Error: 0x%08x", filePath, GetLastError());
#else
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to load library with path=%s, Error: %s", filePath, dlerror());
#endif
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_FOUND); 
    }

    *handle = handle_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSharedLib::close(void* handle)
{
    if (handle) {
#if COMPV_OS_WINDOWS
        COMPV_CHECK_EXP_RETURN(FreeLibrary(static_cast<HMODULE>(handle)) != TRUE, COMPV_ERROR_CODE_E_SYSTEM);
#else
        COMPV_CHECK_EXP_RETURN(dlclose(handle) != 0, COMPV_ERROR_CODE_E_SYSTEM);
#endif
    }
    return COMPV_ERROR_CODE_S_OK;
}

void* CompVSharedLib::sym(void* handle, const char* name)
{
    void* sym_ = NULL;
    COMPV_CHECK_EXP_BAIL(!handle || !name, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

#if COMPV_OS_WINDOWS
#	if COMPV_OS_WINDOWS_CE
    sym_ = static_cast<void*>(GetProcAddressA(static_cast<HMODULE>(handle), name));
#	else
    sym_ = static_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), name));
#	endif
#else
    sym_ = dlsym(handle, name);
#endif

bail:
    return sym_;
}

COMPV_ERROR_CODE CompVSharedLib::newObj(CompVSharedLibPtrPtr sharedlib, const char* filePath, bool quiet COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!sharedlib || !filePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    void* handle_ = nullptr;
	COMPV_ERROR_CODE err = CompVSharedLib::open(filePath, &handle_, quiet);
	if (quiet && COMPV_ERROR_CODE_IS_NOK(err)) {
		return err;
	}
	COMPV_CHECK_CODE_BAIL(err);
    COMPV_CHECK_CODE_BAIL(err = CompVSharedLib::newObj(sharedlib, handle_));
	if (!quiet) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Loaded shared lib: %s", filePath);
	}
bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_ASSERT(CompVSharedLib::close(handle_));
    }
    return err;
}

COMPV_ERROR_CODE CompVSharedLib::newObj(CompVSharedLibPtrPtr sharedlib, void* handle)
{
    COMPV_CHECK_EXP_RETURN(!sharedlib || !handle, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVSharedLibPtr sharedlib_ = new CompVSharedLib(handle);
    COMPV_CHECK_EXP_RETURN(!sharedlib_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    *sharedlib = sharedlib_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
