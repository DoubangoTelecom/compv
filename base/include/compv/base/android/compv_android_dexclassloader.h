/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_ANDROID_DEXCLASSLOADER_H_)
#define _COMPV_BASE_ANDROID_DEXCLASSLOADER_H_

#include "compv/base/compv_config.h"
#if COMPV_OS_ANDROID
#include "compv/base/compv_obj.h"

#include <jni.h>

COMPV_NAMESPACE_BEGIN()

/*
This class implements Android's Dex Class loader: https://developer.android.com/reference/dalvik/system/DexClassLoader.html
*/

class COMPV_BASE_API CompVAndroidDexClassLoader
{
protected:
	CompVAndroidDexClassLoader();
public:
	virtual ~CompVAndroidDexClassLoader();

	static COMPV_ERROR_CODE init(JNIEnv* jEnv);
	static COMPV_ERROR_CODE deInit(JNIEnv* jEnv);
	static COMPV_ERROR_CODE moveDexFileFromAssetsToData(JNIEnv* jEnv, jobject activity, const std::string& dexFileName, const std::string& nativeLibFileName, std::string& dexPath, std::string& optimizedDirectory, std::string& librarySearchPath);
	static COMPV_ERROR_CODE loadClass(JNIEnv* jEnv, const std::string& className, const std::string& dexPath, const std::string& optimizedDirectory, const std::string& librarySearchPath, jclass *clazz);
	static COMPV_ERROR_CODE newInstance(JNIEnv* jEnv, const std::string& className, const std::string& dexPath, const std::string& optimizedDirectory, const std::string& librarySearchPath, jobject *instance);
	static COMPV_ERROR_CODE newInstance(JNIEnv* jEnv, jclass clazz, jobject *instance);

	static bool isInitialized() {
		return s_bInitialized;
	}
	static long InitializationID() {
		return s_lInitializationID;
	}

private:
	static bool s_bInitialized;
	static long s_lInitializationID;
	static jclass s_ClassDexLoader;
	static jclass s_ClassContext;
	static jclass s_ClassAssetManager;
	static jclass s_ClassFile;
	static jclass s_ClassBufferedInputStream;
	static jclass s_ClassBufferedOutputStream;
	static jclass s_ClassFileOutputStream;
	static jclass s_ClassZipInputStream;
	static jobject s_ObjectLoaderParent;
	static jmethodID s_MethodLoadClass;
	static jmethodID s_MethodDexLoaderConstructor;
	static jmethodID s_MethodContextGetDir;
	static jmethodID s_MethodContextGetAssets;
	static jmethodID s_MethodAssetManagerOpen;
	static jmethodID s_MethodFileConstructor;
	static jmethodID s_MethodFileGetAbsolutePath;
	static jmethodID s_MethodBufferedInputStreamConstructor;
	static jmethodID s_MethodBufferedInputStreamRead;
	static jmethodID s_MethodBufferedInputStreamClose;
	static jmethodID s_MethodBufferedInputStreamMarkSupported;
	static jmethodID s_MethodBufferedInputStreamMark;
	static jmethodID s_MethodBufferedInputStreamReset;
	static jmethodID s_MethodBufferedOutputStreamConstructor;
	static jmethodID s_MethodBufferedOutputStreamWrite;
	static jmethodID s_MethodBufferedOutputStreamClose;
	static jmethodID s_MethodFileOutputStreamConstructor;
	static jmethodID s_MethodFileOutputStreamConstructor2;
	static jmethodID s_MethodFileOutputStreamWrite;
	static jmethodID s_MethodZipInputStreamConstructor;
	static jmethodID s_MethodZipInputStreamGetNextEntry;
	static jmethodID s_MethodZipInputStreamRead;
	static jmethodID s_MethodZipEntryGetName;
	static jint s_IntContext_MODE_PRIVATE;
};

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID */
#endif /* _COMPV_BASE_ANDROID_DEXCLASSLOADER_H_ */
