#include <compv/compv_api.h>

using namespace compv;

#define CAMERA_IDX				0
#if COMPV_ARCH_ARM
#	define CAMERA_WIDTH			320
#	define CAMERA_HEIGHT		240
#else
#	define CAMERA_WIDTH			640
#	define CAMERA_HEIGHT		480
#endif
#define CAMERA_FPS				10
#define CAMERA_SUBTYPE			COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS		true

#define CALIB_MIN_PLANS			20
#define CALIB_MAX_ERROR			0.76 // With my Logitech camera 0.41 is the best number I can get
#define CALIB_COMPUTE_TAN_DIST	false // whether to compute tangential distorsion (p1, p2) in addition to radial distorsion (k1, k2)
#define CALIB_COMPUTE_SKEW		true // whether to compute skew value (part of camera matrix K)

#define WINDOW_WIDTH			640
#define WINDOW_HEIGHT			480

#define TAG_SAMPLE			"Camera calibration"

static const compv_float32x4_t __color_black = { 0.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_red = { 1.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_green = { 0.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_bleu = { 0.f, 0.f, 1.f, 1.f };
static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };

/* IWindowSizeChanged */
class IWindowSizeChanged {
public:
	virtual COMPV_ERROR_CODE onWindowSizeChanged(const size_t newWidth, const size_t newHeight) = 0;
};

/* My window listener */
COMPV_OBJECT_DECLARE_PTRS(MyWindowListener)
class CompVMyWindowListener : public CompVWindowListener {
protected:
	CompVMyWindowListener(const IWindowSizeChanged* pcIWindowSizeChanged): m_pcIWindowSizeChanged(pcIWindowSizeChanged) { }
public:
	virtual ~CompVMyWindowListener() {

	}
	static COMPV_ERROR_CODE newObj(CompVMyWindowListenerPtrPtr listener, const IWindowSizeChanged* pcIWindowSizeChanged) {
		COMPV_CHECK_EXP_RETURN(!listener || !pcIWindowSizeChanged, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*listener = new CompVMyWindowListener(pcIWindowSizeChanged);
		COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		return COMPV_ERROR_CODE_S_OK;
	}
	virtual COMPV_ERROR_CODE onSizeChanged(size_t newWidth, size_t newHeight) override {
		COMPV_CHECK_CODE_RETURN(const_cast<IWindowSizeChanged*>(m_pcIWindowSizeChanged)->onWindowSizeChanged(newWidth, newHeight));
		return COMPV_ERROR_CODE_S_OK;
	};
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_WINDOW_STATE newState) override {
		return COMPV_ERROR_CODE_S_OK;
	};
private:
	const IWindowSizeChanged* m_pcIWindowSizeChanged;
};

/* My runloop listener (optional) */
COMPV_OBJECT_DECLARE_PTRS(MyRunLoopListener)
class CompVMyRunLoopListener : public CompVRunLoopListener
{
protected:
	CompVMyRunLoopListener(CompVCameraPtr camera, const std::string& cameraId) : m_ptrCamera(camera), m_strCameraId(cameraId) { }
public:
	virtual ~CompVMyRunLoopListener() { }
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_RUNLOOP_STATE newState) override {
		COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "RunLoop onStateChanged(%d)", newState);
		switch (newState) {
		case COMPV_RUNLOOP_STATE_LOOP_STARTED:
		default:
			return COMPV_ERROR_CODE_S_OK;
		case COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
		case COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
			return m_ptrCamera->start(m_strCameraId);
		case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
		case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
		case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
			return m_ptrCamera->stop();
		}
	}
	static COMPV_ERROR_CODE newObj(CompVMyRunLoopListenerPtrPtr listener, CompVCameraPtr camera, const std::string& cameraId) {
		COMPV_CHECK_EXP_RETURN(!listener || !camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*listener = new CompVMyRunLoopListener(camera, cameraId);
		COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	CompVCameraPtr m_ptrCamera;
	const std::string m_strCameraId;
};

/* My camera listener */
COMPV_OBJECT_DECLARE_PTRS(MyCameraListener)
class CompVMyCameraListener : public CompVCameraListener, public IWindowSizeChanged
{
	friend class CompVMyWindowListener;
protected:
	CompVMyCameraListener(CompVWindowPtr ptrWindow): m_bCalibrationDone(false), m_nImageWidth(0), m_nImageHeight(0), m_ptrWindow(ptrWindow) { }
public:
	virtual ~CompVMyCameraListener() { }

	virtual COMPV_ERROR_CODE onNewFrame(const CompVMatPtr& image) override {
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		if (CompVDrawing::isLoopRunning()) {
			CompVMatPtr imageGray, imageOrig;

#if 0
			COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &imageGray));
			imageOrig = image;
#else
			static size_t __index = 0;
			size_t file_index = 47 + ((__index++) % 20)/*65*//*65*//*47*//*65*//*47*//*55*//*47*//*48*//*52*/;
			std::string file_path = std::string("C:/Projects/GitHub/data/calib/P10100")+ CompVBase::to_string(file_index) +std::string("s_640x480_gray.yuv");
			//std::string file_path = "C:/Projects/GitHub/data/calib/P1010047s_90deg_640x480_gray.yuv";
			COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 640, 480, 640, file_path.c_str(), &imageGray));
			imageOrig = imageGray;
			COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "%s", file_path.c_str());
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove the sleep function");
			if (m_bCalibrationDone) {
				CompVThread::sleep(1000);
			}
#endif
			// Check if image size changed
			if (m_nImageWidth != imageOrig->cols() || m_nImageHeight != imageOrig->rows()) {
				COMPV_CHECK_CODE_RETURN(onImageSizeChanged(imageOrig->cols(), imageOrig->rows()));
			}
			
			/* Process image and calibration */
			if (!m_bCalibrationDone) {
				COMPV_CHECK_CODE_RETURN(m_ptrCalib->process(imageGray, m_CalibResult));
				if (m_CalibResult.planes.size() >= CALIB_MIN_PLANS) {
					/* Calibration */
					COMPV_CHECK_CODE_RETURN(m_ptrCalib->calibrate(m_CalibResult));
					if (m_CalibResult.reproj_error > CALIB_MAX_ERROR) {
						COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Reproj error too high (%f > %f), trying again", m_CalibResult.reproj_error, CALIB_MAX_ERROR);
						m_CalibResult.clean(); // clean all plans and start over
					}
					else {
						m_CalibResult.clean(); // FIXME(dmi): remove
						//m_bCalibrationDone = true;
						//COMPV_CHECK_CODE_RETURN(onImageSizeChanged(imageOrig->cols(), imageOrig->rows()));
						COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Calibration is done!!");
					}
				}
			}

			/* Begin drawing */
			COMPV_CHECK_CODE_BAIL(err = m_ptrWindow->beginDraw());
			
			/* Original image */
			COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceOriginal->activate());
			COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceOriginal->drawImage(imageOrig));

			if (m_bCalibrationDone) {
				/* Undist */
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceUndist->activate());
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceUndist->drawImage(imageOrig));
			}
			else {
				/* Edges */
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceEdges->activate());
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceEdges->drawImage(m_CalibResult.edges));

				/* Raw Lines */
				m_DrawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_RANDOM;
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLinesRaw->activate());
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLinesRaw->drawImage(m_CalibResult.edges));
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLinesRaw->renderer()->canvas()->drawLines(m_CalibResult.lines_raw.lines_cartesian, &m_DrawingOptions));

				/* Grouped lines */
				m_DrawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
				m_DrawingOptions.setColor(__color_yellow);
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLineGrouped->activate());
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLineGrouped->drawImage(m_CalibResult.edges));
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLineGrouped->renderer()->canvas()->drawLines(m_CalibResult.lines_grouped.lines_cartesian, &m_DrawingOptions));
				m_DrawingOptions.setColor(__color_red);
				COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLineGrouped->renderer()->canvas()->drawPoints(m_CalibResult.plane_curr.intersections, &m_DrawingOptions));
				if (!m_CalibResult.plane_curr.intersections.empty() && m_ptrSurfaceLineGrouped->renderer()->canvas()->haveDrawTexts()) {
					CompVStringVector labels(m_CalibResult.plane_curr.intersections.size());
					for (size_t index = 0; index < m_CalibResult.plane_curr.intersections.size(); ++index) {
						labels[index] = CompVBase::to_string(index);
					}
					m_DrawingOptions.setColor(__color_yellow);
					COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLineGrouped->renderer()->canvas()->drawTexts(labels, m_CalibResult.plane_curr.intersections, &m_DrawingOptions));
				}
				if (m_CalibResult.plane_curr.homography) {
					const size_t pattern_w = m_CalibResult.plane_curr.pattern_width;
					const size_t pattern_h = m_CalibResult.plane_curr.pattern_height;
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
					COMPV_CHECK_CODE_BAIL(err = CompVMathTransform<compv_float64_t>::perspective2D(rectPattern, m_CalibResult.plane_curr.homography, &rectHomograyApplied));
					// Draw the transformed rectangle
					x = rectHomograyApplied->ptr<compv_float64_t>(0);
					y = rectHomograyApplied->ptr<compv_float64_t>(1);
					// using drawLines instead of drawRectangles because the rectangle doesn't have square angles (90 degrees)
					rectLines[0].a.x = static_cast<compv_float32_t>(x[0]), rectLines[0].a.y = static_cast<compv_float32_t>(y[0]), rectLines[0].b.x = static_cast<compv_float32_t>(x[1]), rectLines[0].b.y = static_cast<compv_float32_t>(y[1]);
					rectLines[1].a.x = static_cast<compv_float32_t>(x[1]), rectLines[1].a.y = static_cast<compv_float32_t>(y[1]), rectLines[1].b.x = static_cast<compv_float32_t>(x[2]), rectLines[1].b.y = static_cast<compv_float32_t>(y[2]);
					rectLines[2].a.x = static_cast<compv_float32_t>(x[2]), rectLines[2].a.y = static_cast<compv_float32_t>(y[2]), rectLines[2].b.x = static_cast<compv_float32_t>(x[3]), rectLines[2].b.y = static_cast<compv_float32_t>(y[3]);
					rectLines[3].a.x = static_cast<compv_float32_t>(x[3]), rectLines[3].a.y = static_cast<compv_float32_t>(y[3]), rectLines[3].b.x = static_cast<compv_float32_t>(x[0]), rectLines[3].b.y = static_cast<compv_float32_t>(y[0]);
					m_DrawingOptions.setColor(__color_bleu);
					COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLineGrouped->renderer()->canvas()->drawLines(rectLines, &m_DrawingOptions));
				}
			}

			/* Swap back buffer <-> front */
			COMPV_CHECK_CODE_BAIL(err = m_ptrMultiSurface->blit());
		bail:
			/* End drawing */
			COMPV_CHECK_CODE_NOP(err = m_ptrWindow->endDraw()); // Make sure 'endDraw()' will be called regardless the result
			// Deactivate the surfaces
			COMPV_CHECK_CODE_NOP(err = m_ptrSurfaceOriginal->deActivate());
			COMPV_CHECK_CODE_NOP(err = m_ptrSurfaceEdges->deActivate());
			COMPV_CHECK_CODE_NOP(err = m_ptrSurfaceLinesRaw->deActivate());
			COMPV_CHECK_CODE_NOP(err = m_ptrSurfaceLineGrouped->deActivate());
			COMPV_CHECK_CODE_NOP(err = m_ptrSurfaceUndist->deActivate());
		}
		return err;
	}

	virtual COMPV_ERROR_CODE onError(const std::string& message) override {
		COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Camera error: %s", message.c_str()); // probably a disconnect
		return COMPV_ERROR_CODE_S_OK;
	}

	virtual COMPV_ERROR_CODE onWindowSizeChanged(const size_t window_Width, const size_t window_height) override
	{
		COMPV_CHECK_CODE_RETURN(onWindowOrImageSizeChanged(window_Width, window_height,
			m_nImageWidth ? m_nImageWidth : CAMERA_WIDTH, m_nImageHeight ? m_nImageHeight : CAMERA_HEIGHT));
		return COMPV_ERROR_CODE_S_OK;
	}


	static COMPV_ERROR_CODE newObj(CompVMyCameraListenerPtrPtr listener, CompVWindowPtr ptrWindow) {
		COMPV_CHECK_EXP_RETURN(!listener || !ptrWindow, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMyCameraListenerPtr listener_ = new CompVMyCameraListener(ptrWindow);
		COMPV_CHECK_EXP_RETURN(!listener_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		COMPV_CHECK_CODE_RETURN(CompVCalibCamera::newObj(&listener_->m_ptrCalib));

		// Calibration options
		listener_->m_CalibResult.compute_skew = CALIB_COMPUTE_SKEW;
		listener_->m_CalibResult.compute_tangential_dist = CALIB_COMPUTE_TAN_DIST;

		// Drawing options
		listener_->m_DrawingOptions.lineWidth = 1.f;
		listener_->m_DrawingOptions.fontSize = 14;

		// Multi-layer surface
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrWindow->addMultiLayerSurface(&listener_->m_ptrMultiSurface));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMultiSurface->addSurface(&listener_->m_ptrSurfaceOriginal, ptrWindow->width(), ptrWindow->height(), false));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMultiSurface->addSurface(&listener_->m_ptrSurfaceEdges, ptrWindow->width(), ptrWindow->height(), false));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMultiSurface->addSurface(&listener_->m_ptrSurfaceLinesRaw, ptrWindow->width(), ptrWindow->height(), false));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMultiSurface->addSurface(&listener_->m_ptrSurfaceLineGrouped, ptrWindow->width(), ptrWindow->height(), false));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMultiSurface->addSurface(&listener_->m_ptrSurfaceUndist, ptrWindow->width(), ptrWindow->height(), false));

		// Window listener
		COMPV_CHECK_CODE_RETURN(CompVMyWindowListener::newObj(&listener_->m_ptrWindowListener, *listener_));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrWindow->addListener(*listener_->m_ptrWindowListener));

		*listener = listener_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	COMPV_ERROR_CODE onImageSizeChanged(const size_t image_Width, const size_t image_height)
	{
		COMPV_CHECK_CODE_RETURN(onWindowOrImageSizeChanged(m_ptrWindow->width(), m_ptrWindow->height(), image_Width, image_height));
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE onWindowOrImageSizeChanged(const size_t window_Width, const size_t window_height, const size_t image_Width, const size_t image_height)
	{
		COMPV_DEBUG_INFO_EX(TAG_SAMPLE, __FUNCTION__);
		
		CompVRectInt src, dst, view;

		const int window_Width_int = static_cast<int>(window_Width);
		const int window_height_int = static_cast<int>(window_height);
		const int image_Width_int = static_cast<int>(image_Width);
		const int image_height_int = static_cast<int>(image_height);
		const int w_width = (window_Width_int >> 1);
		const int w_height = m_bCalibrationDone ? window_height_int : (window_height_int >> 1);

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

		if (m_bCalibrationDone) {
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceOriginal->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left, view.top, v_width, v_height));
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceUndist->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left + w_width, view.top, v_width, v_height));
		}
		else {
			// First row
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceOriginal->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left, view.top, v_width, v_height));
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceEdges->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left + w_width, view.top, v_width, v_height));
			// Second row
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceLinesRaw->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left, (view.top + w_height), v_width, v_height));
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceLineGrouped->viewport()->reset(CompViewportSizeFlags::makeStatic(), view.left + w_width, (view.top + w_height), v_width, v_height));
		}

		m_nImageWidth = image_Width;
		m_nImageHeight = image_height;

		return COMPV_ERROR_CODE_S_OK;
	}

private:
	bool m_bCalibrationDone;
	size_t m_nImageWidth;
	size_t m_nImageHeight;
	CompVWindowPtr m_ptrWindow;
	CompVCalibCameraPtr m_ptrCalib;
	CompVCalibCameraResult m_CalibResult;
	CompVDrawingOptions m_DrawingOptions;
	CompVMyWindowListenerPtr m_ptrWindowListener;
	CompVMultiSurfaceLayerPtr m_ptrMultiSurface;
	CompVSurfacePtr m_ptrSurfaceOriginal;
	CompVSurfacePtr m_ptrSurfaceEdges;
	CompVSurfacePtr m_ptrSurfaceLinesRaw;
	CompVSurfacePtr m_ptrSurfaceLineGrouped;
	CompVSurfacePtr m_ptrSurfaceUndist;
};

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVMyRunLoopListenerPtr runloopListener;
		CompVCameraPtr camera;
		CompVWindowPtr m_ptrWindow;
		CompVMyCameraListenerPtr cameraListener;
		CompVCameraDeviceInfoList devices;
		std::string cameraId = ""; // empty string means default

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create window
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&m_ptrWindow, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));

		// Create my camera and add a listener to it 
		COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));

		COMPV_CHECK_CODE_BAIL(err = CompVMyCameraListener::newObj(&cameraListener, m_ptrWindow));
		COMPV_CHECK_CODE_BAIL(err = camera->setListener(*cameraListener));
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
		COMPV_CHECK_CODE_BAIL(err = camera->start(cameraId)); // use no parameter ('star()') to use default camera device

		// Start ui runloop
		// Setting a listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
		COMPV_CHECK_CODE_BAIL(err = CompVMyRunLoopListener::newObj(&runloopListener, camera, cameraId));
		COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(*runloopListener));

	bail:
		if (COMPV_ERROR_CODE_IS_NOK(err)) {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Something went wrong!!");
		}
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	compv_main_return(0);
}
