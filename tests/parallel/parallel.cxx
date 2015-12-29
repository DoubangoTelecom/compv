#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

static void* COMPV_STDCALL thread_start(void *arg)
{
	COMPV_DEBUG_INFO("Entering thread '%s'", (const char*)arg);
	CompVThread::sleep(1000);
	COMPV_DEBUG_INFO("Exiting thread '%s'", (const char*)arg);
	return NULL;
}

int _tmain(int argc, _TCHAR* argv[])
{
	static const char* thread_names[] = {
		"foo", "bar", "compv", "doubango", "telecom"
	};

	{
		CompVObjWrapper<CompVThread*> thread[sizeof(thread_names) / sizeof(thread_names[0])];
		for (size_t i = 0; i < sizeof(thread_names) / sizeof(thread_names[0]); ++i) {
			CompVThread::newObj(&thread[i], thread_start, (void*)thread_names[i]);
		}
		getchar();
	}

	getchar();

	return 0;
}

