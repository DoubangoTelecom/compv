/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#include "compv/base/android/compv_android_native_activity.h"

#if COMPV_OS_ANDROID
#include "compv/base/compv_debug.h"
#include "compv/base/android/compv_android_dexclassloader.h"

#include <android/log.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#define kModuleNameAndroidNativeActivity "Android Native Activity"

static ANativeActivity* aNativeActivity = NULL;
static struct android_app* androidApp = NULL;

static void free_saved_state(struct android_app* android_app)
{
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->savedState != NULL) {
        free(android_app->savedState), android_app->savedState = NULL;
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&android_app->mutex);
}

int8_t android_app_read_cmd(struct android_app* android_app)
{
    int8_t cmd;
    if (read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        switch (cmd) {
        case APP_CMD_SAVE_STATE:
            free_saved_state(android_app);
            break;
        }
        return cmd;
    }
    else {
        COMPV_DEBUG_ERROR_EX(kModuleNameAndroidNativeActivity, "No data on command pipe!");
    }
    return -1;
}

static void print_cur_config(struct android_app* android_app)
{
    char lang[2], country[2];
    AConfiguration_getLanguage(android_app->config, lang);
    AConfiguration_getCountry(android_app->config, country);

    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
                        "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
                        "modetype=%d modenight=%d",
                        AConfiguration_getMcc(android_app->config),
                        AConfiguration_getMnc(android_app->config),
                        lang[0], lang[1], country[0], country[1],
                        AConfiguration_getOrientation(android_app->config),
                        AConfiguration_getTouchscreen(android_app->config),
                        AConfiguration_getDensity(android_app->config),
                        AConfiguration_getKeyboard(android_app->config),
                        AConfiguration_getNavigation(android_app->config),
                        AConfiguration_getKeysHidden(android_app->config),
                        AConfiguration_getNavHidden(android_app->config),
                        AConfiguration_getSdkVersion(android_app->config),
                        AConfiguration_getScreenSize(android_app->config),
                        AConfiguration_getScreenLong(android_app->config),
                        AConfiguration_getUiModeType(android_app->config),
                        AConfiguration_getUiModeNight(android_app->config));
}

void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd)
{
    switch (cmd) {
    case APP_CMD_INPUT_CHANGED:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_INPUT_CHANGED");
        pthread_mutex_lock(&android_app->mutex);
        if (android_app->inputQueue != NULL) {
            AInputQueue_detachLooper(android_app->inputQueue);
        }
        android_app->inputQueue = android_app->pendingInputQueue;
        if (android_app->inputQueue != NULL) {
            COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Attaching input queue to looper");
            AInputQueue_attachLooper(android_app->inputQueue,
                                     android_app->looper, LOOPER_ID_INPUT, NULL,
                                     &android_app->inputPollSource);
        }
        pthread_cond_broadcast(&android_app->cond);
        pthread_mutex_unlock(&android_app->mutex);
        break;

    case APP_CMD_INIT_WINDOW:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_INIT_WINDOW");
        pthread_mutex_lock(&android_app->mutex);
        android_app->window = android_app->pendingWindow;
        // TODO(dmi): move to Drawing::Init()
		// If 'ANativeWindow_setBuffersGeometry' fails we can call 'eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)' to retrieve the default supported format.
        COMPV_CHECK_EXP_NOP(ANativeWindow_setBuffersGeometry(android_app->window, 0, 0, WINDOW_FORMAT_RGBA_8888) != 0, COMPV_NAMESPACE::COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT, "Cannot set the window format to RGBA_8888");
        pthread_cond_broadcast(&android_app->cond);
        pthread_mutex_unlock(&android_app->mutex);
        break;

    case APP_CMD_TERM_WINDOW:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_TERM_WINDOW");
        pthread_cond_broadcast(&android_app->cond);
        break;

    case APP_CMD_RESUME:
    case APP_CMD_START:
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "activityState=%d", cmd);
        pthread_mutex_lock(&android_app->mutex);
        android_app->activityState = cmd;
        pthread_cond_broadcast(&android_app->cond);
        pthread_mutex_unlock(&android_app->mutex);
        break;

    case APP_CMD_CONFIG_CHANGED:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_CONFIG_CHANGED");
        AConfiguration_fromAssetManager(android_app->config,
                                        android_app->activity->assetManager);
        print_cur_config(android_app);
        break;

    case APP_CMD_DESTROY:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_DESTROY");
        android_app->destroyRequested = 1;
        break;
    }
}

void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd)
{
    switch (cmd) {
    case APP_CMD_TERM_WINDOW:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_TERM_WINDOW");
        pthread_mutex_lock(&android_app->mutex);
        android_app->window = NULL;
        pthread_cond_broadcast(&android_app->cond);
        pthread_mutex_unlock(&android_app->mutex);
        break;

    case APP_CMD_SAVE_STATE:
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "APP_CMD_SAVE_STATE");
        pthread_mutex_lock(&android_app->mutex);
        android_app->stateSaved = 1;
        pthread_cond_broadcast(&android_app->cond);
        pthread_mutex_unlock(&android_app->mutex);
        break;

    case APP_CMD_RESUME:
        free_saved_state(android_app);
        break;
    }
}

static void android_app_destroy(struct android_app* android_app)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "android_app_destroy!");
    free_saved_state(android_app);
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->inputQueue != NULL) {
        AInputQueue_detachLooper(android_app->inputQueue);
    }
    AConfiguration_delete(android_app->config);
    android_app->destroyed = 1;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);
    // Can't touch android_app object after this.
}

static void process_input(struct android_app* app, struct android_poll_source* source)
{
    AInputEvent* event = NULL;
    while (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
        COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "New input event: type=%d", AInputEvent_getType(event));
        if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
            continue;
        }
        int32_t handled = 0;
        if (app->onInputEvent != NULL) {
            handled = app->onInputEvent(app, event);
        }
        AInputQueue_finishEvent(app->inputQueue, event, handled);
    }
}

static void process_cmd(struct android_app* app, struct android_poll_source* source)
{
    int8_t cmd = android_app_read_cmd(app);
    android_app_pre_exec_cmd(app, cmd);
    if (app->onAppCmd != NULL) {
        app->onAppCmd(app, cmd);
    }
    android_app_post_exec_cmd(app, cmd);
}

static void* android_app_entry(void* param)
{
    struct android_app* android_app = (struct android_app*)param;

    android_app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);

    print_cur_config(android_app);

    android_app->cmdPollSource.id = LOOPER_ID_MAIN;
    android_app->cmdPollSource.app = android_app;
    android_app->cmdPollSource.process = process_cmd;
    android_app->inputPollSource.id = LOOPER_ID_INPUT;
    android_app->inputPollSource.app = android_app;
    android_app->inputPollSource.process = process_input;

    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, android_app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL,
                  &android_app->cmdPollSource);
    android_app->looper = looper;

    pthread_mutex_lock(&android_app->mutex);
    android_app->running = 1;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);

	if (android_app->android_main) {
		android_app->android_main(android_app);
	}
	else {
		COMPV_DEBUG_FATAL_EX(kModuleNameAndroidNativeActivity, "No pointer function defined for android_main");
	}

    android_app_destroy(android_app);
    return NULL;
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

static struct android_app* android_app_create(ANativeActivity* activity,
        void* savedState, size_t savedStateSize, void(*android_main)(struct android_app* app))
{
	// Important: do not use CompVMem::malloc to make sure CompVMem::isEmpty check will be ok before exiting the app. No memory leak, free will be call by onDestroy
    androidApp = static_cast<struct android_app*>(malloc(sizeof(struct android_app)));
    memset(androidApp, 0, sizeof(struct android_app));
    androidApp->activity = activity;
	androidApp->android_main = android_main;

    pthread_mutex_init(&androidApp->mutex, NULL);
    pthread_cond_init(&androidApp->cond, NULL);

    if (savedState != NULL) {
        androidApp->savedState = malloc(savedStateSize);
        androidApp->savedStateSize = savedStateSize;
        memcpy(androidApp->savedState, savedState, savedStateSize);
    }

    int msgpipe[2];
    if (pipe(msgpipe)) {
        COMPV_DEBUG_ERROR_EX(kModuleNameAndroidNativeActivity, "could not create pipe: %s", strerror(errno));
        return NULL;
    }
    androidApp->msgread = msgpipe[0];
    androidApp->msgwrite = msgpipe[1];

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&androidApp->thread, &attr, android_app_entry, androidApp);

    // Wait for thread to start.
    pthread_mutex_lock(&androidApp->mutex);
    while (!androidApp->running) {
        pthread_cond_wait(&androidApp->cond, &androidApp->mutex);
    }
    pthread_mutex_unlock(&androidApp->mutex);

    return androidApp;
}

static void android_app_write_cmd(struct android_app* android_app, int8_t cmd)
{
    if (write(android_app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        COMPV_DEBUG_ERROR_EX(kModuleNameAndroidNativeActivity, "Failure writing android_app cmd: %s", strerror(errno));
    }
}

static void android_app_set_input(struct android_app* android_app, AInputQueue* inputQueue)
{
    pthread_mutex_lock(&android_app->mutex);
    android_app->pendingInputQueue = inputQueue;
    android_app_write_cmd(android_app, APP_CMD_INPUT_CHANGED);
    while (android_app->inputQueue != android_app->pendingInputQueue) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_window(struct android_app* android_app, ANativeWindow* window)
{
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->pendingWindow != NULL) {
        android_app_write_cmd(android_app, APP_CMD_TERM_WINDOW);
    }
    android_app->pendingWindow = window;
    if (window != NULL) {
        android_app_write_cmd(android_app, APP_CMD_INIT_WINDOW);
    }
    while (android_app->window != android_app->pendingWindow) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_activity_state(struct android_app* android_app, int8_t cmd)
{
    pthread_mutex_lock(&android_app->mutex);
    android_app_write_cmd(android_app, cmd);
    while (android_app->activityState != cmd) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_free(struct android_app* android_app)
{
    pthread_mutex_lock(&android_app->mutex);
    android_app_write_cmd(android_app, APP_CMD_DESTROY);
    while (!android_app->destroyed) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);

    close(android_app->msgread);
    close(android_app->msgwrite);
    pthread_cond_destroy(&android_app->cond);
    pthread_mutex_destroy(&android_app->mutex);
    if (android_app) free(android_app), android_app = NULL;
    androidApp = NULL;
}

static void onDestroy(ANativeActivity* activity)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Destroy: %p", activity);
	COMPV_CHECK_CODE_NOP(COMPV_NAMESPACE::CompVAndroidDexClassLoader::deInit(activity->env)); // Important: "activity->env" valid on MainThread only
    android_app_free((struct android_app*)activity->instance);
}

static void onStart(ANativeActivity* activity)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Start: %p", activity);
	COMPV_CHECK_CODE_NOP(COMPV_NAMESPACE::CompVAndroidDexClassLoader::init(activity->env)); // Important: "activity->env" valid on MainThread only
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_START);
}

static void onResume(ANativeActivity* activity)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Resume: %p", activity);
	COMPV_CHECK_CODE_NOP(COMPV_NAMESPACE::CompVAndroidDexClassLoader::init(activity->env)); // Important: "activity->env" valid on MainThread only
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_RESUME);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen)
{
    struct android_app* android_app = (struct android_app*)activity->instance;
    void* savedState = NULL;

    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "SaveInstanceState: %p", activity);
    pthread_mutex_lock(&android_app->mutex);
    android_app->stateSaved = 0;
    android_app_write_cmd(android_app, APP_CMD_SAVE_STATE);
    while (!android_app->stateSaved) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }

    if (android_app->savedState != NULL) {
        savedState = android_app->savedState;
        *outLen = android_app->savedStateSize;
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }

    pthread_mutex_unlock(&android_app->mutex);

    return savedState;
}

static void onPause(ANativeActivity* activity)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Pause: %p", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Stop: %p", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged(ANativeActivity* activity)
{
    struct android_app* android_app = (struct android_app*)activity->instance;
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "ConfigurationChanged: %p", activity);
    android_app_write_cmd(android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity)
{
    struct android_app* android_app = (struct android_app*)activity->instance;
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "LowMemory: %p", activity);
    android_app_write_cmd(android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "WindowFocusChanged: %p -- %d", activity, focused);
    android_app_write_cmd((struct android_app*)activity->instance,
                          focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "NativeWindowCreated: %p -- %p", activity, window);
    android_app_set_window((struct android_app*)activity->instance, window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "NativeWindowDestroyed: %p -- %p", activity, window);
    android_app_set_window((struct android_app*)activity->instance, NULL);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "InputQueueCreated: %p -- %p", activity, queue);
    android_app_set_input((struct android_app*)activity->instance, queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)
{
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "InputQueueDestroyed: %p -- %p", activity, queue);
    android_app_set_input((struct android_app*)activity->instance, NULL);
}

struct android_app* AndroidApp_get()
{
    return androidApp;
}

ANativeActivity* ANativeActivity_get()
{
    return aNativeActivity;
}

void ANativeActivity_onCreatePriv(ANativeActivity* activity, void* savedState, size_t savedStateSize, void(*android_main)(struct android_app* app))
{
    aNativeActivity = activity;
    COMPV_DEBUG_INFO_EX(kModuleNameAndroidNativeActivity, "Creating: %p", activity);
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

    activity->instance = android_app_create(activity, savedState, savedStateSize, android_main);
}

#endif /* COMPV_OS_ANDROID */
