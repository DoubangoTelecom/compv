#include <compv/compv_api.h>

using namespace compv;

#if COMPV_OS_WINDOWS
#	define COMPV_SAMPLE_IMAGE_FOLDER			"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_SAMPLE_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_SAMPLE_IMAGE_FOLDER			NULL
#endif

#define CAMERA_IDX				0
#define CAMERA_WIDTH			1280
#define CAMERA_HEIGHT			720
#define CAMERA_FPS				30
#define CAMERA_SUBTYPE			COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS		true // autofocus should be disabled on final product

#define WINDOW_WIDTH			1280
#define WINDOW_HEIGHT			720

#define NONMAXIMA				true
#define THRESHOLD				20
#define FAST_TYPE				COMPV_FAST_TYPE_9
// TODO(dmi): On ARM use 500 features to decrease CPU usage
#define MAXFEATURES				2000 // use negative value to retain all features and improve accuracy (more cpu usage!!)
#define PYRAMID_LEVELS			8
#define PYRAMID_SCALE_FACTOR	0.83f // (1 / 1.2)

#define KNN						2 // Should be 2 unless you know what you're doing
#define NORM					COMPV_BRUTEFORCE_NORM_HAMMING
#define CROSS_CHECK				true

#define KNN_RATIO_TEST			0.67 // http://www.cs.ubc.ca/~lowe/papers/ijcv04.pdf#page=20
#define THRESHOLD_GOOD_MATCHES	8

#define TRAIN_FILE_NAME			"opengl_programming_guide_8th_edition_200x258_rgb.rgb" // Should not use large train image to avoid high CPU usage for nothing (not performance gain for matching rate)
#define TRAIN_WIDTH				200
#define TRAIN_HEIGHT			258
#define TRAIN_STRIDE			200

#define TAG_SAMPLE				"Object Recognition App"

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
class CompVMyCameraListener : public CompVCameraListener
{
protected:
	CompVMyCameraListener(CompVWindowPtr ptrWindow, CompVMatchingSurfaceLayerPtr ptrMatchingSurfaceLayer)
		: m_ptrWindow(ptrWindow), m_ptrMatchingSurfaceLayer(ptrMatchingSurfaceLayer) { }
public:
	virtual ~CompVMyCameraListener() { }

	virtual COMPV_ERROR_CODE onNewFrame(const CompVMatPtr& image) override {
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		// TODO(dmi): 'drawImage' is a pure GPU function while 'FAST->process' is a pure CPU function -> can be done on parallel
		if (CompVDrawing::isLoopRunning()) {
			// Conver image to grayscalr
			COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &m_ptrImageGrayQuery), "Failed to convert the image to grayscale"); // convert the image to grayscale once (otherwise will be converted in detector and descriptor)
			// Get matches
			COMPV_CHECK_CODE_RETURN(match());
			bool recognized = m_vecGoodMatches.size() >= THRESHOLD_GOOD_MATCHES;
			// Homography
			if (recognized) {
				COMPV_CHECK_CODE_RETURN(homography());
			}
			/* Drawing */
			COMPV_CHECK_CODE_BAIL(err = m_ptrWindow->beginDraw());
			// Matched points
			COMPV_CHECK_CODE_BAIL(err = m_ptrMatchingSurfaceLayer->drawMatches(m_ptrImageTrain, m_ptrGoodMatchesTrain, image, m_ptrGoodMatchesQuery, &m_DrawingOptionsMatches));
					
			// Bounding box arround the recognized object
			if (recognized) {
				// TODO(dmi): DrawText("Object Recognized!");
				static size_t num = 0;								
				const compv_float64_t* x = m_ptrRectMatched->ptr<compv_float64_t>(0);
				const compv_float64_t* y = m_ptrRectMatched->ptr<compv_float64_t>(1);
				const compv_float64_t xoffset = static_cast<compv_float64_t>(m_ptrImageTrain->cols() + COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET);
				// using drawLines instead of drawRectangles because the rectangle doesn't have square angles (90 degrees)
				CompVLineFloat32Vector lines(4);
				CompVLineFloat32& line0 = lines[0];
				CompVLineFloat32& line1 = lines[1];
				CompVLineFloat32& line2 = lines[2];
				CompVLineFloat32& line3 = lines[3];
				line0.a.x = static_cast<compv_float32_t>(x[0] + xoffset), line0.a.y = static_cast<compv_float32_t>(y[0]), line0.b.x = static_cast<compv_float32_t>(x[1] + xoffset), line0.b.y = static_cast<compv_float32_t>(y[1]);
				line1.a.x = static_cast<compv_float32_t>(x[1] + xoffset), line1.a.y = static_cast<compv_float32_t>(y[1]), line1.b.x = static_cast<compv_float32_t>(x[2] + xoffset), line1.b.y = static_cast<compv_float32_t>(y[2]);
				line2.a.x = static_cast<compv_float32_t>(x[2] + xoffset), line2.a.y = static_cast<compv_float32_t>(y[2]), line2.b.x = static_cast<compv_float32_t>(x[3] + xoffset), line2.b.y = static_cast<compv_float32_t>(y[3]);
				line3.a.x = static_cast<compv_float32_t>(x[3] + xoffset), line3.a.y = static_cast<compv_float32_t>(y[3]), line3.b.x = static_cast<compv_float32_t>(x[0] + xoffset), line3.b.y = static_cast<compv_float32_t>(y[0]);
				COMPV_CHECK_CODE_BAIL(err = m_ptrMatchingSurfaceLayer->surface()->canvas()->drawLines(lines, &m_DrawingOptions));
				if (m_ptrMatchingSurfaceLayer->surface()->canvas()->haveDrawTexts()) {
					CompVStringVector text(1);
					CompVPointFloat32Vector pos(1);
					text[0] = std::string("Object Recognized(") + CompVBase::to_string(num++) + std::string(")!");
					pos[0] = line0.a;
					COMPV_CHECK_CODE_BAIL(err = m_ptrMatchingSurfaceLayer->surface()->canvas()->drawTexts(text, pos, &m_DrawingOptions));
				}
				else {
					COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Object Recognized (%zu)!", num++);
				}
			}

			COMPV_CHECK_CODE_BAIL(err = m_ptrMatchingSurfaceLayer->blit());
		bail:
			COMPV_CHECK_CODE_NOP(err = m_ptrWindow->endDraw()); // Make sure 'endDraw()' will be called regardless the result
		}
		return err;
	}

	virtual COMPV_ERROR_CODE onError(const std::string& message) override {
		COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Camera error: %s", message.c_str()); // probably a disconnect
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE newObj(CompVMyCameraListenerPtrPtr listener, CompVWindowPtr ptrWindow, CompVMatchingSurfaceLayerPtr ptrMatchingSurfaceLayer) {
		COMPV_CHECK_EXP_RETURN(!listener || !ptrWindow || !ptrMatchingSurfaceLayer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMyCameraListenerPtr listener_ = new CompVMyCameraListener(ptrWindow, ptrMatchingSurfaceLayer);
		COMPV_CHECK_EXP_RETURN(!listener_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

		// ORB detector
		COMPV_CHECK_CODE_RETURN(CompVCornerDete::newObj(&listener_->m_ptrDeteORB, COMPV_ORB_ID));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrDeteORB->setInt(COMPV_ORB_SET_INT_FAST_THRESHOLD, THRESHOLD));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrDeteORB->setInt(COMPV_ORB_SET_INT_INTERNAL_DETE_ID, FAST_TYPE));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrDeteORB->setInt(COMPV_ORB_SET_INT_MAX_FEATURES, MAXFEATURES));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrDeteORB->setBool(COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP, NONMAXIMA));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrDeteORB->setInt(COMPV_ORB_SET_INT_PYRAMID_LEVELS, PYRAMID_LEVELS));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrDeteORB->setFloat32(COMPV_ORB_SET_FLT32_PYRAMID_SCALE_FACTOR, PYRAMID_SCALE_FACTOR));
		
		// ORB descriptor
		COMPV_CHECK_CODE_RETURN(CompVCornerDesc::newObj(&listener_->m_ptrDescORB, COMPV_ORB_ID, listener_->m_ptrDeteORB));

		// Bruteforce matcher
		COMPV_CHECK_CODE_RETURN(CompVMatcher::newObj(&listener_->m_ptrMatcher, COMPV_BRUTEFORCE_ID));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMatcher->setInt(COMPV_BRUTEFORCE_SET_INT_KNN, KNN));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMatcher->setInt(COMPV_BRUTEFORCE_SET_INT_NORM, NORM));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrMatcher->setBool(COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK, (CROSS_CHECK && KNN == 1)));

		// Init train
		COMPV_CHECK_CODE_RETURN(listener_->initTrain());

		// Set drawing options
		listener_->m_DrawingOptionsMatches.colorType = COMPV_DRAWING_COLOR_TYPE_RANDOM;
		listener_->m_DrawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		listener_->m_DrawingOptions.color[0] = 1.f;
		listener_->m_DrawingOptions.color[1] = 1.f;
		listener_->m_DrawingOptions.color[2] = 0.f;
		listener_->m_DrawingOptions.color[3] = 1.f;
		listener_->m_DrawingOptions.lineWidth = 4.f;
		listener_->m_DrawingOptions.fontSize = 32;

		// Set output
		*listener = listener_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	COMPV_ERROR_CODE initTrain()
	{
		std::string path = COMPV_PATH_FROM_NAME(TRAIN_FILE_NAME); // path from android's assets, iOS' bundle....
		// The path isn't correct when the binary is loaded from another process(e.g. when Intel VTune is used)
		if (!CompVFileUtils::exists(path.c_str())) {
			path = std::string(COMPV_SAMPLE_IMAGE_FOLDER) + std::string("/") + std::string(TRAIN_FILE_NAME);
		}
		// Build interest points and descriptions
		COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB24, TRAIN_WIDTH, TRAIN_HEIGHT, TRAIN_STRIDE, path.c_str(), &m_ptrImageTrain));
		COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(m_ptrImageTrain, &m_ptrImageGrayTrain), "Failed to convert the image to grayscale");
		COMPV_CHECK_CODE_RETURN(m_ptrDeteORB->process(m_ptrImageGrayTrain, m_vecInterestPointsTrain));
		COMPV_CHECK_CODE_RETURN(m_ptrDescORB->process(m_ptrImageGrayTrain, m_vecInterestPointsTrain, &m_ptrDescriptionsTrain));
		// Build rectangle (homogeneous coords.)
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrRectTrain, 3, 4));
		COMPV_CHECK_CODE_RETURN(m_ptrRectTrain->one_row<compv_float64_t>(2)); // with Z = 1
		compv_float64_t* x = m_ptrRectTrain->ptr<compv_float64_t>(0);
		compv_float64_t* y = m_ptrRectTrain->ptr<compv_float64_t>(1);
		x[0] = 0, x[1] = static_cast<compv_float64_t>(m_ptrImageGrayTrain->cols()), x[2] = static_cast<compv_float64_t>(m_ptrImageGrayTrain->cols()), x[3] = 0;
		y[0] = 0, y[1] = 0, y[2] = static_cast<compv_float64_t>(m_ptrImageGrayTrain->rows()), y[3] = static_cast<compv_float64_t>(m_ptrImageGrayTrain->rows());
		return COMPV_ERROR_CODE_S_OK;
	}

	static bool myfunction(const CompVInterestPoint& i, const CompVInterestPoint& j) { return (i.strength > j.strength); }

	COMPV_ERROR_CODE match()
	{
		m_vecGoodMatches.clear();

		// Detect and describe the features
		COMPV_CHECK_CODE_RETURN(m_ptrDeteORB->process(m_ptrImageGrayQuery, m_vecInterestPointsQuery));
		if (!m_vecInterestPointsQuery.empty()) { // this is a sample app and we want to display all matched points even is the object isn't in the scene, in final app make sure "m_vecInterestPointsQuery.size() > THRESHOLD_GOOD_MATCHES"
			COMPV_CHECK_CODE_RETURN(m_ptrDescORB->process(m_ptrImageGrayQuery, m_vecInterestPointsQuery, &m_ptrDescriptionsQuery));
			if (!m_ptrDescriptionsQuery->isEmpty()) {
				COMPV_CHECK_CODE_RETURN(m_ptrMatcher->process(m_ptrDescriptionsQuery, m_ptrDescriptionsTrain, &m_ptrMatches));
				// Filter the matches to get the good ones
#if KNN == 2
				const CompVDMatch *match1 = m_ptrMatches->ptr<const CompVDMatch>(0), *match2 = m_ptrMatches->ptr<const CompVDMatch>(1);
				size_t count = COMPV_MATH_MIN(m_ptrDescriptionsQuery->rows() - 1, m_ptrMatches->cols());
				for (size_t i = 0; i < count; i++) {
					if (match1[i].distance < KNN_RATIO_TEST * match2[i].distance) {
						m_vecGoodMatches.push_back(match1[i]);
					}
				}
#else
				COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "This code should not be called unless you know what you're doing");
				const CompVDMatch* match = m_ptrMatches->ptr<const CompVDMatch>(0);
				size_t count = COMPV_MATH_MIN(m_ptrDescriptionsQuery->rows() - 1, m_ptrMatches->cols());
				for (size_t i = 0; i < count; i++) {
					if (match[i].distance <= 35) {
						m_vecGoodMatches.push_back(match[i]);
					}
				}
#endif

				// Build match points
				if (m_vecGoodMatches.size() > 0) { // this is a sample app and we want to display all matched points even is the object isn't in the scene, in final app make sure "m_vecGoodMatches.size() > THRESHOLD_GOOD_MATCHES"
					COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrGoodMatchesQuery, 3, m_vecGoodMatches.size()));
					COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrGoodMatchesTrain, 3, m_vecGoodMatches.size()));
					COMPV_CHECK_CODE_RETURN(m_ptrGoodMatchesQuery->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
					COMPV_CHECK_CODE_RETURN(m_ptrGoodMatchesTrain->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
					compv_float64_t* queryX = m_ptrGoodMatchesQuery->ptr<compv_float64_t>(0);
					compv_float64_t* queryY = m_ptrGoodMatchesQuery->ptr<compv_float64_t>(1);
					compv_float64_t* trainX = m_ptrGoodMatchesTrain->ptr<compv_float64_t>(0);
					compv_float64_t* trainY = m_ptrGoodMatchesTrain->ptr<compv_float64_t>(1);
					CompVInterestPoint queryPoint, trainPoint;
					for (size_t i = 0; i < m_vecGoodMatches.size(); i++) {
						queryPoint = m_vecInterestPointsQuery[m_vecGoodMatches[i].queryIdx];
						trainPoint = m_vecInterestPointsTrain[m_vecGoodMatches[i].trainIdx];
						queryX[i] = queryPoint.x;
						queryY[i] = queryPoint.y;
						trainX[i] = trainPoint.x;
						trainY[i] = trainPoint.y;
					}
				}
			}
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE homography()
	{
		COMPV_CHECK_EXP_RETURN(m_vecGoodMatches.size() < THRESHOLD_GOOD_MATCHES, COMPV_ERROR_CODE_E_INVALID_CALL, "No enough points");
		
		// Find homography
		uint64_t timeStart = CompVTime::nowMillis();
		COMPV_CHECK_CODE_RETURN(CompVHomography<compv_float64_t>::find(m_ptrGoodMatchesTrain, m_ptrGoodMatchesQuery, &m_ptrHomography));
		uint64_t timeEnd = CompVTime::nowMillis();
		COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Homography Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));
		// Perspecive transform using homography matrix
		COMPV_CHECK_CODE_RETURN(CompVMathTransform<compv_float64_t>::perspective2D(m_ptrRectTrain, m_ptrHomography, &m_ptrRectMatched));
		
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	CompVWindowPtr m_ptrWindow;
	CompVMatchingSurfaceLayerPtr m_ptrMatchingSurfaceLayer;
	CompVCornerDetePtr m_ptrDeteORB;
	CompVCornerDescPtr m_ptrDescORB;
	CompVMatcherPtr m_ptrMatcher;
	CompVMatPtr m_ptrImageGrayQuery;
	CompVMatPtr m_ptrImageGrayTrain;
	CompVMatPtr m_ptrImageTrain;
	CompVMatPtr m_ptrDescriptionsQuery;
	CompVMatPtr m_ptrDescriptionsTrain;
	CompVMatPtr m_ptrGoodMatchesQuery;
	CompVMatPtr m_ptrGoodMatchesTrain;
	CompVMatPtr m_ptrMatches;
	CompVMatPtr m_ptrHomography;
	CompVMatPtr m_ptrRectTrain;
	CompVMatPtr m_ptrRectMatched; // cartz corrds
	std::vector<CompVDMatch> m_vecGoodMatches;
	CompVInterestPointVector m_vecInterestPointsQuery;
	CompVInterestPointVector m_vecInterestPointsTrain;
	CompVDrawingOptions m_DrawingOptionsMatches;
	CompVDrawingOptions m_DrawingOptions;
};

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVMyRunLoopListenerPtr runloopListener;
		CompVMatchingSurfaceLayerPtr matchingSurfaceLayer;
		CompVCameraPtr camera;
		CompVMyCameraListenerPtr cameraListener;
		CompVCameraDeviceInfoList devices;
		std::string cameraId = ""; // empty string means default

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create "Hello world!" window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addMatchingLayerSurface(&matchingSurfaceLayer));

		// Create my camera and add a listener to it 
		COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));

		COMPV_CHECK_CODE_BAIL(err = CompVMyCameraListener::newObj(&cameraListener, window, *matchingSurfaceLayer));
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
