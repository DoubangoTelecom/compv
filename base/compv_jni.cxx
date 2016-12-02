/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_jni.h"
#if defined(HAVE_JNI_H) || COMPV_OS_ANDROID
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

std::string CompVJNI::toString(JNIEnv* jEnv, jstring jstr)
{
    if (!jEnv || !jstr) {
        COMPV_DEBUG_ERROR("Invalid parameter");
        return "";
    }
    const char* cstr = jEnv->GetStringUTFChars(jstr, NULL);
    std::string str(cstr ? cstr : "");
    jEnv->ReleaseStringUTFChars(jstr, cstr);
    return str;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_JNI_H) || COMPV_OS_ANDROID */

