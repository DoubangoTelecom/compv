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

// You *must* clear the exception before calling this function
std::string CompVJNI::toString(JNIEnv* jEnv, jthrowable exc)
{
	if (!jEnv || !exc) {
		COMPV_DEBUG_ERROR("Invalid parameter");
		return "";
	}
	std::string error = "";
	jclass klass = NULL;
	jmethodID toString = NULL;
	jstring jstr = NULL;
	const char* utf;

	klass = jEnv->FindClass("java/lang/Object");
	COMPV_CHECK_EXP_BAIL(!klass, COMPV_ERROR_CODE_E_JNI);
	toString = jEnv->GetMethodID(klass, "toString", "()Ljava/lang/String;");
	COMPV_CHECK_EXP_BAIL(!toString, COMPV_ERROR_CODE_E_JNI);
	jstr = reinterpret_cast<jstring>(jEnv->CallObjectMethod(exc, toString));
	COMPV_CHECK_EXP_BAIL(!jstr, COMPV_ERROR_CODE_E_JNI);
	utf = jEnv->GetStringUTFChars(jstr, NULL);
	COMPV_CHECK_EXP_BAIL(!jstr, COMPV_ERROR_CODE_E_JNI);
	error = std::string(utf);

bail:
	COMPV_jni_checkException1(jEnv);
	if (jstr) {
		jEnv->ReleaseStringUTFChars(jstr, utf);
	}
	COMPV_jni_DeleteLocalRef(jEnv, klass);
	COMPV_jni_DeleteLocalRef(jEnv, jstr);
	return error;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_JNI_H) || COMPV_OS_ANDROID */

