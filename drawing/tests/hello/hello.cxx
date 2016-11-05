#include <compv/compv_api.h>

#if COMPV_OS_WINDOWS
#include <tchar.h>
#endif

using namespace compv;

CompVWindowPtr window1, window2;

static void* COMPV_STDCALL WorkerThread(void* arg);

#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char** argv)
#endif
{
	COMPV_ERROR_CODE err;
	
	// Init the modules
	COMPV_CHECK_CODE_BAIL(err = CompVInit());

	// Create "Hello world!" window
	COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window1, 640, 480, "Hello world!"));
    //COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window2, 640, 480, "Hello france!"));

	COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(WorkerThread));

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("Something went wrong!!");
	}

	window1 = NULL;
    window2 = NULL;
	
	// DeInit the modules
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	// Make sure we freed all allocated memory
	COMPV_ASSERT(CompVMem::isEmpty());
	// Make sure we freed all allocated objects
	COMPV_ASSERT(CompVObj::isEmpty());
    
	return 0;
}

static void* COMPV_STDCALL WorkerThread(void* arg)
{
	COMPV_ERROR_CODE err;
	CompVMatPtr mat[3];
	static int count = 0;
	
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile("C:/Projects/GitHub/compv/deprecated/tests/girl.jpg", &mat[0]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile("C:/Projects/GitHub/compv/deprecated/tests/Valve_original.jpg", &mat[1]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile("C:/Projects/GitHub/compv/deprecated/tests/mandekalou.jpg", &mat[2]));
	while (CompVDrawing::isLoopRunning()) {
		std::string text = "Hello Doubango " + std::to_string(count);
		COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->surface()->drawImage(mat[count % 3])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->surface()->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);

		++count;

		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->drawImage(mat[/*index % 3*/0])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->test(mat[/*index % 3*/0])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window2->draw(mat[/*index++ % 3*/0])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//if(count==1)break;
	}

bail:
	return NULL;
}
