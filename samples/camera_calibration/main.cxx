#include <compv/compv_api.h>

using namespace compv;

#define CAMERA_IDX							0
#if COMPV_ARCH_ARM
#	define CAMERA_WIDTH						320
#	define CAMERA_HEIGHT					240
#else
#	define CAMERA_WIDTH						640
#	define CAMERA_HEIGHT					480
#endif
#define CAMERA_FPS							10
#define CAMERA_SUBTYPE						COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS					true

#define CALIB_MIN_PLANS						20
#define CALIB_MAX_ERROR						0.65 // With my Logitech camera 0.41 is the best number I can get
#define CALIB_COMPUTE_TAN_DIST				false // whether to compute tangential distorsion (p1, p2) in addition to radial distorsion (k1, k2)
#define CALIB_COMPUTE_SKEW					false // whether to compute skew value (part of camera matrix K)
#define CALIB_VERBOSITY						0
#define CALIB_CHECK_PLANS					true // whether to check if the current and previous plan are almost the same. If they are almost the same, reject!
#define CALIB_CHECK_PLANS_MIN_SAD			13.f
#define CALIB_CHEKERBORAD_ROWS_COUNT		17//17//10 // Number of rows
#define CALIB_CHEKERBORAD_COLS_COUNT		12//12//8  // Number of cols

#if COMPV_OS_WINDOWS
#	define COMPV_SAMPLE_IMAGE_FOLDER			"C:/Projects/GitHub/adas-videos/iPhone6/calibration/"
#elif COMPV_OS_OSX
#	define COMPV_SAMPLE_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/calib/"
#else
#	define COMPV_SAMPLE_IMAGE_FOLDER			""
#endif

#define WINDOW_WIDTH			640
#define WINDOW_HEIGHT			480

#define TAG_SAMPLE			"Camera calibration"

static const compv_float32x4_t __color_red = { 1.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_bleu = { 0.f, 0.f, 1.f, 1.f };
static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };

static bool s_bCalibDone = false;
static size_t s_image_width = 1;
static size_t s_image_height = 1;

static COMPV_ERROR_CODE onWindowOrImageSizeChanged(
	const size_t window_Width, const size_t window_height, const size_t image_Width, const size_t image_height,
	CompVSurfacePtr surfaceOrig, CompVSurfacePtr surfaceEdges, CompVSurfacePtr surfaceLinesRaw, CompVSurfacePtr surfaceLinesGrouped, CompVSurfacePtr surfaceUndist
)
{
	COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "%s", __FUNCTION__);

	CompVRectInt src, dst, view;

	const int window_Width_int = static_cast<int>(window_Width);
	const int window_height_int = static_cast<int>(window_height);
	const int image_Width_int = static_cast<int>(image_Width);
	const int image_height_int = static_cast<int>(image_height);
	const int w_width = (window_Width_int >> 1);
	const int w_height = s_bCalibDone ? window_height_int : (window_height_int >> 1);

	src.left = src.top = dst.left = dst.top = 0;
	src.right = (image_Width_int);
	src.bottom = (image_height_int);
	dst.right = w_width;
	dst.bottom = w_height;
	CompVViewportPtr viewportAspectRatio;
	COMPV_CHECK_CODE_RETURN(CompVViewport::newObj(&viewportAspectRatio, CompViewportSizeFlags::makeDynamicAspectRatio()));

	COMPV_CHECK_CODE_RETURN(CompVViewport::viewport(src, dst, viewportAspectRatio, &view));

	const int v_width = (view.right - view.left);
	const int v_height = (view.bottom - view.top);

	if (s_bCalibDone) {
		COMPV_CHECK_CODE_RETURN(surfaceOrig->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left, view.top, v_width, v_height));
		COMPV_CHECK_CODE_RETURN(surfaceUndist->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left + w_width, view.top, v_width, v_height));
	}
	else {
		// First row
		COMPV_CHECK_CODE_RETURN(surfaceOrig->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left, view.top, v_width, v_height));
		COMPV_CHECK_CODE_RETURN(surfaceEdges->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left + w_width, view.top, v_width, v_height));
		// Second row
		COMPV_CHECK_CODE_RETURN(surfaceLinesRaw->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left, (view.top + w_height), v_width, v_height));
		COMPV_CHECK_CODE_RETURN(surfaceLinesGrouped->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left + w_width, (view.top + w_height), v_width, v_height));
	}

	s_image_width = image_Width;
	s_image_height = image_height;

	return COMPV_ERROR_CODE_S_OK;
}

/* My window listener */
COMPV_OBJECT_DECLARE_PTRS(MyWindowListener)
class CompVMyWindowListener : public CompVWindowListener {
protected:
	CompVMyWindowListener(CompVSurfacePtr surfaceOrig, CompVSurfacePtr surfaceEdges, CompVSurfacePtr surfaceLinesRaw, CompVSurfacePtr surfaceLinesGrouped, CompVSurfacePtr surfaceUndist)
		: m_ptrSurfaceOrig(surfaceOrig), m_ptrSurfaceEdges(surfaceEdges), m_ptrSurfaceLinesRaw(surfaceLinesRaw), m_ptrSurfaceLinesGrouped(surfaceLinesGrouped), m_ptrSurfaceUndist(surfaceUndist) { }
public:
	virtual ~CompVMyWindowListener() {

	}
	
	static COMPV_ERROR_CODE newObj(CompVMyWindowListenerPtrPtr listener, CompVSurfacePtr surfaceOrig, CompVSurfacePtr surfaceEdges, CompVSurfacePtr surfaceLinesRaw, CompVSurfacePtr surfaceLinesGrouped, CompVSurfacePtr surfaceUndist) {
		COMPV_CHECK_EXP_RETURN(!listener || !surfaceOrig || !surfaceEdges || !surfaceLinesRaw || !surfaceLinesGrouped || !surfaceUndist, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*listener = new CompVMyWindowListener(surfaceOrig, surfaceEdges, surfaceLinesRaw, surfaceLinesGrouped, surfaceUndist);
		COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		return COMPV_ERROR_CODE_S_OK;
	}
	virtual COMPV_ERROR_CODE onSizeChanged(size_t newWidth, size_t newHeight) override {
		COMPV_CHECK_CODE_RETURN(onWindowOrImageSizeChanged(
			newWidth, newHeight, s_image_width, s_image_height,
			m_ptrSurfaceOrig, m_ptrSurfaceEdges, m_ptrSurfaceLinesRaw, m_ptrSurfaceLinesGrouped, m_ptrSurfaceUndist
		));
		return COMPV_ERROR_CODE_S_OK;
	};
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_WINDOW_STATE newState) override {
		return COMPV_ERROR_CODE_S_OK;
	};
private:
	CompVSurfacePtr m_ptrSurfaceOrig;
	CompVSurfacePtr m_ptrSurfaceEdges;
	CompVSurfacePtr m_ptrSurfaceLinesRaw;
	CompVSurfacePtr m_ptrSurfaceLinesGrouped;
	CompVSurfacePtr m_ptrSurfaceUndist;
};


/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVMyWindowListenerPtr windowListener;
		CompVMultiSurfaceLayerPtr surfaceMulti;
		CompVSurfacePtr surfaceOrig;
		CompVSurfacePtr surfaceEdges;
		CompVSurfacePtr surfaceLinesRaw;
		CompVSurfacePtr surfaceLinesGrouped;
		CompVSurfacePtr surfaceUndist;
		CompVCameraPtr camera;
		CompVCameraDeviceInfoList devices;
		std::string cameraId = ""; // empty string means default
		CompVMatPtr image;
		CompVCameraCallbackOnNewFrame funcPtrOnNewFrame;
		CompVCalibCameraPtr calib;
		CompVCalibContex calibCtx;
		CompVDrawingOptions drawingOptions;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create window, hook listener and add surfaces
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addMultiLayerSurface(&surfaceMulti));
		COMPV_CHECK_CODE_BAIL(err = surfaceMulti->addSurface(&surfaceOrig, window->width(), window->height(), false));
		COMPV_CHECK_CODE_BAIL(err = surfaceMulti->addSurface(&surfaceEdges, window->width(), window->height(), false));
		COMPV_CHECK_CODE_BAIL(err = surfaceMulti->addSurface(&surfaceLinesRaw, window->width(), window->height(), false));
		COMPV_CHECK_CODE_BAIL(err = surfaceMulti->addSurface(&surfaceLinesGrouped, window->width(), window->height(), false));
		COMPV_CHECK_CODE_BAIL(err = surfaceMulti->addSurface(&surfaceUndist, window->width(), window->height(), false));
		COMPV_CHECK_CODE_BAIL(err = CompVMyWindowListener::newObj(&windowListener, surfaceOrig, surfaceEdges, surfaceLinesRaw, surfaceLinesGrouped, surfaceUndist));
		COMPV_CHECK_CODE_BAIL(err = window->addListener(*windowListener));

		// Create calibration object and context options
		COMPV_CHECK_CODE_RETURN(CompVCalibCamera::newObj(&calib, CALIB_CHEKERBORAD_ROWS_COUNT, CALIB_CHEKERBORAD_COLS_COUNT));
		calibCtx.compute_skew = CALIB_COMPUTE_SKEW;
		calibCtx.compute_tangential_dist = CALIB_COMPUTE_TAN_DIST;
		calibCtx.verbosity = CALIB_VERBOSITY;
		calibCtx.check_plans = CALIB_CHECK_PLANS;
		calibCtx.check_plans_min_sad = CALIB_CHECK_PLANS_MIN_SAD;

		// Set drawing options (common values)
		drawingOptions.lineWidth = 1.f;
		drawingOptions.fontSize = 12;

		// Create my camera
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

		// Add 'OnNewFrame' callback to the camera
		funcPtrOnNewFrame = [&](const CompVMatPtr& image) -> COMPV_ERROR_CODE {
			COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
			if (CompVDrawing::isLoopRunning()) {
				CompVMatPtr imageGray, imageOrig, imageUndist;

#if 0
				COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &imageGray));
				imageOrig = image;
#else
				static size_t __index = 0;
				size_t file_index = ((__index++ % 20) + 1)/*7*//*2*//*65*//*65*//*47*//*65*//*47*//*55*//*47*//*48*//*52*/;
				if (s_bCalibDone) {
					//file_index = 5;
				}
				const std::string file_name = std::string("calib") + CompVBase::to_string(file_index) + std::string("_3264x2448_gray.yuv");
				const std::string file_path = COMPV_PATH_FROM_NAME((std::string(COMPV_SAMPLE_IMAGE_FOLDER) + file_name).c_str());
				COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 3264, 2448, 3264, file_path.c_str(), &imageGray));

				//const std::string file_path = "C:/Projects/GitHub/data/adas/birdview_chess_1920x1090_gray.yuv";
				//COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1920, 1090, 1920, file_path.c_str(), &imageGray));

				imageOrig = imageGray;
				//COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "%s", file_name.c_str());
				COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove the sleep function");
				//if (s_bCalibDone) {
				//CompVThread::sleep(1000);
				//}
#endif
				// Check if image size changed
				if (s_image_width != imageOrig->cols() || s_image_height != imageOrig->rows()) {
					COMPV_CHECK_CODE_RETURN(onWindowOrImageSizeChanged(window->width(), window->height(), imageOrig->cols(), imageOrig->rows(),
						surfaceOrig, surfaceEdges, surfaceLinesRaw, surfaceLinesGrouped, surfaceUndist));
				}

				/* Process image and calibration */
				if (!s_bCalibDone) {
					COMPV_CHECK_CODE_RETURN(calib->process(imageGray, calibCtx));

					// FIXME:
					COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove");
					if (calibCtx.code != COMPV_CALIB_CAMERA_RESULT_OK) {
						int kaka = 0;
					}

					if (calibCtx.planes.size() >= CALIB_MIN_PLANS) {
						/* Calibration */
						COMPV_CHECK_CODE_RETURN(calib->calibrate(calibCtx));
						if (calibCtx.reproj_error > CALIB_MAX_ERROR) {
							COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Reproj error too high (%f > %f). You should really try with #%d different planes trying again", calibCtx.reproj_error, CALIB_MAX_ERROR, CALIB_MIN_PLANS);
							calibCtx.clean(); // clean all plans and start over
						}
						else {
							s_bCalibDone = true;
							COMPV_CHECK_CODE_RETURN(onWindowOrImageSizeChanged(window->width(), window->height(), imageOrig->cols(), imageOrig->rows(),
								surfaceOrig, surfaceEdges, surfaceLinesRaw, surfaceLinesGrouped, surfaceUndist));
							COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Calibration is done!!");
							COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Camera matrix (K) = \n%f,%f,%f\n%f,%f,%f\n%f,%f,%f",
								*calibCtx.K->ptr<const compv_float64_t>(0, 0), *calibCtx.K->ptr<const compv_float64_t>(0, 1), *calibCtx.K->ptr<const compv_float64_t>(0, 2),
								*calibCtx.K->ptr<const compv_float64_t>(1, 0), *calibCtx.K->ptr<const compv_float64_t>(1, 1), *calibCtx.K->ptr<const compv_float64_t>(1, 2),
								*calibCtx.K->ptr<const compv_float64_t>(2, 0), *calibCtx.K->ptr<const compv_float64_t>(2, 1), *calibCtx.K->ptr<const compv_float64_t>(2, 2));
							COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Radial distorsions (d.r) = %f,%f",
								*calibCtx.d->ptr<const compv_float64_t>(0, 0), *calibCtx.d->ptr<const compv_float64_t>(1, 0));
							if (calibCtx.d->rows() > 2) {
								COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Tangential distorsions (d.t) = %f,%f",
									*calibCtx.d->ptr<const compv_float64_t>(2, 0), *calibCtx.d->ptr<const compv_float64_t>(3, 0));
							}
							// Rotation matrices and translation vectors are stored in "calibCtx.planes[n].R/t"
						}
					}
				}

				/* undist */
				if (s_bCalibDone) {
					COMPV_CHECK_CODE_RETURN(CompVCalibUtils::undist2DImage(imageGray, calibCtx.K, calibCtx.d, &imageUndist));
				}

				/* Begin drawing */
				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());

				/* Original image */
				COMPV_CHECK_CODE_BAIL(err = surfaceOrig->activate());
				COMPV_CHECK_CODE_BAIL(err = surfaceOrig->drawImage(imageOrig));

				if (s_bCalibDone) {
					/* Undist */
					COMPV_CHECK_CODE_BAIL(err = surfaceUndist->activate());
					COMPV_CHECK_CODE_BAIL(err = surfaceUndist->drawImage(imageUndist));
				}
				else {
					/* Edges */
					COMPV_CHECK_CODE_BAIL(err = surfaceEdges->activate());
					COMPV_CHECK_CODE_BAIL(err = surfaceEdges->drawImage(calibCtx.edges));

					/* Raw Lines */
					drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_RANDOM;
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesRaw->activate());
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesRaw->drawImage(calibCtx.edges));
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesRaw->renderer()->canvas()->drawLines(calibCtx.lines_raw.lines_cartesian, &drawingOptions));

					/* Grouped lines */
					drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
					drawingOptions.setColor(__color_yellow);
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesGrouped->activate());
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesGrouped->drawImage(calibCtx.edges));
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesGrouped->renderer()->canvas()->drawLines(calibCtx.lines_grouped.lines_cartesian, &drawingOptions));
					drawingOptions.setColor(__color_red);
					COMPV_CHECK_CODE_BAIL(err = surfaceLinesGrouped->renderer()->canvas()->drawPoints(calibCtx.plane_curr.intersections, &drawingOptions));
					if (!calibCtx.plane_curr.intersections.empty() && surfaceLinesGrouped->renderer()->canvas()->haveDrawTexts()) {
						CompVStringVector labels(calibCtx.plane_curr.intersections.size());
						for (size_t index = 0; index < calibCtx.plane_curr.intersections.size(); ++index) {
							labels[index] = CompVBase::to_string(index);
						}
						drawingOptions.setColor(__color_yellow);
						COMPV_CHECK_CODE_BAIL(err = surfaceLinesGrouped->renderer()->canvas()->drawTexts(labels, calibCtx.plane_curr.intersections, &drawingOptions));
					}
					if (calibCtx.plane_curr.homography) {
						const size_t pattern_w = calibCtx.plane_curr.pattern_width;
						const size_t pattern_h = calibCtx.plane_curr.pattern_height;
						CompVMatPtr rectPattern, rectHomograyApplied;
						compv_float64_t *x, *y;
						CompVLineFloat32Vector rectLines(4);
						COMPV_CHECK_CODE_BAIL(err = CompVMat::newObjAligned<compv_float64_t>(&rectPattern, 3, 4));
						COMPV_CHECK_CODE_BAIL(err = rectPattern->one_row<compv_float64_t>(2)); // with Z = 1
						x = rectPattern->ptr<compv_float64_t>(0);
						y = rectPattern->ptr<compv_float64_t>(1);
						x[0] = 0, x[1] = static_cast<compv_float64_t>(pattern_w), x[2] = static_cast<compv_float64_t>(pattern_w), x[3] = 0.0;
						y[0] = 0, y[1] = 0, y[2] = static_cast<compv_float64_t>(pattern_h), y[3] = static_cast<compv_float64_t>(pattern_h);
						// Perspecive transform using homography matrix
						COMPV_CHECK_CODE_BAIL(err = CompVMathTransform::perspective2D(rectPattern, calibCtx.plane_curr.homography, &rectHomograyApplied));
						// Draw the transformed rectangle
						x = rectHomograyApplied->ptr<compv_float64_t>(0);
						y = rectHomograyApplied->ptr<compv_float64_t>(1);
						// using drawLines instead of drawRectangles because the rectangle doesn't have square angles (90 degrees)
						rectLines[0].a.x = static_cast<compv_float32_t>(x[0]), rectLines[0].a.y = static_cast<compv_float32_t>(y[0]), rectLines[0].b.x = static_cast<compv_float32_t>(x[1]), rectLines[0].b.y = static_cast<compv_float32_t>(y[1]);
						rectLines[1].a.x = static_cast<compv_float32_t>(x[1]), rectLines[1].a.y = static_cast<compv_float32_t>(y[1]), rectLines[1].b.x = static_cast<compv_float32_t>(x[2]), rectLines[1].b.y = static_cast<compv_float32_t>(y[2]);
						rectLines[2].a.x = static_cast<compv_float32_t>(x[2]), rectLines[2].a.y = static_cast<compv_float32_t>(y[2]), rectLines[2].b.x = static_cast<compv_float32_t>(x[3]), rectLines[2].b.y = static_cast<compv_float32_t>(y[3]);
						rectLines[3].a.x = static_cast<compv_float32_t>(x[3]), rectLines[3].a.y = static_cast<compv_float32_t>(y[3]), rectLines[3].b.x = static_cast<compv_float32_t>(x[0]), rectLines[3].b.y = static_cast<compv_float32_t>(y[0]);
						drawingOptions.setColor(__color_bleu);
						COMPV_CHECK_CODE_BAIL(err = surfaceLinesGrouped->renderer()->canvas()->drawLines(rectLines, &drawingOptions));
					}
				}

				/* Swap back buffer <-> front */
				COMPV_CHECK_CODE_BAIL(err = surfaceMulti->blit());
			bail:
				/* End drawing */
				COMPV_CHECK_CODE_NOP(err = window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
				// Deactivate the surfaces
				COMPV_CHECK_CODE_NOP(err = surfaceOrig->deActivate());
				COMPV_CHECK_CODE_NOP(err = surfaceEdges->deActivate());
				COMPV_CHECK_CODE_NOP(err = surfaceLinesRaw->deActivate());
				COMPV_CHECK_CODE_NOP(err = surfaceLinesGrouped->deActivate());
				COMPV_CHECK_CODE_NOP(err = surfaceUndist->deActivate());
			}
			return err;
		};
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnNewFrame(funcPtrOnNewFrame));

		// Add 'OnError' callback to the camera
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnError([&](const std::string& message) -> COMPV_ERROR_CODE {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Camera error: %s", message.c_str()); // probably a disconnect
			return COMPV_ERROR_CODE_S_OK;
		}));

		// Start ui runloop
		// Setting a state listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
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
