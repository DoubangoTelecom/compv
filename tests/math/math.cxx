#include <compv/compv_api.h>
#include <tchar.h>

using namespace compv;

#if COMPV_OS_WINDOWS
int _tmain(int argc, _TCHAR* argv[])
#else
int main()
#endif
{
    CompVCpu::init();

    COMPV_DEBUG_INFO("CPU flags=%s", CompVCpu::getFlagsAsString());
    COMPV_DEBUG_INFO("Cores count=%d", CompVCpu::getCoresCount());

    getchar();
    return 0;
}

