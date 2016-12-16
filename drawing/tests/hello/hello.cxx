#include <compv/compv_api.h>

using namespace compv;

CompVWindowPtr window;
CompVCameraPtr camera;

static void* COMPV_STDCALL WorkerThread(void* arg);

compv_main()
{
    COMPV_ERROR_CODE err;

    // Change debug level to INFO before starting
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

    // Init the modules
    COMPV_CHECK_CODE_BAIL(err = CompVInit());

    // Create "Hello world!" window
    COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, 670, 580, "Hello world!"));

    // Start ui runloop
    COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(WorkerThread));

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_DEBUG_ERROR("Something went wrong!!");
    }

	camera = NULL;
    window = NULL;

    // DeInit the modules
    COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
    // Make sure we freed all allocated memory
    COMPV_ASSERT(CompVMem::isEmpty());
    // Make sure we freed all allocated objects
    COMPV_ASSERT(CompVObj::isEmpty());

    compv_main_return(0);
}

class MyCameraListener;
typedef CompVPtr<MyCameraListener*> MyCameraListenerPtr;
typedef MyCameraListenerPtr* MyCameraListenerPtrPtr;
class MyCameraListener : public CompVCameraListener
{
protected:
	MyCameraListener(CompVSingleSurfaceLayerPtr ptrSingleSurfaceLayer)
		: m_ptrSingleSurfaceLayer(ptrSingleSurfaceLayer)
	{ 
	}
public:
	virtual ~MyCameraListener()
	{  
	}
	virtual COMPV_ERROR_CODE onNewFrame(const CompVMatPtr& image) override
	{
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		//static char buff_[33];
		//static int count;
		if (CompVDrawing::isLoopRunning()) {
			//snprintf(buff_, sizeof(buff_), "%d", ++count);
			//std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
			COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->drawImage(image));
			//COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->renderer()->canvas()->drawText(text.c_str(), text.length(), 463, 86));
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->blit());
			COMPV_CHECK_CODE_BAIL(err = window->endDraw());
		}
	bail:
		return err;
	}

	static COMPV_ERROR_CODE newObj(MyCameraListenerPtrPtr listener, CompVSingleSurfaceLayerPtr ptrSingleSurfaceLayer)
	{
		COMPV_CHECK_EXP_RETURN(!listener || !ptrSingleSurfaceLayer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		MyCameraListenerPtr listener_ = new MyCameraListener(ptrSingleSurfaceLayer);
		COMPV_CHECK_EXP_RETURN(!listener_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

		*listener = listener_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	CompVSingleSurfaceLayerPtr m_ptrSingleSurfaceLayer;
};

static void* COMPV_STDCALL WorkerThread(void* arg)
{
#if 0 // Chroma conversion
	CompVMatPtr image;
	COMPV_ERROR_CODE err;
	CompVSingleSurfaceLayerPtr singleSurfaceLayer;
	static int count = 0;
	static uint64_t timeStart;
	
	// FIXME: add support for RGB565 and BGR565, both LE and BE, LE being the default ones (ARM and X86 devices are LE by default)
	
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV444P, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_yuv444p.yuv", &image));
	COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV422P, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_yuv422p.yuv", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV420P, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_yuv420p.yuv", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_gray.yuv", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_UYVY422, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_uyvy422.yuv", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUYV422, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_yuyv422.yuv", &image)); // DirectShow / MediaFoundation
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_NV12, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_nv12.yuv", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_NV21, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_nv21.yuv", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGBA32, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_rgba.rgb", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_ARGB32, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_argb.rgb", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGRA32, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_bgra.rgb", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGR24, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_bgr.rgb", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB565LE, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_rgb565le.rgb", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB565BE, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_rgb565be.rgb", &image));
	// COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGR565LE, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_bgr565le.rgb", &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGR565BE, 750, 472, 750, "C:/Projects/GitHub/data/colorspace/girl_750x501x750_bgr565be.rgb", &image));

	COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));
	timeStart = CompVTime::getNowMills();
	while (CompVDrawing::isLoopRunning()) {
		COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->drawImage(image));
		
		if (count == 1000) {
			uint64_t duration = (CompVTime::getNowMills() - timeStart);
			float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
			COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
			count = 0;
			timeStart = CompVTime::getNowMills();
		}
		++count;

		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
		COMPV_CHECK_CODE_BAIL(err = window->endDraw());
	}

bail:
	return NULL;

#elif 1 // Camera

#if COMPV_OS_ANDROID && 0
	{
		static JNINativeMethod android_hardware_Camera_NativeMethods[] =
		{
			{ "getNumberOfCameras", "()I", NULL/*FIXME: function pointer*/ }
		};

		// FIXME: DeleteLocalRef for all

		struct android_app* androidApp = AndroidApp_get();
		JavaVM* jVM = androidApp ? (androidApp->activity ? androidApp->activity->vm : NULL) : NULL;
		JNIEnv* jEnv = NULL;
		jthrowable exc;
		if (jVM->AttachCurrentThread(&jEnv, NULL) == JNI_OK) {
			jclass class_Camera = jEnv->FindClass("android/hardware/Camera");
			jclass class_Camera_PreviewCallback = jEnv->FindClass("android/hardware/Camera$PreviewCallback");

			jobject parent = NULL;
			jclass klass = jEnv->FindClass("java/lang/Class");
			jmethodID get_class_loader = jEnv->GetMethodID(klass, "getClassLoader", "()Ljava/lang/ClassLoader;");
			if (get_class_loader) {
				parent = jEnv->CallObjectMethod(klass, get_class_loader);
			}

			jclass class_dex_loader = jEnv->FindClass("dalvik/system/DexClassLoader");
			jmethodID constructor = jEnv->GetMethodID(class_dex_loader, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
			jmethodID load_class = jEnv->GetMethodID(class_dex_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

			jstring optimized_directory = jEnv->NewStringUTF(androidApp->activity->internalDataPath); // FIXME: free
			jstring dexPathJ = jEnv->NewStringUTF("/data/user/0/com.hello/files/CameraCallback.apk"); // FIXME: free
			jobject dexloader = jEnv->NewObject(class_dex_loader, constructor, dexPathJ, optimized_directory, NULL, parent);
			exc = jEnv->ExceptionOccurred();
			if (exc) {
				jEnv->ExceptionDescribe();
				jEnv->ExceptionClear();
			}

			jstring callback_class_name = jEnv->NewStringUTF("org/doubango/camera/Callback");
			jobject temp = jEnv->CallObjectMethod(dexloader, load_class, callback_class_name);
			exc = jEnv->ExceptionOccurred();
			if (exc) {
				jEnv->ExceptionDescribe();
				jboolean isCopy = false;
				jclass class_object = jEnv->FindClass("java/lang/Object");
				jmethodID toString = jEnv->GetMethodID(class_object, "toString", "()Ljava/lang/String;");
				jstring s = (jstring)jEnv->CallObjectMethod(exc, toString);
				const char* utf = jEnv->GetStringUTFChars(s, &isCopy);
				
				jEnv->ExceptionClear();
			}

			COMPV_DEBUG_INFO("created dex loader = %p", dexloader);
			
			jEnv->DeleteLocalRef(klass);

			//jmethodID testClassCtor = jEnv->GetMethodID(class_Camera_PreviewCallback, "<init>", "(Landroid/hardware/Camera;)V");

			//jobject testClassInstance = jEnv->NewObject(class_Camera_PreviewCallback, testClassCtor);
			

			// FIXME: use RegisterNatives
			// jEnv->RegisterNatives(class_Camera, android_hardware_Camera_NativeMethods, sizeof(android_hardware_Camera_NativeMethods) / sizeof(android_hardware_Camera_NativeMethods[0]));

			jmethodID methodID_static_Camera_getNumberOfCameras = jEnv->GetStaticMethodID(class_Camera, "getNumberOfCameras", "()I");
			jmethodID methodID_static_Camera_open0 = jEnv->GetStaticMethodID(class_Camera, "open", "()Landroid/hardware/Camera;"); // Camera = Camera.open() -> https://developer.android.com/reference/android/hardware/Camera.html#open()
			jmethodID methodID_static_Camera_open1 = jEnv->GetStaticMethodID(class_Camera, "open", "(I)Landroid/hardware/Camera;"); // Camera = Camera.open(int cameraId) -> https://developer.android.com/reference/android/hardware/Camera.html#open(int)
			jmethodID methodID_member_Camera_release = jEnv->GetMethodID(class_Camera, "release", "()V"); // void = mCamera.release() -> https://developer.android.com/reference/android/hardware/Camera.html#release()
			jmethodID methodID_member_Camera_lock = jEnv->GetMethodID(class_Camera, "lock", "()V"); // void = mCamera.lock() -> https://developer.android.com/reference/android/hardware/Camera.html#lock()
			jmethodID methodID_member_Camera_unlock = jEnv->GetMethodID(class_Camera, "release", "()V"); // void = mCamera.unlock() -> https://developer.android.com/reference/android/hardware/Camera.html#unlock()
			jmethodID methodID_member_Camera_addCallbackBuffer = jEnv->GetMethodID(class_Camera, "addCallbackBuffer", "([B)V"); // void = mCamera.addCallbackBuffer(byte[]) -> https://developer.android.com/reference/android/hardware/Camera.html#addCallbackBuffer(byte[])
			jmethodID methodID_member_Camera_setPreviewCallbackWithBuffer = jEnv->GetMethodID(class_Camera, "setPreviewCallbackWithBuffer", "(Landroid/hardware/Camera$PreviewCallback;)V"); // void = mCamera.setPreviewCallbackWithBuffer(Camera.PreviewCallback cb) -> https://developer.android.com/reference/android/hardware/Camera.html#setPreviewCallbackWithBuffer(android.hardware.Camera.PreviewCallback)
			jmethodID methodID_member_Camera_getParameters = jEnv->GetMethodID(class_Camera, "getParameters", "()Landroid/hardware/Camera$Parameters;"); // Parameters = mCamera.getParameters() -> https://developer.android.com/reference/android/hardware/Camera.html#getParameters()

			

			int numberOfCameras = jEnv->CallStaticIntMethod(class_Camera, methodID_static_Camera_getNumberOfCameras);
			if (numberOfCameras > 0) {
				// Default camera -> use methodID_static_Camera_open0()
				// Front camera -> use methodID_static_Camera_open1(numberOfCameras - 1)
				jobject object_Camera = jEnv->CallStaticObjectMethod(class_Camera, methodID_static_Camera_open1, (numberOfCameras - 1));
				if (!object_Camera) {
					jobject object_parameters = jEnv->CallObjectMethod(object_Camera, methodID_member_Camera_getParameters);
					//parameters.setPreviewFormat(PixelFormat.YCbCr_420_SP);
					//parameters.setPreviewFrameRate(framerate);
					//parameters.setPictureSize(width, height);
					//mCamera.setParameters(parameters);
					
					

					jEnv->CallVoidMethod(object_Camera, methodID_member_Camera_release);
				}
			}


			jVM->DetachCurrentThread();
		}

	}
#endif

    COMPV_ERROR_CODE err;
    CompVCameraDeviceInfoList devices;
	CompVSingleSurfaceLayerPtr singleSurfaceLayer;
	MyCameraListenerPtr listener;
	
	COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));
	COMPV_CHECK_CODE_BAIL(err = MyCameraListener::newObj(&listener, *singleSurfaceLayer));
    COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));
	COMPV_CHECK_CODE_BAIL(err = camera->setListener(*listener));
    COMPV_CHECK_CODE_BAIL(err = camera->devices(devices));
    for (CompVCameraDeviceInfoList::iterator it = devices.begin(); it != devices.end(); ++it) {
        COMPV_DEBUG_INFO("Camera device: %s -> %s, %s", it->id.c_str(), it->name.c_str(), it->description.c_str());
    }
    COMPV_CHECK_CODE_BAIL(err = camera->start(devices[0].id));

    //while (CompVDrawing::isLoopRunning()) {
        //CompVThread::sleep(1); // FIXME
    //}

bail:
    return NULL;
#elif 0 // Multiple
    COMPV_ERROR_CODE err;
    CompVMatPtr mat[3];
    CompVMultiSurfaceLayerPtr multipleSurfaceLayer;
    CompVSurfacePtr surfaces[3];
    CompVViewportPtr ptrViewPort;
    CompVMVPPtr ptrMVP;
    static int count = 0;
    static uint64_t timeStart;
    char buff_[33] = { 0 };

    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

    COMPV_CHECK_CODE_BAIL(err = window->addMultiLayerSurface(&multipleSurfaceLayer));
    COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->addSurface(&surfaces[0], window->width(), window->height()));
    COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->addSurface(&surfaces[1], window->width(), window->height()));
    COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->addSurface(&surfaces[2], window->width(), window->height()));

    COMPV_CHECK_CODE_BAIL(err = surfaces[0]->viewport()->reset(CompViewportSizeFlags::makeStatic(), 0, 0, 120, 120));
    COMPV_CHECK_CODE_BAIL(err = surfaces[1]->viewport()->reset(CompViewportSizeFlags::makeStatic(), 120, 0, 120, 120));
    COMPV_CHECK_CODE_BAIL(err = surfaces[2]->viewport()->reset(CompViewportSizeFlags::makeStatic(), 0, 120, 120, 120));

    timeStart = CompVTime::getNowMills();
    while (CompVDrawing::isLoopRunning()) {
        snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
        std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
        COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
        //COMPV_CHECK_CODE_BAIL(err = surfaces[0]->drawText(text.c_str(), text.length(), 0, 0));
        COMPV_CHECK_CODE_BAIL(err = surfaces[0]->drawImage(mat[0/*(count + 0) % 3*/]));
        COMPV_CHECK_CODE_BAIL(err = surfaces[1]->drawImage(mat[1/*(count + 0) % 3*/]));
        COMPV_CHECK_CODE_BAIL(err = surfaces[2]->drawImage(mat[2/*(count + 0) % 3*/]));

        if (count == 1000) {
            uint64_t duration = (CompVTime::getNowMills() - timeStart);
            float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
            COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
            count = 0;
            timeStart = CompVTime::getNowMills();
        }
        ++count;

        COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->blit());
        COMPV_CHECK_CODE_BAIL(err = window->endDraw());
    }

bail:
    return NULL;

#elif 1 // Matching
    COMPV_ERROR_CODE err;
    CompVMatPtr mat[3];
    CompVMatchingSurfaceLayerPtr matchingSurfaceLayer;
    static int count = 0;
    static uint64_t timeStart;
    char buff_[33] = { 0 };

    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

    COMPV_CHECK_CODE_BAIL(err = window->addMatchingLayerSurface(&matchingSurfaceLayer));

    timeStart = CompVTime::getNowMills();
    while (CompVDrawing::isLoopRunning()) {
        snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
        std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
        COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
        COMPV_CHECK_CODE_BAIL(err = matchingSurfaceLayer->drawMatches(mat[0/*(count + 0) % 3*/], mat[1/*(count + 0) % 3*/]));

        if (count == 1000) {
            uint64_t duration = (CompVTime::getNowMills() - timeStart);
            float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
            COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
            count = 0;
            timeStart = CompVTime::getNowMills();
        }
        ++count;

        COMPV_CHECK_CODE_BAIL(err = matchingSurfaceLayer->blit());
        COMPV_CHECK_CODE_BAIL(err = window->endDraw());
    }

bail:
    return NULL;
#elif 1 // Single
    COMPV_ERROR_CODE err;
    CompVMatPtr mat[3];
    CompVSingleSurfaceLayerPtr singleSurfaceLayer;
    CompVViewportPtr ptrViewPort;
    CompVMVPPtr ptrMVP;
    static int count = 0;
    static uint64_t timeStart;
    char buff_[33] = { 0 };

    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
    COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

    COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));
    /*
    // Set viewport
    COMPV_CHECK_CODE_BAIL(err = CompVViewport::newObj(&ptrViewPort, CompViewportSizeFlags::makeDynamicAspectRatio()));
    COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->setViewport(ptrViewPort));

    // Set MVP
    COMPV_CHECK_CODE_BAIL(err = CompVDrawingMVP::newObj2D(&ptrMVP));
    COMPV_CHECK_CODE_BAIL(err = ptrMVP->model()->matrix()->scale(CompVVec3f(1.f, 1.f, 1.f)));
    COMPV_CHECK_CODE_BAIL(err = ptrMVP->view()->setCamera(CompVVec3f(0.f, 0.f, 1.f), CompVVec3f(0.f, 0.f, 0.f), CompVVec3f(0.f, 1.f, 0.f)));
    COMPV_CHECK_CODE_BAIL(err = ptrMVP->projection2D()->setOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f));
    COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->setMVP(ptrMVP));*/

    timeStart = CompVTime::getNowMills();
    while (CompVDrawing::isLoopRunning()) {
        snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
        std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
        COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
        COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->drawImage(mat[0/*(count + 0) % 3*/]));
        //COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->drawText(text.c_str(), text.length(), 463, 86));
        COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->renderer()->canvas()->drawText(text.c_str(), text.length(), 463, 86));

        if (count == 1000) {
            uint64_t duration = (CompVTime::getNowMills() - timeStart);
            float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
            COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
            count = 0;
            timeStart = CompVTime::getNowMills();
        }
        ++count;

        COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
        COMPV_CHECK_CODE_BAIL(err = window->endDraw());

        //COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
        //COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->surface()->drawImage(mat[count % 3])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
        //COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->surface()->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
        //COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);

        ++count;
    }

bail:
    return NULL;
#endif
}
