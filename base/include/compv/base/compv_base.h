/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BASE_H_)
#define _COMPV_BASE_BASE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"

#include <string>
#if COMPV_OS_ANDROID
#include <sstream> /* std::ostringstream */
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVBase
{
protected:
    CompVBase();
public:
    virtual ~CompVBase();
    static COMPV_ERROR_CODE init(int32_t numThreads = -1);
    static COMPV_ERROR_CODE deInit();
    static COMPV_ERROR_CODE setTestingModeEnabled(bool bTesting);
    static bool isInitialized();
    static bool isInitializing();
    static bool isTestingMode();
#if COMPV_OS_WINDOWS
    static DWORD winMajorVersion() {
        return s_dwMajorVersion;
    }
    static DWORD winMinorVersion() {
        return s_dwMinorVersion;
    }
    static bool isWin8OrLater();
    static bool isWin7OrLater();
    static bool isWinVistaOrLater();
    static bool isWinXPOrLater();
#endif
#if COMPV_OS_ANDROID
	static std::string CPU_ABI() {
		return s_strCPU_ABI;
	}
#endif
	template <typename T>
	static std::string to_string(T value) {
#if COMPV_OS_ANDROID
		std::ostringstream os;
		os << value;
		return os.str();
#else
        return std::to_string(value);
#endif
	}

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static bool s_bInitialized;
    static bool s_bInitializing;
    static bool s_bTesting;
#if COMPV_OS_ANDROID
	static std::string s_strCPU_ABI;
#endif
#if COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_RT
    static DWORD s_dwMajorVersion;
    static DWORD s_dwMinorVersion;
#endif
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BASE_H_ */
