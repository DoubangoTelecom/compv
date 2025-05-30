#include <compv/compv_api.h>

using namespace compv;

CompVWindowPtr window;
CompVCameraPtr camera;
CompVCameraDeviceInfoList devices;

#define CAMERA_WIDTH		1280 //640
#define CAMERA_HEIGHT		720 //480
#define CAMERA_FPS			25
#define CAMERA_SUBTYPE		COMPV_SUBTYPE_PIXELS_YUY2

static void* WorkerThread(void* arg);

/* My runloop listener (optional) */
COMPV_OBJECT_DECLARE_PTRS(MyRunLoopListener)
class CompVMyRunLoopListener : public CompVRunLoopListener
{
protected:
	CompVMyRunLoopListener() {}
public:
	virtual ~CompVMyRunLoopListener() {}
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_RUNLOOP_STATE newState) override;
	static COMPV_ERROR_CODE newObj(CompVMyRunLoopListenerPtrPtr listener);
private:
};

compv_main()
{
    COMPV_ERROR_CODE err;
	CompVMyRunLoopListenerPtr listener;

    // Change debug level to INFO before starting
    CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

    // Init the modules
    COMPV_CHECK_CODE_BAIL(err = CompVInit());

    // Create "Hello world!" window
    COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, 670, 580, "Hello world!"));

    // Start ui runloop
	COMPV_CHECK_CODE_BAIL(err = CompVMyRunLoopListener::newObj(&listener));
    COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(*listener));

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
		//static int count;
		if (CompVDrawing::isLoopRunning()) {
			//std::string text = "Hello doubango telecom [" + CompVBase::to_string(count) + "]";
			COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->drawImage(image));
			//COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->renderer()->canvas()->drawText(text.c_str(), text.length(), 463, 86));
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->blit());
			COMPV_CHECK_CODE_BAIL(err = window->endDraw());
		}
	bail:
		return err;
	}
	virtual COMPV_ERROR_CODE onError(const std::string& message) override
	{
		COMPV_DEBUG_ERROR("Camera error: %s", message.c_str()); // probably a disconnect
		return COMPV_ERROR_CODE_S_OK;
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

// FIXME
static void *COMPV_STDCALL cameraRestart(void * arg)
{
	camera->stop();
	COMPV_CHECK_CODE_ASSERT(camera->setInt(COMPV_CAMERA_CAP_INT_WIDTH, 100));
	COMPV_CHECK_CODE_ASSERT(camera->setInt(COMPV_CAMERA_CAP_INT_HEIGHT, 100));
	COMPV_CHECK_CODE_ASSERT(camera->start(devices[0].id));
	return NULL;
}


COMPV_ERROR_CODE CompVMyRunLoopListener::onStateChanged(COMPV_RUNLOOP_STATE newState)
{
	if (newState == COMPV_RUNLOOP_STATE_ANIMATION_RESUMED || newState == COMPV_RUNLOOP_STATE_ANIMATION_STARTED) {
		WorkerThread(NULL);
	}
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVMyRunLoopListener::newObj(CompVMyRunLoopListenerPtrPtr listener)
{
	COMPV_CHECK_EXP_RETURN(!listener, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*listener = new CompVMyRunLoopListener();
	COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	return COMPV_ERROR_CODE_S_OK;
}

static const std::string hello_path_from_file(const char* filename, const char* optional_folder = NULL)
{
	std::string path = COMPV_PATH_FROM_NAME(filename); // path from android's assets, iOS' bundle....
	// The path isn't correct when the binary is loaded from another process(e.g. when Intel VTune is used)
	if (optional_folder && !CompVFileUtils::exists(path.c_str())) {
		path = std::string(optional_folder) + std::string("/") + std::string(filename);
	}
	return path;
}


static void* WorkerThread(void* arg)
{
#if 1 // Chroma conversion
	CompVMatPtr image;
	COMPV_ERROR_CODE err;
	CompVSingleSurfaceLayerPtr singleSurfaceLayer;
	static int count = 0;
	static uint64_t timeStart;
	
	// FIXME: add support for RGB565 and BGR565, both LE and BE, LE being the default ones (ARM and X86 devices are LE by default)
	
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV444P, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_yuv444p.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV422P, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_yuv422p.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV420P, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_yuv420p.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_gray.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_UYVY422, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_uyvy422.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUYV422, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_yuyv422.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image)); // DirectShow / MediaFoundation
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_NV12, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_nv12.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_NV21, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_nv21.yuv", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGBA32, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_rgba.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_ARGB32, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_argb.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGRA32, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_bgra.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB24, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_rgb.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGR24, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_bgr.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB565LE, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_rgb565le.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB565BE, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_rgb565be.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGR565LE, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_bgr565le.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));
	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_BGR565BE, 1282, 720, 1282, hello_path_from_file("equirectangular_1282x720_bgr565be.rgb", "C:/Projects/GitHub/data/colorspace").c_str(), &image));

	//COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_YUV444P, 1282, 720, 1282, hello_path_from_file("yuv444p.yuv", "C:/Projects/GitHub/compv/tests/image").c_str(), &image));


	COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));
	timeStart = CompVTime::nowMillis();
	while (CompVDrawing::isLoopRunning()) {
		COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->drawImage(image));
		
		if (count == 1000) {
			uint64_t duration = (CompVTime::nowMillis() - timeStart);
			float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
			COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
			count = 0;
			timeStart = CompVTime::nowMillis();
		}
		++count;

		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
		COMPV_CHECK_CODE_BAIL(err = window->endDraw());
	}

bail:
	return NULL;

#elif 0 // Camera
    COMPV_ERROR_CODE err;
	CompVSingleSurfaceLayerPtr singleSurfaceLayer;
	MyCameraListenerPtr listener;
	CompVThreadPtr thread;
	int deviceIndex;

	//COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	//SALUT_CHECK_CODE_BAIL(COMPV_ERROR_CODE_E_INVALID_CALL, "Invalid call");
	
	COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));
	COMPV_CHECK_CODE_BAIL(err = MyCameraListener::newObj(&listener, *singleSurfaceLayer));
    COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));
	COMPV_CHECK_CODE_BAIL(err = camera->setListener(*listener));
    COMPV_CHECK_CODE_BAIL(err = camera->devices(devices));
    for (CompVCameraDeviceInfoList::iterator it = devices.begin(); it != devices.end(); ++it) {
        COMPV_DEBUG_INFO("Camera device: %s -> %s, %s", it->id.c_str(), it->name.c_str(), it->description.c_str());
    }
	deviceIndex = COMPV_MATH_MAX(static_cast<int>(devices.size() - 1), 0);
	COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_WIDTH, CAMERA_WIDTH));
	COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_HEIGHT, CAMERA_HEIGHT));
	COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_FPS, CAMERA_FPS));
	COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_SUBTYPE, CAMERA_SUBTYPE));
    COMPV_CHECK_CODE_BAIL(err = camera->start(devices[deviceIndex].id));

	//CompVThread::sleep(5000);
	//COMPV_CHECK_CODE_BAIL(err = camera->stop());
	//COMPV_CHECK_CODE_BAIL(err = camera->stop());
	//CompVThread::sleep(1000);
	//COMPV_CHECK_CODE_BAIL(err = camera->start(devices[0].id));

	// FIXME
	//for (int i = 0; i < 100; ++i) {
	//	CompVThread::sleep(0);
	//	COMPV_CHECK_CODE_BAIL(err = camera->stop());
	//	CompVThread::sleep(0);
	//	COMPV_CHECK_CODE_BAIL(err = camera->start(devices[0].id));
	//}

	//if (devices.size() > 1) {
	//	CompVThread::newObj(&thread, cameraRestart);
	//}

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
