#include <compv/compv_api.h>
#include <random>

using namespace compv;

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define NUM_NOISY_POINTS	1300
#define NUM_OUTLIERS		500

#define ZERO_MEAN_STDEV		8.3

static const compv_float32x4_t __color_red = { 1.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_green = { 0.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_blue = { 0.f, 0.f, 1.f, 1.f };
static const compv_float32x4_t __color_white = { 1.f, 1.f, 1.f, 1.f };
static const compv_float32x4_t __color_black = { 0.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };


#define __color_background			__color_black
#define __color_text				__color_white
#define __color_points_noisy		__color_blue
#define __color_points_anchor		__color_green
#define __color_parabola_perfect	__color_yellow
#define __color_parabola_fitted		__color_red

#define TAG_SAMPLE	"Fit parabola" 

static COMPV_ERROR_CODE __build_random_points(
	const size_t window_width, const  size_t window_height,
	compv_float32_t (&params_perfect)[3],
	CompVPointFloat32Vector& pointsAnchor,
	CompVPointFloat32Vector& pointsNoisy
)
{
	const int window_height_int = static_cast<int>(window_height);
	const int window_width_int = static_cast<int>(window_width);

	const compv_float32_t x1 = static_cast<compv_float32_t>(rand() % window_width_int);
	const compv_float32_t x2 = static_cast<compv_float32_t>(rand() % window_width_int);
	const compv_float32_t x3 = static_cast<compv_float32_t>(rand() % window_width_int);
	const compv_float32_t y1 = static_cast<compv_float32_t>(rand() % window_height_int);
	const compv_float32_t y2 = static_cast<compv_float32_t>(rand() % window_height_int);
	const compv_float32_t y3 = static_cast<compv_float32_t>(rand() % window_height_int);
	pointsAnchor.resize(3);
	pointsAnchor[0].x = x1;
	pointsAnchor[0].y = y1;
	pointsAnchor[1].x = x2;
	pointsAnchor[1].y = y2;
	pointsAnchor[2].x = x3;
	pointsAnchor[2].y = y3;

	const compv_float32_t scale = 1.f / ((x1 - x2) * (x1 - x3) * (x2 - x3));
	const compv_float32_t A = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) * scale;
	const compv_float32_t B = (x3 * x3 * (y1 - y2) + x2 * x2 * (y3 - y1) + x1 * x1 * (y2 - y3)) * scale;
	const compv_float32_t C = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) * scale;
	params_perfect[0] = A;
	params_perfect[1] = B;
	params_perfect[2] = C;
	COMPV_DEBUG_VERBOSE_EX(TAG_SAMPLE, "Perfect parabola: %f, %f, %f", A, B, C);

	pointsNoisy.resize(NUM_NOISY_POINTS + NUM_OUTLIERS);
	for (CompVPointFloat32Vector::iterator i = pointsNoisy.begin(); i < pointsNoisy.end(); ++i) {
		i->x = static_cast<compv_float32_t>(rand() % window_width_int);
		i->y = (A * (i->x * i->x)) + (B * i->x) + C;
	}
	
	// Add gaussian noise
	const compv_float32_t mean = 0.f;
	const compv_float32_t stddev = static_cast<compv_float32_t>(ZERO_MEAN_STDEV);
	std::default_random_engine generator;
	std::normal_distribution<compv_float32_t> dist(mean, stddev);
	for (CompVPointFloat32Vector::iterator i = pointsNoisy.begin(); i < pointsNoisy.end(); ++i) {
		i->x += dist(generator);
		i->y += dist(generator);
	}

	// Now add outliers
	for (CompVPointFloat32Vector::iterator i = (pointsNoisy.begin() + NUM_NOISY_POINTS); i < pointsNoisy.end(); ++i) {
		i->x += static_cast<compv_float32_t>((rand() % window_width_int) * ((rand() & 1) ? -1 : 1));
		i->y += static_cast<compv_float32_t>((rand() % window_height_int) * ((rand() & 1) ? -1 : 1));
	}

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE __draw_parabola(
	const size_t window_width, const  size_t window_height,
	const compv_float32_t(&params)[3],
	const CompVDrawingOptions* drawingOptions,
	CompVCanvasPtr canvas
)
{
	const compv_float32_t A = params[0];
	const compv_float32_t B = params[1];
	const compv_float32_t C = params[2];

	CompVPointFloat32Vector points(window_width);
	for (size_t x = 0; x < window_width; ++x) {
		const compv_float32_t xf = static_cast<compv_float32_t>(x);
		points[x].x = xf;
		points[x].y = (A * (xf * xf)) + (B * xf) + C;
	}
	COMPV_CHECK_CODE_RETURN(canvas->drawLines(points, drawingOptions));
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE __fit_parabola(
	const size_t window_width, const size_t window_height,
	const CompVPointFloat32Vector& pointsNoisy,
	compv_float32_t(&params_fitted)[3]
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Converting from vector to MatPtr maybe change ransac to accept vector");
	CompVMatPtr ptr32fPointsNoisy;
	const size_t count = pointsNoisy.size();
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&ptr32fPointsNoisy, 2, count));
	compv_float32_t* ptr32fPointsNoisyXPtr = ptr32fPointsNoisy->ptr<compv_float32_t>(0);
	compv_float32_t* ptr32fPointsNoisyYPtr = ptr32fPointsNoisy->ptr<compv_float32_t>(1);
	size_t i = 0;
	CompVPointFloat32Vector::const_iterator j = pointsNoisy.begin();
	for (; i < count; ++i, ++j) {
		ptr32fPointsNoisyXPtr[i] = j->x;
		ptr32fPointsNoisyYPtr[i] = j->y;
	}
	CompVMatPtr ptr32fParams;
	COMPV_CHECK_CODE_RETURN(CompVMathStatsFit::parabola(ptr32fPointsNoisy, ZERO_MEAN_STDEV, &ptr32fParams));
	params_fitted[0] = *ptr32fParams->ptr<const compv_float32_t>(0, 0);
	params_fitted[1] = *ptr32fParams->ptr<const compv_float32_t>(0, 1);
	params_fitted[2] = *ptr32fParams->ptr<const compv_float32_t>(0, 2);
	COMPV_DEBUG_VERBOSE_EX(TAG_SAMPLE, "Fitted parabola: %f, %f, %f", params_fitted[0], params_fitted[1], params_fitted[2]);
	return COMPV_ERROR_CODE_S_OK;
}

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		CompVMatPtr imageFake;
		CompVRunnablePtr runnable;
		CompVRunnableCallbackOnRunning onRunning;
		CompVDrawingOptions drawingOptions;
		CompVCanvasPtr canvas;
		CompVPointFloat32Vector pointsNoisy;
		CompVPointFloat32Vector pointsAnchor;
		compv_float32_t params_perfect[3], params_fitted[3];
		CompVStringVector textFrameNum(1);
		CompVPointFloat32Vector pointFrameNum(1);

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_VERBOSE);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Drawing options
		drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		drawingOptions.fontSize = 12;
		drawingOptions.lineWidth = 4.f;
		drawingOptions.lineLoop = false;

		pointFrameNum[0].x = static_cast<compv_float32_t>(drawingOptions.fontSize);
		pointFrameNum[0].y = static_cast<compv_float32_t>(drawingOptions.fontSize);

		// Create window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create Runnable and execute the task (lane detection)
		onRunning = [&]() -> COMPV_ERROR_CODE {
			COMPV_DEBUG_INFO_CODE_TODO("Uncomment srand");
			//srand(static_cast<unsigned int>(CompVTime::nowMillis()));
			size_t nFrameNum = 0;
			while (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_BAIL(__build_random_points(
					window->width(), window->height(),
					params_perfect,
					pointsAnchor,
					pointsNoisy
				));
				COMPV_CHECK_CODE_BAIL(__fit_parabola(
					window->width(), window->height(),
					pointsNoisy,
					params_fitted
				));
				COMPV_CHECK_CODE_BAIL(window->beginDraw());
				COMPV_CHECK_EXP_BAIL(!(canvas = singleSurfaceLayer->cover()->requestCanvas()), COMPV_ERROR_CODE_E_INVALID_CALL, "Cannot create a canvas for the cover");
				drawingOptions.setColor(__color_background);
				COMPV_CHECK_CODE_BAIL(canvas->clear(&drawingOptions));
				if (canvas->haveDrawTexts()) {
					textFrameNum[0] = std::string("Frame #") + CompVBase::to_string(nFrameNum);
					drawingOptions.setColor(__color_text);
					COMPV_CHECK_CODE_BAIL(canvas->drawTexts(textFrameNum, pointFrameNum, &drawingOptions));
				}
				drawingOptions.setColor(__color_parabola_perfect);
				COMPV_CHECK_CODE_BAIL(__draw_parabola(
					window->width(), window->height(),
					params_perfect,
					&drawingOptions,
					canvas
				));
				drawingOptions.setColor(__color_parabola_fitted);
				COMPV_CHECK_CODE_BAIL(__draw_parabola(
					window->width(), window->height(),
					params_fitted,
					&drawingOptions,
					canvas
				));
				drawingOptions.pointSize = 2.f;
				drawingOptions.setColor(__color_points_noisy);
				COMPV_CHECK_CODE_BAIL(canvas->drawPoints(pointsNoisy, &drawingOptions));
				drawingOptions.pointSize = 10.f;
				drawingOptions.setColor(__color_points_anchor);
				COMPV_CHECK_CODE_BAIL(canvas->drawPoints(pointsAnchor, &drawingOptions));
				COMPV_CHECK_CODE_BAIL(singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
				COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove the sleep function and getchar");
				//CompVThread::sleep(1000); // FIXME(dmi): remove
				if (nFrameNum >= 0) {
					//getchar();
				}
				++nFrameNum;
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_BAIL(err = CompVRunnable::newObj(&runnable, onRunning));

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
				return runnable->start();
			case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
			case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
			case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
				return runnable->stop();
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