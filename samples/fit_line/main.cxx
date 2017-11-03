#include <compv/compv_api.h>
#include <random>

using namespace compv;

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define NUM_NOISY_POINTS	1300
#define NUM_OUTLIERS		200

static const compv_float32x4_t __color_red = { 1.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_black = { 0.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_blue = { 0.f, 0.f, 1.f, 1.f };

#define __color_background		__color_black
#define __color_points			__color_blue
#define __color_line_perfect	__color_yellow
#define __color_line_fitted		__color_red

#define TAG_SAMPLE	"Fit line"

static COMPV_ERROR_CODE __build_random_points(
	const  compv_float32_t window_width, const  compv_float32_t window_height, 
	CompVLineFloat32& linePerfect,
	CompVPointFloat32Vector& pointsNoisy
) 
{
	const int window_height_int = static_cast<int>(window_height);
	const int window_width_int = static_cast<int>(window_width);
	
	linePerfect.a.x = static_cast<compv_float32_t>(rand() % window_width_int);
	linePerfect.a.y = static_cast<compv_float32_t>(rand() % window_width_int);
	linePerfect.b.x = static_cast<compv_float32_t>(rand() % window_width_int);
	linePerfect.b.y = static_cast<compv_float32_t>(rand() % window_width_int);

	pointsNoisy.clear();

	const compv_float32_t slope = (linePerfect.b.y - linePerfect.a.y) / (linePerfect.b.x - linePerfect.a.x);
	const compv_float32_t intercept = linePerfect.a.y - (slope * linePerfect.a.x);
	const compv_float32_t slope_scale = 1.f / slope;

	COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "linePerfect = (%f, %f)", slope, intercept);

	// Perfect points
	pointsNoisy.resize(NUM_NOISY_POINTS + NUM_OUTLIERS);
	for (CompVPointFloat32Vector::iterator i = pointsNoisy.begin(); i < pointsNoisy.end(); ++i) {
		i->y = static_cast<compv_float32_t>(rand() % window_height_int);
		i->x = (i->y - intercept) * slope_scale;
	}

	// Add gaussian noise
	const compv_float32_t mean = 0.f;
	const compv_float32_t stddev = 8.3f;
	std::default_random_engine generator;
	std::normal_distribution<compv_float32_t> dist(mean, stddev);
	for (CompVPointFloat32Vector::iterator i = pointsNoisy.begin(); i < pointsNoisy.end(); ++i) {
		i->x += dist(generator);
		i->y += dist(generator);
	}

	// Now add 100 outliers
	for (CompVPointFloat32Vector::iterator i = (pointsNoisy.begin() + NUM_NOISY_POINTS); i < pointsNoisy.end(); ++i) {
		i->x += static_cast<compv_float32_t>((rand() % window_width_int) * ((rand() & 1) ? -1 : 1));
		i->y += static_cast<compv_float32_t>((rand() % window_height_int) * ((rand() & 1) ? -1 : 1));
	}

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE __fit_line(
	const  compv_float32_t window_width, const  compv_float32_t window_height,
	const CompVPointFloat32Vector& pointsNoisy,
	CompVLineFloat32& lineFitted
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
	COMPV_CHECK_CODE_RETURN(CompVMathStatsFit::line(ptr32fPointsNoisy, &ptr32fParams));
	const compv_float32_t slope = *ptr32fParams->ptr<const compv_float32_t>(0, 0);
	const compv_float32_t scale = 1.f / slope;
	const compv_float32_t intercept = *ptr32fParams->ptr<const compv_float32_t>(0, 1);
	COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "lineFitted = (%f, %f)", slope, intercept);

	lineFitted.a.y = 0.f;
	lineFitted.a.x = (lineFitted.a.y - intercept) * scale;
	lineFitted.b.y = window_height - 1.f;
	lineFitted.b.x = (lineFitted.b.y - intercept) * scale;

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
		CompVLineFloat32Vector linesPerfect(1);
		CompVLineFloat32Vector linesFitted(1);

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Drawing options
		drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		drawingOptions.pointSize = 2.f;
		drawingOptions.fontSize = 12;
		drawingOptions.lineWidth = 4.f;

		// Create window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create Runnable and execute the task (lane detection)
		onRunning = [&]() -> COMPV_ERROR_CODE {
			COMPV_DEBUG_INFO_CODE_TODO("Uncomment srand");
			//--srand(static_cast<unsigned int>(CompVTime::nowMillis()));
			while (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_BAIL(__build_random_points(
					static_cast<compv_float32_t>(window->width()), static_cast<compv_float32_t>(window->height()),
					linesPerfect[0],
					pointsNoisy
				));
				COMPV_CHECK_CODE_BAIL(__fit_line(
					static_cast<compv_float32_t>(window->width()), static_cast<compv_float32_t>(window->height()),
					pointsNoisy,
					linesFitted[0]
				));
				COMPV_CHECK_CODE_BAIL(window->beginDraw());
				COMPV_CHECK_EXP_BAIL(!(canvas = singleSurfaceLayer->cover()->requestCanvas()), COMPV_ERROR_CODE_E_INVALID_CALL, "Cannot create a canvas for the cover");
				drawingOptions.setColor(__color_background);
				COMPV_CHECK_CODE_BAIL(canvas->clear(&drawingOptions));
				drawingOptions.setColor(__color_line_perfect);
				COMPV_CHECK_CODE_BAIL(canvas->drawLines(linesPerfect, &drawingOptions));
				drawingOptions.setColor(__color_line_fitted);
				COMPV_CHECK_CODE_BAIL(canvas->drawLines(linesFitted, &drawingOptions));
				drawingOptions.setColor(__color_points);
				COMPV_CHECK_CODE_BAIL(canvas->drawPoints(pointsNoisy, &drawingOptions));
				COMPV_CHECK_CODE_BAIL(singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
				COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove the sleep function");
				//CompVThread::sleep(1000); // FIXME(dmi): remove
				getchar();
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