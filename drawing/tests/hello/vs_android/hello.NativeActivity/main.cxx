#include <compv/compv_api.h>

// FIXME
#include <sys/stat.h>
#include <errno.h>

using namespace compv;

CompVWindowPtr window1, window2;

#if COMPV_OS_ANDROID
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
	ANativeActivity_onCreatePriv(activity, savedState, savedStateSize);
}
#endif

static void* COMPV_STDCALL WorkerThread(void* arg);

static int android_read(void* cookie, char* buf, int size) {
	return AAsset_read((AAsset*)cookie, buf, size);
}

static int android_write(void* cookie, const char* buf, int size) {
	return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void* cookie, fpos_t offset, int whence) {
	return AAsset_seek((AAsset*)cookie, offset, whence);
}

static int android_close(void* cookie) {
	AAsset_close((AAsset*)cookie);
	return 0;
}
static FILE* android_fopen(AAssetManager* assetManager, const char* fname, const char* mode) {
	AAsset* asset = AAssetManager_open(assetManager, fname, 0);
	if (!asset) return NULL;
	return funopen(asset, android_read, android_write, android_seek, android_close);
}

void android_main(struct android_app* state)
{
	COMPV_ERROR_CODE err;

	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

	//FILE* file = android_fopen(state->activity->assetManager, "girl.jpg", "rb");

	// Init the modules
	COMPV_CHECK_CODE_BAIL(err = CompVInit());

	// Create "Hello world!" window
	COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window1, 1080, 1776, "Hello world!"));

	COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(state, WorkerThread));

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
}


static void* COMPV_STDCALL WorkerThread(void* arg)
{
	COMPV_ERROR_CODE err;
	CompVMatPtr mat[3];
	static int count = 0;
	char buff_[33] = { 0 };

#if COMPV_OS_ANDROID
#else
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile("C:/Projects/GitHub/compv/deprecated/tests/girl.jpg", &mat[0]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile("C:/Projects/GitHub/compv/deprecated/tests/valve_original.jpg", &mat[1]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile("C:/Projects/GitHub/compv/deprecated/tests/mandekalou.jpg", &mat[2]));
#endif
	while (CompVDrawing::isLoopRunning()) {
		snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
		std::string text = "Hello Doubango " + std::string(buff_);
		COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->surface()->drawImage(mat[count % 3])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		(COMPV_ERROR_CODE_IS_NOK(err = window1->surface()->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);

		++count;

		// FIXME(dmi):
		//CompVThread::sleep(1);

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