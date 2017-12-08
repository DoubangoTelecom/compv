#include <compv/compv_api.h>

using namespace compv;

#define CAMERA_IDX			0
#define CAMERA_WIDTH		1280
#define CAMERA_HEIGHT		720
#define CAMERA_FPS			25
#define CAMERA_SUBTYPE		COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS	true

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define CCL_ID				COMPV_LSL_ID // Light Speed Labeling

#define TAG_SAMPLE			"Connected Component Labeling"

static COMPV_ERROR_CODE FIXME_extract_label(const CompVConnectedComponentLabelingPtr& ccl_obj, const CompVConnectedComponentLabelingResult& ccl_result, const int label, CompVMatPtrPtr image)
{
	const size_t width = ccl_result.labels->cols();
	const size_t height = ccl_result.labels->rows();
	const size_t stride = ccl_result.labels->stride();

	CompVMatPtr imageOut;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(image, height, width, stride));

	const int* labelsPtr = ccl_result.labels->ptr<const int>();
	uint8_t* imageOutPtr = (*image)->ptr<uint8_t>();

	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			imageOutPtr[i] = (labelsPtr[i] == label) ? 0xff : 0x00;
		}
		labelsPtr += stride;
		imageOutPtr += stride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		CompVCameraPtr camera;
		CompVCameraDeviceInfoList devices;
		std::string cameraId = ""; // empty string means default

		CompVMatPtr imageBinar, blob;
		CompVConnectedComponentLabelingResult ccl_result;
		CompVConnectedComponentLabelingPtr ccl_obj;
		double threshold;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create my camera and add a listener to it 
		COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));
		// Get list of devices/cameras and print them to the screen (optional)
		COMPV_CHECK_CODE_BAIL(err = camera->devices(devices));
		COMPV_CHECK_EXP_BAIL(devices.empty(), err = COMPV_ERROR_CODE_E_NOT_FOUND, "No camera device found");
		for (CompVCameraDeviceInfoList::iterator it = devices.begin(); it != devices.end(); ++it) {
			COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Camera device: %s -> %s, %s", it->id.c_str(), it->name.c_str(), it->description.c_str());
		}
		// Set camera parameters (optional)
		cameraId = devices[COMPV_MATH_MIN(CAMERA_IDX, (devices.size() - 1))].id;
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_WIDTH, CAMERA_WIDTH));
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_HEIGHT, CAMERA_HEIGHT));
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_FPS, CAMERA_FPS));
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_SUBTYPE, CAMERA_SUBTYPE));
		COMPV_CHECK_CODE_BAIL(err = camera->setBool(COMPV_CAMERA_CAP_BOOL_AUTOFOCUS, CAMERA_AUTOFOCUS));

		// Create the ccl and set default settings
		COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabeling::newObj(&ccl_obj, CCL_ID));
		if (CCL_ID == COMPV_LSL_ID) {
			COMPV_CHECK_CODE_RETURN(ccl_obj->setInt(COMPV_LSL_SET_INT_TYPE, COMPV_LSL_TYPE_STD));
		}

		// Add 'OnNewFrame' callback to the camera
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnNewFrame([&](const CompVMatPtr& image) -> COMPV_ERROR_CODE {
			COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
			static int __label = 0;
			if (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &imageBinar));
				COMPV_CHECK_CODE_RETURN(CompVImageThreshold::otsu(imageBinar, threshold, &imageBinar));
				//COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1285, 1285, 1285, "C:/Projects/GitHub/data/morpho/diffract_1285x1285_gray.yuv", &imageBinar));
				//COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1285, 803, 1285, "C:/Projects/GitHub/data/morpho/dummy_1285x803_gray.yuv", &imageBinar));
				COMPV_CHECK_CODE_RETURN(ccl_obj->process(imageBinar, ccl_result));
				COMPV_CHECK_CODE_RETURN(FIXME_extract_label(ccl_obj, ccl_result, (++__label % ccl_result.labels_count), &blob));
				COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "label = %d", __label);

				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->drawImage(blob/*imageBinar*/));
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(err = window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
			}
			//getchar();
			return err;
		}));

		// Add 'OnError' callback to the camera
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnError([&](const std::string& message) -> COMPV_ERROR_CODE {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Camera error: %s", message.c_str()); // probably a disconnect
			return COMPV_ERROR_CODE_S_OK;
		}));

		// Start ui runloop
		// Setting a listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
		COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop([&](const COMPV_RUNLOOP_STATE& newState) -> COMPV_ERROR_CODE {
			COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "RunLoop onStateChanged(%d)", newState);
			switch (newState) {
			case COMPV_RUNLOOP_STATE_LOOP_STARTED:
			default:
				return COMPV_ERROR_CODE_S_OK;
			case COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
			case COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
				return camera->start(cameraId); // start camera
			case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
			case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
			case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
				return camera->stop(); // stop camera
			}
		}));

	bail:
		if (COMPV_ERROR_CODE_IS_NOK(err)) {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Something went wrong!!");
		}
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	compv_main_return(0);
}
