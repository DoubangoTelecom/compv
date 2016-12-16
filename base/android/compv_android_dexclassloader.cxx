/* Copyright(C) 2016 - 2017 Doubango Telecom <https://www.doubango.org>
*File author : Mamadou DIOP(Doubango Telecom, France).
* License : GPLv3.For commercial license please contact us.
* Source code : https://github.com/DoubangoTelecom/compv
*WebSite : http ://compv.org
*/
#include "compv/base/android/compv_android_dexclassloader.h"
#include "compv/base/compv_jni.h"
#include "compv/base/compv_base.h"

#if COMPV_OS_ANDROID

/*
This class implements Android's Dex Class loader: [0]
* [0] DexClassLoader: https://developer.android.com/reference/dalvik/system/DexClassLoader.html
* [1] ClassLoader: https://developer.android.com/reference/java/lang/ClassLoader.html
* [2] https://developer.android.com/training/articles/perf-jni.html
*/

COMPV_NAMESPACE_BEGIN()

bool CompVAndroidDexClassLoader::s_bInitialized = false;
long CompVAndroidDexClassLoader::s_lInitializationID = 0; // Identifier used to track the loader validity
jclass CompVAndroidDexClassLoader::s_ClassDexLoader = NULL;
jclass CompVAndroidDexClassLoader::s_ClassContext = NULL;
jclass CompVAndroidDexClassLoader::s_ClassAssetManager = NULL;
jclass CompVAndroidDexClassLoader::s_ClassFile = NULL;
jclass CompVAndroidDexClassLoader::s_ClassBufferedInputStream = NULL;
jclass CompVAndroidDexClassLoader::s_ClassBufferedOutputStream = NULL;
jclass CompVAndroidDexClassLoader::s_ClassFileOutputStream = NULL;
jclass CompVAndroidDexClassLoader::s_ClassZipInputStream = NULL;
jobject CompVAndroidDexClassLoader::s_ObjectLoaderParent = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodLoadClass = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodDexLoaderConstructor = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodContextGetDir = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodContextGetAssets = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodAssetManagerOpen = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodFileConstructor = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodFileGetAbsolutePath = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedInputStreamConstructor = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedInputStreamRead = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedInputStreamClose = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedInputStreamMarkSupported = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedInputStreamMark = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedInputStreamReset = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedOutputStreamConstructor = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedOutputStreamWrite = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodBufferedOutputStreamClose = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodFileOutputStreamConstructor = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodFileOutputStreamConstructor2 = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodFileOutputStreamWrite = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodZipInputStreamConstructor = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodZipInputStreamGetNextEntry = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodZipInputStreamRead = NULL;
jmethodID CompVAndroidDexClassLoader::s_MethodZipEntryGetName = NULL;
jint CompVAndroidDexClassLoader::s_IntContext_MODE_PRIVATE = 0;

CompVAndroidDexClassLoader::CompVAndroidDexClassLoader()
{

}

CompVAndroidDexClassLoader::~CompVAndroidDexClassLoader()
{

}

// This function is automatically called by NativeActivity::onResume().
COMPV_ERROR_CODE CompVAndroidDexClassLoader::init(JNIEnv* jEnv)
{
	if (s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!jEnv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO("CompVAndroidDexClassLoader::init(%p)", jEnv);

	// According to [2]: The implementation is only required to reserve slots for 16 local references, so if you need more than that you should either delete as you go or use 
	// EnsureLocalCapacity/PushLocalFrame to reserve more.
	// Make sure number of 'jobject' and 'jclass' local references are < 16. Ignore 'jmethodID' and 'jfieldID' references.

	// Android uses a single JVM (see [2]) which means we're sure the one used for init() will be used for deInit()
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	jobject localObjectLoaderParent = NULL; // Instance of ClassLoader(see [1]). https://developer.android.com/reference/java/lang/ClassLoader.html
	jclass localClassContext = NULL; // android.content.Context. https://developer.android.com/reference/android/content/Context.html
	jclass localClassJavaLang = NULL; // java/lang/Class. https://developer.android.com/reference/java/lang/Class.html
	jclass localClassDexLoader = NULL; // dalvik/system/DexClassLoader. https://developer.android.com/reference/dalvik/system/DexClassLoader.html
	jclass localClassAssetManager = NULL; // android.content.res.AssetManager. https://developer.android.com/reference/android/content/res/AssetManager.html
	jclass localClassFile = NULL; // java.io.File. https://developer.android.com/reference/java/io/File.html
	jclass localClassBufferedInputStream = NULL; // java.io.BufferedInputStream. https://developer.android.com/reference/java/io/BufferedInputStream.html
	jclass localClassBufferedOutputStream = NULL; // java.io.BufferedOutputStream. https://developer.android.com/reference/java/io/BufferedOutputStream.html
	jclass localClassFileOutputStream = NULL; // java.io.FileOutputStream. https://developer.android.com/reference/java/io/FileOutputStream.html
	jclass localClassZipInputStream = NULL; // 	java.util.zip.ZipInputStream. https://developer.android.com/reference/java/util/zip/ZipInputStream.html
	jclass localClassZipEntry = NULL; // java.util.zip.ZipEntry. https://developer.android.com/reference/java/util/zip/ZipEntry.html
	jmethodID localMethodDexLoaderConstructor = NULL; // dalvik/system/DexClassLoader/<init>. https://developer.android.com/reference/dalvik/system/DexClassLoader.html#DexClassLoader(java.lang.String, java.lang.String, java.lang.String, java.lang.ClassLoader)
	jmethodID localMethodGetClassLoader = NULL; // java.lang.Calss.getClassLoader() https://developer.android.com/reference/java/lang/Class.html#getClassLoader()
	jmethodID localMethodLoadClass = NULL; // ClassLoader.loadClass(String) https://developer.android.com/reference/java/lang/ClassLoader.html#loadClass(java.lang.String)
	jmethodID localMethodContextGetDir = NULL; // android.content.Context.getDir(String, int) https://developer.android.com/reference/android/content/Context.html#getDir(java.lang.String, int)
	jmethodID localMethodGetAssets = NULL; // android.content.Context.getAssets() https://developer.android.com/reference/android/content/Context.html#getAssets()
	jmethodID localMethodAssetManagerOpen = NULL; // android.content.res.AssetManager.open(String)	https://developer.android.com/reference/android/content/res/AssetManager.html#open(java.lang.String)
	jmethodID localMethodFileConstructor = NULL; // java.io.File.<init>(File dir, String name) https://developer.android.com/reference/java/io/File.html#File(java.io.File, java.lang.String)
	jmethodID localMethodFileGetAbsolutePath = NULL; // String = java.io.File.getAbsolutePath() https://developer.android.com/reference/java/io/File.html#getAbsolutePath()
	jmethodID localMethodBufferedInputStreamConstructor = NULL; // java.io.BufferedInputStream.<init>(InputStream) https://developer.android.com/reference/java/io/BufferedInputStream.html#BufferedInputStream(java.io.InputStream)
	jmethodID localMethodBufferedInputStreamRead = NULL; // java.io.BufferedInputStream.read(byte[], int, int) https://developer.android.com/reference/java/io/BufferedInputStream.html#read(byte[], int, int)
	jmethodID localMethodBufferedInputStreamClose = NULL; // java.io.BufferedInputStream.close() https://developer.android.com/reference/java/io/BufferedInputStream.html#close()
	jmethodID localMethodBufferedInputStreamMarkSupported = NULL; // boolean = java.io.BufferedInputStream.markSupported() https://developer.android.com/reference/java/io/BufferedInputStream.html#markSupported()
	jmethodID localMethodBufferedInputStreamMark = NULL; // void = java.io.BufferedInputStream.mark(int) https://developer.android.com/reference/java/io/BufferedInputStream.html#mark(int)
	jmethodID localMethodBufferedInputStreamReset = NULL; // void = java.io.BufferedInputStream.reset() https://developer.android.com/reference/java/io/BufferedInputStream.html#reset()
	jmethodID localMethodBufferedOutputStreamConstructor = NULL;  // java.io.BufferedOutputStream.<init>(OutputStream) https://developer.android.com/reference/java/io/BufferedOutputStream.html#BufferedOutputStream(java.io.OutputStream)
	jmethodID localMethodBufferedOutputStreamWrite = NULL; // java.io.BufferedOutputStream.write(byte[], int, int) https://developer.android.com/reference/java/io/BufferedOutputStream.html#write(byte[], int, int)
	jmethodID localMethodBufferedOutputStreamClose = NULL; // java.io.BufferedOutputStream.close() Inherited from OutputStream.close()
	jmethodID localMethodFileOutputStreamConstructor = NULL; // java.io.FileOutputStream.<init>(File) https://developer.android.com/reference/java/io/FileOutputStream.html#FileOutputStream(java.io.File)
	jmethodID localMethodFileOutputStreamConstructor2 = NULL; // java.io.FileOutputStream.<init>(String) https://developer.android.com/reference/java/io/FileOutputStream.html#FileOutputStream(java.lang.String)
	jmethodID localMethodFileOutputStreamWrite = NULL; // void = java.io.FileOutputStream.write(byte[], int, int) https://developer.android.com/reference/java/io/FileOutputStream.html#write(byte[], int, int)
	jmethodID localMethodZipInputStreamConstructor = NULL; // java.util.zip.ZipInputStream.<init>(InputStream) https://developer.android.com/reference/java/util/zip/ZipInputStream.html#ZipInputStream(java.io.InputStream)
	jmethodID localMethodZipInputStreamGetNextEntry = NULL; // ZipEntry = java.util.zip.ZipInputStream.getNextEntry() https://developer.android.com/reference/java/util/zip/ZipInputStream.html#getNextEntry()
	jmethodID localMethodZipInputStreamRead = NULL; // int = java.util.zip.ZipInputStream.read(byte[], int, int) https://developer.android.com/reference/java/util/zip/ZipInputStream.html#read(byte[], int, int)
	jmethodID localMethodZipEntryGetName = NULL; // String java.util.zip.ZipEntry.getName() https://developer.android.com/reference/java/util/zip/ZipEntry.html#getName()

	jfieldID localField;

	// Initialize parent ClassLoader
	localClassJavaLang = jEnv->FindClass("java/lang/Class");
	COMPV_CHECK_EXP_BAIL(!localClassJavaLang, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodGetClassLoader = jEnv->GetMethodID(localClassJavaLang, "getClassLoader", "()Ljava/lang/ClassLoader;");
	if (localMethodGetClassLoader) {
		localObjectLoaderParent = jEnv->CallObjectMethod(localClassJavaLang, localMethodGetClassLoader);
		COMPV_CHECK_EXP_BAIL(!localObjectLoaderParent, (err = COMPV_ERROR_CODE_E_JNI));
	}

	// Initialize DexClassLoader (dalvik/system/DexClassLoader)
	localClassDexLoader = jEnv->FindClass("dalvik/system/DexClassLoader");
	COMPV_CHECK_EXP_BAIL(!localClassDexLoader, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodDexLoaderConstructor = jEnv->GetMethodID(localClassDexLoader, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodDexLoaderConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodLoadClass = jEnv->GetMethodID(localClassDexLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	COMPV_CHECK_EXP_BAIL(!localMethodLoadClass, (err = COMPV_ERROR_CODE_E_JNI));

	// android.content.Context
	localClassContext = jEnv->FindClass("android/content/Context");
	COMPV_CHECK_EXP_BAIL(!localClassContext, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodContextGetDir = jEnv->GetMethodID(localClassContext, "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
	COMPV_CHECK_EXP_BAIL(!localMethodContextGetDir, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodGetAssets = jEnv->GetMethodID(localClassContext, "getAssets", "()Landroid/content/res/AssetManager;");
	COMPV_CHECK_EXP_BAIL(!localMethodGetAssets, (err = COMPV_ERROR_CODE_E_JNI));
	localField = jEnv->GetStaticFieldID(localClassContext, "MODE_PRIVATE", "I");
	COMPV_CHECK_EXP_BAIL(!localField, (err = COMPV_ERROR_CODE_E_JNI));
	s_IntContext_MODE_PRIVATE = jEnv->GetStaticIntField(localClassContext, localField);

	// android.content.res.AssetManager
	localClassAssetManager = jEnv->FindClass("android/content/res/AssetManager");
	COMPV_CHECK_EXP_BAIL(!localClassAssetManager, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodAssetManagerOpen = jEnv->GetMethodID(localClassAssetManager, "open", "(Ljava/lang/String;)Ljava/io/InputStream;");
	COMPV_CHECK_EXP_BAIL(!localMethodAssetManagerOpen, (err = COMPV_ERROR_CODE_E_JNI));

	// java.io.File
	localClassFile = jEnv->FindClass("java/io/File");
	COMPV_CHECK_EXP_BAIL(!localClassFile, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodFileConstructor = jEnv->GetMethodID(localClassFile, "<init>", "(Ljava/io/File;Ljava/lang/String;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodFileConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodFileGetAbsolutePath = jEnv->GetMethodID(localClassFile, "getAbsolutePath", "()Ljava/lang/String;");
	COMPV_CHECK_EXP_BAIL(!localMethodFileGetAbsolutePath, (err = COMPV_ERROR_CODE_E_JNI));

	// java.io.BufferedInputStream
	localClassBufferedInputStream = jEnv->FindClass("java/io/BufferedInputStream");
	COMPV_CHECK_EXP_BAIL(!localClassBufferedInputStream, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedInputStreamConstructor = jEnv->GetMethodID(localClassBufferedInputStream, "<init>", "(Ljava/io/InputStream;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedInputStreamConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedInputStreamRead = jEnv->GetMethodID(localClassBufferedInputStream, "read", "([BII)I");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedInputStreamRead, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedInputStreamClose = jEnv->GetMethodID(localClassBufferedInputStream, "close", "()V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedInputStreamClose, (err = COMPV_ERROR_CODE_E_JNI));	
	localMethodBufferedInputStreamMarkSupported = jEnv->GetMethodID(localClassBufferedInputStream, "markSupported", "()Z");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedInputStreamMarkSupported, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedInputStreamMark = jEnv->GetMethodID(localClassBufferedInputStream, "mark", "(I)V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedInputStreamMark, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedInputStreamReset = jEnv->GetMethodID(localClassBufferedInputStream, "reset", "()V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedInputStreamReset, (err = COMPV_ERROR_CODE_E_JNI));

	// java.io.BufferedOutputStream
	localClassBufferedOutputStream = jEnv->FindClass("java/io/BufferedOutputStream");
	COMPV_CHECK_EXP_BAIL(!localClassBufferedOutputStream, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedOutputStreamConstructor = jEnv->GetMethodID(localClassBufferedOutputStream, "<init>", "(Ljava/io/OutputStream;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedOutputStreamConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedOutputStreamWrite = jEnv->GetMethodID(localClassBufferedOutputStream, "write", "([BII)V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedOutputStreamWrite, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodBufferedOutputStreamClose = jEnv->GetMethodID(localClassBufferedOutputStream, "close", "()V");
	COMPV_CHECK_EXP_BAIL(!localMethodBufferedOutputStreamClose, (err = COMPV_ERROR_CODE_E_JNI));

	// java.io.FileOutputStream
	localClassFileOutputStream = jEnv->FindClass("java/io/FileOutputStream");
	COMPV_CHECK_EXP_BAIL(!localClassFileOutputStream, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodFileOutputStreamConstructor = jEnv->GetMethodID(localClassFileOutputStream, "<init>", "(Ljava/io/File;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodFileOutputStreamConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodFileOutputStreamConstructor2 = jEnv->GetMethodID(localClassFileOutputStream, "<init>", "(Ljava/lang/String;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodFileOutputStreamConstructor2, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodFileOutputStreamWrite = jEnv->GetMethodID(localClassFileOutputStream, "write", "([BII)V");
	COMPV_CHECK_EXP_BAIL(!localMethodFileOutputStreamWrite, (err = COMPV_ERROR_CODE_E_JNI));
	

	// 	java.util.zip.ZipInputStream
	localClassZipInputStream = jEnv->FindClass("java/util/zip/ZipInputStream");
	COMPV_CHECK_EXP_BAIL(!localClassZipInputStream, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodZipInputStreamConstructor = jEnv->GetMethodID(localClassZipInputStream, "<init>", "(Ljava/io/InputStream;)V");
	COMPV_CHECK_EXP_BAIL(!localMethodZipInputStreamConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodZipInputStreamGetNextEntry = jEnv->GetMethodID(localClassZipInputStream, "getNextEntry", "()Ljava/util/zip/ZipEntry;");
	COMPV_CHECK_EXP_BAIL(!localMethodZipInputStreamGetNextEntry, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodZipInputStreamRead = jEnv->GetMethodID(localClassZipInputStream, "read", "([BII)I");
	COMPV_CHECK_EXP_BAIL(!localMethodZipInputStreamRead, (err = COMPV_ERROR_CODE_E_JNI));

	// 	java.util.zip.ZipEntry
	localClassZipEntry = jEnv->FindClass("java/util/zip/ZipEntry");
	COMPV_CHECK_EXP_BAIL(!localClassZipEntry, (err = COMPV_ERROR_CODE_E_JNI));
	localMethodZipEntryGetName = jEnv->GetMethodID(localClassZipEntry, "getName", "()Ljava/lang/String;");
	COMPV_CHECK_EXP_BAIL(!localMethodZipEntryGetName, (err = COMPV_ERROR_CODE_E_JNI));

	// Initialize the global references
	s_ClassDexLoader = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassDexLoader));
	COMPV_CHECK_EXP_BAIL(!s_ClassDexLoader, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassContext = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassContext));
	COMPV_CHECK_EXP_BAIL(!s_ClassContext, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassAssetManager = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassAssetManager));
	COMPV_CHECK_EXP_BAIL(!s_ClassAssetManager, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassFile = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassFile));
	COMPV_CHECK_EXP_BAIL(!s_ClassFile, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassBufferedInputStream = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassBufferedInputStream));
	COMPV_CHECK_EXP_BAIL(!s_ClassBufferedInputStream, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassBufferedOutputStream = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassBufferedOutputStream));
	COMPV_CHECK_EXP_BAIL(!s_ClassBufferedOutputStream, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassFileOutputStream = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassFileOutputStream));
	COMPV_CHECK_EXP_BAIL(!s_ClassFileOutputStream, (err = COMPV_ERROR_CODE_E_JNI));
	s_ClassZipInputStream = reinterpret_cast<jclass>(jEnv->NewGlobalRef(localClassZipInputStream));
	COMPV_CHECK_EXP_BAIL(!s_ClassZipInputStream, (err = COMPV_ERROR_CODE_E_JNI));
	s_ObjectLoaderParent = jEnv->NewGlobalRef(localObjectLoaderParent);
	COMPV_CHECK_EXP_BAIL(!s_ObjectLoaderParent, (err = COMPV_ERROR_CODE_E_JNI));

	s_MethodLoadClass = localMethodLoadClass;
	s_MethodDexLoaderConstructor = localMethodDexLoaderConstructor;
	s_MethodContextGetDir = localMethodContextGetDir;
	s_MethodContextGetAssets = localMethodGetAssets;
	s_MethodAssetManagerOpen = localMethodAssetManagerOpen;
	s_MethodFileConstructor = localMethodFileConstructor;
	s_MethodFileGetAbsolutePath = localMethodFileGetAbsolutePath;
	s_MethodBufferedInputStreamConstructor = localMethodBufferedInputStreamConstructor;
	s_MethodBufferedInputStreamRead = localMethodBufferedInputStreamRead;
	s_MethodBufferedInputStreamClose = localMethodBufferedInputStreamClose;
	s_MethodBufferedInputStreamMarkSupported = localMethodBufferedInputStreamMarkSupported;
	s_MethodBufferedInputStreamMark = localMethodBufferedInputStreamMark;
	s_MethodBufferedInputStreamReset = localMethodBufferedInputStreamReset;
	s_MethodBufferedOutputStreamConstructor = localMethodBufferedOutputStreamConstructor;
	s_MethodBufferedOutputStreamWrite = localMethodBufferedOutputStreamWrite;
	s_MethodBufferedOutputStreamClose = localMethodBufferedOutputStreamClose;
	s_MethodFileOutputStreamConstructor = localMethodFileOutputStreamConstructor;
	s_MethodFileOutputStreamConstructor2 = localMethodFileOutputStreamConstructor2;
	s_MethodFileOutputStreamWrite = localMethodFileOutputStreamWrite;
	s_MethodZipInputStreamConstructor = localMethodZipInputStreamConstructor;
	s_MethodZipInputStreamGetNextEntry = localMethodZipInputStreamGetNextEntry;
	s_MethodZipInputStreamRead = localMethodZipInputStreamRead;
	s_MethodZipEntryGetName = localMethodZipEntryGetName;

	s_bInitialized = true;
	compv_atomic_inc(&s_lInitializationID);
	COMPV_DEBUG_INFO("Android DexClassLoader successfully initialized");

bail:
	// Goto bail probably called because of an exception (such as NoSuchMethodException when method GetMethodID called) -> clear exception
	COMPV_jni_checkException1(jEnv);
	
	COMPV_jni_DeleteLocalRef(jEnv, localObjectLoaderParent);
	COMPV_jni_DeleteLocalRef(jEnv, localClassContext);
	COMPV_jni_DeleteLocalRef(jEnv, localClassAssetManager);
	COMPV_jni_DeleteLocalRef(jEnv, localClassJavaLang);
	COMPV_jni_DeleteLocalRef(jEnv, localClassDexLoader);
	COMPV_jni_DeleteLocalRef(jEnv, localClassFile);
	COMPV_jni_DeleteLocalRef(jEnv, localClassBufferedInputStream);
	COMPV_jni_DeleteLocalRef(jEnv, localClassBufferedOutputStream);
	COMPV_jni_DeleteLocalRef(jEnv, localClassFileOutputStream);
	COMPV_jni_DeleteLocalRef(jEnv, localClassZipInputStream);
	COMPV_jni_DeleteLocalRef(jEnv, localClassZipEntry);
	
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(CompVAndroidDexClassLoader::deInit(jEnv));
	}
	return err;
}

// This function is automatically called by NativeActivity::onDestroy().
COMPV_ERROR_CODE CompVAndroidDexClassLoader::deInit(JNIEnv* jEnv)
{
	COMPV_CHECK_EXP_RETURN(!jEnv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO("CompVAndroidDexClassLoader::deInit(%p)", jEnv);

	// Android uses a single JVM (see [2]) which means we're sure the one used for init() will be used for deInit()
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassDexLoader);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassContext);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassAssetManager);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassFile);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassBufferedInputStream);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassBufferedOutputStream);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassFileOutputStream);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ClassZipInputStream);
	COMPV_jni_DeleteGlobalRef(jEnv, s_ObjectLoaderParent);

	s_MethodLoadClass = NULL;
	s_MethodDexLoaderConstructor = NULL;
	s_bInitialized = false;
	return err;
}

COMPV_ERROR_CODE CompVAndroidDexClassLoader::moveDexFileFromAssetsToData(JNIEnv* jEnv, jobject activity, const std::string& dexFileName, const std::string& nativeLibFileName, std::string& dexPath, std::string& optimizedDirectory, std::string& librarySearchPath)
{
	COMPV_CHECK_EXP_RETURN(!jEnv || !activity || dexFileName.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVAndroidDexClassLoader::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);

	//!\\ Important: Make sure not to use more than #16 local references: see [2] for more information.

	static const char* kDexDirName = "dex";
	static const char* kDexOptimizDirName = "optimizdex";
	static const jint kMaxBufferSize = 8192;
	
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	jstring jstringDirName = NULL;
	jstring jstringDexFileName = NULL;
	jstring jstringZipEntryName = NULL;
	jobject jobjectDirFile = NULL;
	jobject jobjectDexInternalStorageFile = NULL;
	jobject jobjectBufferInputStream = NULL;
	jobject jobjectOutputStream = NULL;
	jobject jobjectAssets = NULL;
	jobject jobjectTemp = NULL;
	jobject jobjectZipInputStream = NULL;
	jobject jobjectZipEntry = NULL;
	jbyteArray jbyteArrayBuff = NULL;
	bool bExcOccured;
	jint len, total = 0;
	jboolean markSupported;

	// jobjectDexInternalStorageFile = new File(context.getDir(kDexDirName, Context.MODE_PRIVATE), dexFileName);
	jstringDexFileName = jEnv->NewStringUTF(dexFileName.c_str());
	COMPV_CHECK_EXP_BAIL(!jstringDexFileName, (err = COMPV_ERROR_CODE_E_JNI));
	jstringDirName = jEnv->NewStringUTF(kDexDirName);
	COMPV_CHECK_EXP_BAIL(!jstringDirName, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectDirFile = jEnv->CallObjectMethod(activity, s_MethodContextGetDir, jstringDirName, static_cast<jint>(s_IntContext_MODE_PRIVATE));
	COMPV_CHECK_EXP_BAIL(!jobjectDirFile, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectDexInternalStorageFile = jEnv->NewObject(s_ClassFile, s_MethodFileConstructor, jobjectDirFile, jstringDexFileName);
	COMPV_CHECK_EXP_BAIL(!jobjectDexInternalStorageFile, (err = COMPV_ERROR_CODE_E_JNI));

	// jobjectBufferInputStream = new BufferedInputStream(context.getAssets().open(dexFileName));
	jobjectAssets = jEnv->CallObjectMethod(activity, s_MethodContextGetAssets);
	COMPV_CHECK_EXP_BAIL(!jobjectAssets, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectTemp = jEnv->CallObjectMethod(jobjectAssets, s_MethodAssetManagerOpen, jstringDexFileName);
	COMPV_CHECK_EXP_BAIL(!jobjectTemp, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectBufferInputStream = jEnv->NewObject(s_ClassBufferedInputStream, s_MethodBufferedInputStreamConstructor, jobjectTemp);
	COMPV_CHECK_EXP_BAIL(!jobjectBufferInputStream, (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_jni_DeleteLocalRef(jEnv, jobjectTemp);
	// Make sure mark() is supported
	markSupported = jEnv->CallBooleanMethod(jobjectBufferInputStream, s_MethodBufferedInputStreamMarkSupported);
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_CHECK_EXP_RETURN(!markSupported, (err = COMPV_ERROR_CODE_E_SYSTEM));
	// jobjectBufferInputStream.mark(INT_MAX)
	jEnv->CallVoidMethod(jobjectBufferInputStream, s_MethodBufferedInputStreamMark, static_cast<jint>(INT_MAX));
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));

	// jobjectOutputStream = new BufferedOutputStream(new FileOutputStream(jobjectDexInternalStorageFile));
	jobjectTemp = jEnv->NewObject(s_ClassFileOutputStream, s_MethodFileOutputStreamConstructor, jobjectDexInternalStorageFile);
	COMPV_CHECK_EXP_BAIL(!jobjectTemp, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectOutputStream = jEnv->NewObject(s_ClassBufferedOutputStream, s_MethodBufferedOutputStreamConstructor, jobjectTemp);
	COMPV_CHECK_EXP_BAIL(!jobjectOutputStream, (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_jni_DeleteLocalRef(jEnv, jobjectTemp);

	// while ((len = jobjectBufferInputStream.read(buf, 0, kMaxBufferSize)) > 0)
	jbyteArrayBuff = jEnv->NewByteArray(kMaxBufferSize);
	COMPV_CHECK_EXP_BAIL(!jbyteArrayBuff, (err = COMPV_ERROR_CODE_E_JNI));
	while ((len = jEnv->CallIntMethod(jobjectBufferInputStream, s_MethodBufferedInputStreamRead, jbyteArrayBuff, static_cast<jint>(0), kMaxBufferSize)) > 0) {
		// jobjectOutputStream.write(buf, 0, len);
		jEnv->CallVoidMethod(jobjectOutputStream, s_MethodBufferedOutputStreamWrite, jbyteArrayBuff, static_cast<jint>(0), len);
		COMPV_jni_checkException(jEnv, &bExcOccured);
		COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
		total += len;
	}
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_DEBUG_INFO("DexClassLoader copied %d bytes for file '%s'", total, dexFileName.c_str());

	// jobjectOutputStream.close();
	jEnv->CallVoidMethod(jobjectOutputStream, s_MethodBufferedOutputStreamClose);
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_jni_DeleteLocalRef(jEnv, jobjectOutputStream);

	// dexPath = jobjectDexInternalStorageFile.getAbsolutePath()
	jobjectTemp = jEnv->CallObjectMethod(jobjectDexInternalStorageFile, s_MethodFileGetAbsolutePath);
	COMPV_CHECK_EXP_BAIL(!jobjectTemp, (err = COMPV_ERROR_CODE_E_JNI));
	dexPath = CompVJNI::toString(jEnv, reinterpret_cast<jstring>(jobjectTemp));
	COMPV_CHECK_EXP_BAIL(dexPath.empty(), (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_DEBUG_INFO("DexClassLoader for file name '%s' absolutePath = '%s'", dexFileName.c_str(), dexPath.c_str());
	COMPV_jni_DeleteLocalRef(jEnv, jobjectTemp);

	// optimizedDexOutputPath = context.getDir(kDexOptimizDirName, Context.MODE_PRIVATE).getAbsolutePath();
	COMPV_jni_DeleteLocalRef(jEnv, jstringDirName);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectDirFile);
	jstringDirName = jEnv->NewStringUTF(kDexOptimizDirName);
	COMPV_CHECK_EXP_BAIL(!jstringDirName, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectDirFile = jEnv->CallObjectMethod(activity, s_MethodContextGetDir, jstringDirName, static_cast<jint>(s_IntContext_MODE_PRIVATE));
	COMPV_CHECK_EXP_BAIL(!jobjectDirFile, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectTemp = jEnv->CallObjectMethod(jobjectDirFile, s_MethodFileGetAbsolutePath);
	COMPV_CHECK_EXP_BAIL(!jobjectTemp, (err = COMPV_ERROR_CODE_E_JNI));
	optimizedDirectory = CompVJNI::toString(jEnv, reinterpret_cast<jstring>(jobjectTemp));
	COMPV_CHECK_EXP_BAIL(optimizedDirectory.empty(), (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_DEBUG_INFO("DexClassLoader for file name '%s' optimizedDirectory = '%s'", dexFileName.c_str(), optimizedDirectory.c_str());
	COMPV_jni_DeleteLocalRef(jEnv, jobjectTemp);

	//librarySearchPath = optimizedDexOutputPath;
	librarySearchPath = optimizedDirectory;
	COMPV_DEBUG_INFO("DexClassLoader for file name '%s' librarySearchPath = '%s'", dexFileName.c_str(), librarySearchPath.c_str());

	/* Unzip the apk, extract the native lib and copy it to a readable folder */
	if (!nativeLibFileName.empty()) {
		bool bNativeLibFound = false;
		
		COMPV_CHECK_EXP_RETURN(CompVBase::CPU_ABI().empty(), COMPV_ERROR_CODE_E_SYSTEM);
		const std::string nativeLibSourceFilePath = std::string("lib/") + CompVBase::CPU_ABI() + std::string("/") + nativeLibFileName;
		COMPV_DEBUG_INFO("DexClassLoader trying to extract native lib from '%s'", nativeLibSourceFilePath.c_str());
		// jobjectBufferInputStream.reset()
		jEnv->CallVoidMethod(jobjectBufferInputStream, s_MethodBufferedInputStreamReset);
		COMPV_jni_checkException(jEnv, &bExcOccured);
		COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
		// jobjectZipInputStream = new ZipInputStream(jobjectBufferInputStream);
		jobjectZipInputStream = jEnv->NewObject(s_ClassZipInputStream, s_MethodZipInputStreamConstructor, jobjectBufferInputStream);
		COMPV_CHECK_EXP_BAIL(!jobjectZipInputStream, (err = COMPV_ERROR_CODE_E_JNI));
		// while ((jobjectZipEntry = jobjectZipInputStream.getNextEntry()) != null)
		while (!bNativeLibFound && (jobjectZipEntry = jEnv->CallObjectMethod(jobjectZipInputStream, s_MethodZipInputStreamGetNextEntry))) {
			// if (jobjectZipEntry.getName().equals(nativeLibFileName))
			jstringZipEntryName = reinterpret_cast<jstring>(jEnv->CallObjectMethod(jobjectZipEntry, s_MethodZipEntryGetName));
			COMPV_CHECK_EXP_BAIL(!jstringZipEntryName, (err = COMPV_ERROR_CODE_E_JNI));
			if (CompVJNI::toString(jEnv, jstringZipEntryName).compare(nativeLibSourceFilePath) == 0) {
				const std::string nativeLibDestFilePath = librarySearchPath + "/" + nativeLibFileName;
				jobjectTemp = reinterpret_cast<jstring>(jEnv->NewStringUTF(nativeLibDestFilePath.c_str()));
				COMPV_CHECK_EXP_BAIL(!jobjectTemp, (err = COMPV_ERROR_CODE_E_JNI));
				// jobjectOutputStream = new FileOutputStream(nativeLibDestFilePath);
				jobjectOutputStream = jEnv->NewObject(s_ClassFileOutputStream, s_MethodFileOutputStreamConstructor2, jobjectTemp);
				COMPV_CHECK_EXP_BAIL(!jobjectOutputStream, (err = COMPV_ERROR_CODE_E_JNI));
				COMPV_jni_DeleteLocalRef(jEnv, jobjectTemp);
				COMPV_DEBUG_INFO("DexClassLoader trying to unzip native lib to '%s'", nativeLibDestFilePath.c_str());
				bNativeLibFound = true;
				// while ((len = jobjectZipInputStream.read(buffer, 0, kMaxBufferSize)) > 0)
				total = 0;
				while ((len = jEnv->CallIntMethod(jobjectZipInputStream, s_MethodZipInputStreamRead, jbyteArrayBuff, static_cast<jint>(0), kMaxBufferSize)) > 0) {
					// jobjectOutputStream.write(buffer, 0, len);
					jEnv->CallVoidMethod(jobjectOutputStream, s_MethodFileOutputStreamWrite, jbyteArrayBuff, static_cast<jint>(0), len);
					COMPV_jni_checkException(jEnv, &bExcOccured);
					COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
					total += len;
				}
				COMPV_jni_checkException(jEnv, &bExcOccured);
				COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
				COMPV_DEBUG_INFO("DexClassLoader copied %d bytes for file '%s'", total, nativeLibSourceFilePath.c_str());
			}
			COMPV_jni_DeleteLocalRef(jEnv, jobjectZipEntry);
			COMPV_jni_DeleteLocalRef(jEnv, jstringZipEntryName);
			COMPV_jni_DeleteLocalRef(jEnv, jobjectOutputStream);
		}
		COMPV_jni_checkException(jEnv, &bExcOccured);
		COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
		COMPV_CHECK_EXP_BAIL(!bNativeLibFound, (err = COMPV_ERROR_CODE_E_FILE_NOT_FOUND));
	}

	// jobjectBufferInputStream.close();
	jEnv->CallVoidMethod(jobjectBufferInputStream, s_MethodBufferedInputStreamClose);
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));

bail:
	// Goto bail probably called because of an exception (such as NoSuchMethodException when method GetMethodID called) -> clear exception
	COMPV_jni_checkException1(jEnv);

	COMPV_jni_DeleteLocalRef(jEnv, jstringDirName);
	COMPV_jni_DeleteLocalRef(jEnv, jstringDexFileName);
	COMPV_jni_DeleteLocalRef(jEnv, jstringZipEntryName);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectDirFile);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectDexInternalStorageFile);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectBufferInputStream);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectOutputStream);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectAssets);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectTemp);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectZipInputStream);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectZipEntry);
	COMPV_jni_DeleteLocalRef(jEnv, jbyteArrayBuff);
	
	return err;
}

// 'instance' must be freed using DeleteGlobalRef()
// Important: The instance's constructor *must* be parameterless
COMPV_ERROR_CODE CompVAndroidDexClassLoader::newInstance(JNIEnv* jEnv, const std::string& className, const std::string& dexPath, const std::string& optimizedDirectory, const std::string& librarySearchPath, jobject *instance)
{
	COMPV_CHECK_EXP_RETURN(!jEnv || className.empty() || dexPath.empty() || optimizedDirectory.empty() || librarySearchPath.empty() || !instance, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVAndroidDexClassLoader::isInitialized(), COMPV_ERROR_CODE_E_INVALID_STATE);

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	jobject jobjectClassLoader = NULL;
	jobject jobjectClass = NULL;
	jobject jobjectInstance = NULL;
	jmethodID jmethodIDConstructor = NULL;
	jstring jstringClassName = NULL;
	jstring jstringDexPath = NULL;
	jstring jstringOptimizedDirectory = NULL;
	jstring jstringLibrarySearchPath = NULL;

	*instance = NULL;
	
	jstringClassName = jEnv->NewStringUTF(className.c_str());
	COMPV_CHECK_EXP_BAIL(!jstringClassName, (err = COMPV_ERROR_CODE_E_JNI));
	jstringDexPath = jEnv->NewStringUTF(dexPath.c_str());
	COMPV_CHECK_EXP_BAIL(!jstringDexPath, (err = COMPV_ERROR_CODE_E_JNI));
	jstringOptimizedDirectory = jEnv->NewStringUTF(optimizedDirectory.c_str());
	COMPV_CHECK_EXP_BAIL(!jstringOptimizedDirectory, (err = COMPV_ERROR_CODE_E_JNI));
	jstringLibrarySearchPath = jEnv->NewStringUTF(librarySearchPath.c_str());
	COMPV_CHECK_EXP_BAIL(!jstringLibrarySearchPath, (err = COMPV_ERROR_CODE_E_JNI));

	// Create the dex class loader object
	// https://developer.android.com/reference/dalvik/system/DexClassLoader.html#DexClassLoader(java.lang.String, java.lang.String, java.lang.String, java.lang.ClassLoader)
	COMPV_DEBUG_INFO("new DexClassLoader(%s, %s, %s, %p)", dexPath.c_str(), optimizedDirectory.c_str(), librarySearchPath.c_str(), s_ObjectLoaderParent);
	jobjectClassLoader = jEnv->NewObject(s_ClassDexLoader, s_MethodDexLoaderConstructor, jstringDexPath, jstringOptimizedDirectory, jstringLibrarySearchPath, s_ObjectLoaderParent);
	COMPV_CHECK_EXP_BAIL(!jstringLibrarySearchPath, (err = COMPV_ERROR_CODE_E_JNI));

	// load the class
	// https://developer.android.com/reference/java/lang/ClassLoader.html#loadClass(java.lang.String)
	COMPV_DEBUG_INFO("loadClass(%s)", className.c_str());
	jobjectClass = jEnv->CallObjectMethod(jobjectClassLoader, s_MethodLoadClass, jstringClassName);
	COMPV_CHECK_EXP_BAIL(!jstringLibrarySearchPath, (err = COMPV_ERROR_CODE_E_JNI));

	// newInstance
	jmethodIDConstructor = jEnv->GetMethodID(reinterpret_cast<jclass>(jobjectClass), "<init>", "()V");
	COMPV_CHECK_EXP_BAIL(!jmethodIDConstructor, (err = COMPV_ERROR_CODE_E_JNI));
	jobjectInstance = jEnv->NewObject(reinterpret_cast<jclass>(jobjectClass), jmethodIDConstructor);
	COMPV_CHECK_EXP_BAIL(!jobjectInstance, (err = COMPV_ERROR_CODE_E_JNI));
	*instance = jEnv->NewGlobalRef(jobjectInstance);
	COMPV_CHECK_EXP_BAIL(!*instance, (err = COMPV_ERROR_CODE_E_JNI));

bail:
	// Goto bail probably called because of an exception (such as NoSuchMethodException when method GetMethodID called) -> clear exception
	COMPV_jni_checkException1(jEnv);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectClassLoader);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectClass)
	COMPV_jni_DeleteLocalRef(jEnv, jstringClassName);
	COMPV_jni_DeleteLocalRef(jEnv, jstringDexPath);
	COMPV_jni_DeleteLocalRef(jEnv, jstringOptimizedDirectory);
	COMPV_jni_DeleteLocalRef(jEnv, jstringLibrarySearchPath);
	
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		if (*instance) {
			COMPV_jni_DeleteGlobalRef(jEnv, *instance);
		}
	}
	return err;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID */