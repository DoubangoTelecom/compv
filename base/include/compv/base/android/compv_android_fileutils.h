/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_ANDROID_FILEUTILS_H_)
#define _COMPV_BASE_ANDROID_FILEUTILS_H_

#include "compv/base/compv_config.h"

#if COMPV_OS_ANDROID
#include "compv/base/android/compv_android_native_activity.h"
#include "compv/base/compv_debug.h"
#include <errno.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-function")

static bool compv_android_have_assetmgr()
{
    return (ANativeActivity_get() != NULL);
}

static bool compv_android_asset_fexist(const char* pcPath)
{
    if (pcPath) {
        ANativeActivity* activity = ANativeActivity_get();
        if (activity) {
            AAsset* asset = AAssetManager_open(activity->assetManager, pcPath, 0);
            if (asset) {
                AAsset_close(asset);
                return true;
            }
            return false;
        }
    }
    return false;
}

static size_t compv_android_asset_fsize(const char* pcPath)
{
    if (pcPath) {
        ANativeActivity* activity = ANativeActivity_get();
        if (activity) {
            AAsset* asset = AAssetManager_open(activity->assetManager, pcPath, 0);
            if (asset) {
                size_t size = static_cast<size_t>(AAsset_getLength(asset));
                AAsset_close(asset);
                return size;
            }
            return 0;
        }
    }
    return 0;
}

static int compv_android_asset_fread(void* cookie, char* buf, int size)
{
    return AAsset_read((AAsset*)cookie, buf, size);
}

static int compv_android_asset_fwrite(void* cookie, const char* buf, int size)
{
    COMPV_DEBUG_ERROR("Not implemented");
    return EACCES;
}

static fpos_t compv_android_asset_fseek(void* cookie, fpos_t offset, int whence)
{
    return AAsset_seek((AAsset*)cookie, offset, whence);
}

static int compv_android_asset_fclose(void* cookie)
{
    AAsset_close((AAsset*)cookie);
    return 0;
}

static FILE* compv_android_asset_fopen(const char* fname, const char* mode)
{
    if (fname) {
        ANativeActivity* activity = ANativeActivity_get();
        if (activity) {
            AAsset* asset = AAssetManager_open(activity->assetManager, fname, 0);
            if (!asset) {
                return NULL;
            }
            return funopen(asset, compv_android_asset_fread, compv_android_asset_fwrite, compv_android_asset_fseek, compv_android_asset_fclose);
        }
    }
    return NULL;
}

COMPV_GCC_DISABLE_WARNINGS_END()

#endif /* COMPV_OS_ANDROID */

#endif /* _COMPV_BASE_ANDROID_FILEUTILS_H_ */
