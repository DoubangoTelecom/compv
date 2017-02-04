/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/* Partial Copyright for LibYUV project (BSD) for "CompVX86CpuId" and "TestOsSaveYmm" functions */

#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"
#include "compv/base/android/compv_android_cpu-features.h"
#include "compv/base/compv_fileutils.h"

#define COMPV_THIS_CLASSNAME	"CompVCpu"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC long long compv_utils_rdtsc_x86_asm();
#endif

#if COMPV_OS_APPLE || COMPV_OS_BSD || defined(HAVE_SYS_SYSCTL_H)
#   include <sys/sysctl.h>
#elif COMPV_OS_LINUX || defined(HAVE_LINUX_SYSCTL_H)
#   include <linux/sysctl.h>
#endif

#if COMPV_OS_LINUX || defined(HAVE_UNISTD_H) || COMPV_OS_ANDROID
#	include <unistd.h>
#endif

#if defined(_OPENMP) || defined(_OPENMP) || defined(HAVE_OMP_H)
#	include <omp.h>
#endif

#if COMPV_OS_APPLE
#   include <sys/types.h>
#   include <sys/sysctl.h>
#   include <mach/machine.h>
#endif

#if defined(_MSC_VER)
#	pragma intrinsic(__rdtsc, __cpuid)
#endif

COMPV_NAMESPACE_BEGIN()

// Low level cpuid for X86. Returns zeros on other CPUs.
#if !defined(__pnacl__) && !defined(__CLR_VER) &&  (defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__))
static void CompVX86CpuId(uint32_t info_eax, uint32_t info_ecx, uint32_t* cpu_info)
{
#if defined(_MSC_VER) && !defined(__clang__)
#if (_MSC_FULL_VER >= 160040219)
    __cpuidex((int*)(cpu_info), info_eax, info_ecx);
#elif defined(_M_IX86)
    __asm {
        mov        eax, info_eax
        mov        ecx, info_ecx
        mov        edi, cpu_info
        cpuid
        mov[edi], eax
        mov[edi + 4], ebx
        mov[edi + 8], ecx
        mov[edi + 12], edx
    }
#else
    if (info_ecx == 0) {
        __cpuid((int*)(cpu_info), info_eax);
    }
    else {
        cpu_info[3] = cpu_info[2] = cpu_info[1] = cpu_info[0] = 0;
    }
#endif
#else  // defined(_MSC_VER)
    uint32_t info_ebx, info_edx;
    asm volatile (  // NOLINT
#if defined( __i386__) && defined(__PIC__)
        // Preserve ebx for fpic 32 bit.
        "mov %%ebx, %%edi                          \n"
        "cpuid                                     \n"
        "xchg %%edi, %%ebx                         \n"
        : "=D" (info_ebx),
#else
        "cpuid                                     \n"
        : "=b" (info_ebx),
#endif  //  defined( __i386__) && defined(__PIC__)
        "+a" (info_eax), "+c" (info_ecx), "=d" (info_edx));
    cpu_info[0] = info_eax;
    cpu_info[1] = info_ebx;
    cpu_info[2] = info_ecx;
    cpu_info[3] = info_edx;
#endif  // defined(_MSC_VER)
}

#if !defined(__native_client__)
#define HAS_XGETBV
// X86 CPUs have xgetbv to detect OS saves high parts of ymm registers.
int TestOsSaveYmm()
{
    uint32_t xcr0 = 0u;
#if defined(_MSC_VER) && (_MSC_FULL_VER >= 160040219)
    xcr0 = (uint32_t)(_xgetbv(0));  // VS2010 SP1 required.
#elif defined(_M_IX86) && defined(_MSC_VER)
    __asm {
        xor        ecx, ecx    // xcr 0
        _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0  // For VS2010 and earlier.
        mov        xcr0, eax
    }
#elif defined(__i386__) || defined(__x86_64__)
    asm(".byte 0x0f, 0x01, 0xd0" : "=a" (xcr0) : "c" (0) : "%edx");
#endif  // defined(_MSC_VER)
    return((xcr0 & 6) == 6);  // Is ymm saved?
}
#endif  // !defined(__native_client__)
#elif COMPV_ARCH_X86 || COMPV_ARCH_X64
static void CompVX86CpuId(uint32_t eax, uint32_t ecx, uint32_t* cpu_info)
{
    cpu_info[0] = cpu_info[1] = cpu_info[2] = cpu_info[3] = 0;
}
#endif

#if COMPV_ARCH_ARM && !COMPV_OS_ANDROID && !COMPV_OS_IPHONE
static uint64_t CompVArmCaps(const char* cpuinfo_name)
{
    char cpuinfo_line[1024];
#if COMPV_ARCH_ARM64
	uint64_t flags = kCpuFlagARM64;
#else
	uint64_t flags = kCpuFlagARM;
#endif
    FILE* f = fopen(cpuinfo_name, "r");
    if (!f) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to open '/proc/cpuinfo'");
        return flags;
    }
    while (fgets(cpuinfo_line, sizeof(cpuinfo_line) - 1, f)) {
        if (memcmp(cpuinfo_line, "Features", 8) == 0) {
            char* p = strstr(cpuinfo_line, " neon");
			flags |= (p && (p[5] == ' ' || p[5] == '\n')) ? kCpuFlagARM_NEON : kCpuFlagNone;
			p = strstr(cpuinfo_line, " vfpv3");
			flags |= (p && (p[6] == ' ' || p[6] == '\n')) ? kCpuFlagARM_VFPv3 : kCpuFlagNone;
			p = strstr(cpuinfo_line, " vfpv4");
			flags |= (p && (p[6] == ' ' || p[6] == '\n')) ? kCpuFlagARM_VFPv4 : kCpuFlagNone;
            
            // aarch64 uses asimd for Neon
            p = strstr(cpuinfo_line, " asimd");
			flags |= (p && (p[6] == ' ' || p[6] == '\n')) ? kCpuFlagARM_NEON : kCpuFlagNone;

			if (flags & kCpuFlagARM64) {
				flags |= kCpuFlagARM_NEON_FMA | kCpuFlagARM_VFPv4; // required on Aarch64
			}
			if ((flags & kCpuFlagARM_NEON) && (flags & kCpuFlagARM_VFPv4)) {
				flags |= kCpuFlagARM_NEON_FMA;
			}

			fclose(f);
			return flags;
        }
    }
    fclose(f);
    return flags;
}
#endif

#if defined(COMPV_ARCH_MIPS) && defined(__linux__)
static uint64_t CompVMipsCaps(const char* search_string)
{
    char cpuinfo_line[512];
    const char* file_name = "/proc/cpuinfo";
    FILE* f = fopen(file_name, "r");
    if (!f) {
        // Assume DSP if /proc/cpuinfo is unavailable.
        // This will occur for Chrome sandbox for Pepper or Render process.
        return kCpuFlagMIPS_DSP;
    }
    while (fgets(cpuinfo_line, sizeof(cpuinfo_line) - 1, f) != NULL) {
        if (strstr(cpuinfo_line, search_string) != NULL) {
            fclose(f);
            return kCpuFlagMIPS_DSP;
        }
    }
    fclose(f);
    return 0;
}
#endif


uint64_t CompVCpu::s_uFlags = 0;
uint64_t CompVCpu::s_uFlagsDisabled = 0;
uint64_t CompVCpu::s_uFlagsEnabled = 0;
int32_t CompVCpu::s_iCores = 0;
int32_t CompVCpu::s_iCache1LineSize = 0;
int32_t CompVCpu::s_iCache1Size = 0;
bool CompVCpu::s_bInitialized = false;
#if COMPV_ASM
bool CompVCpu::s_bAsmEnabled = true;
#else
bool CompVCpu::s_bAsmEnabled = false;
#endif
#if COMPV_INTRINSIC
bool CompVCpu::s_bIntrinsicsEnabled = true;
#else
bool CompVCpu::s_bIntrinsicsEnabled = false;
#endif
bool CompVCpu::s_bMathTrigFast = true;
bool CompVCpu::s_bMathFixedPoint = true;
#if TARGET_RT_LITTLE_ENDIAN
bool CompVCpu::s_bBigEndian = false;
#elif TARGET_RT_BIG_ENDIAN
bool CompVCpu::s_bBigEndian = true;
#else
bool CompVCpu::s_bBigEndian = false;
#endif
bool CompVCpu::s_bIntelIpp = false;

CompVCpu::CompVCpu()
{

}

CompVCpu::~CompVCpu()
{

}

COMPV_ERROR_CODE CompVCpu::init()
{
	if (s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	//
	// /proc/cpuinfo
	//
#if COMPV_OS_LINUX || COMPV_OS_BSD || COMPV_OS_ANDROID
	if (CompVDebugMgr::getLevel() >= COMPV_DEBUG_LEVEL_INFO) {
		FILE* fcpuinfo = fopen("/proc/cpuinfo", "r");
		if (fcpuinfo) {
			char cpuinfo_line[1024];
			size_t count;
			while ((count = fread(cpuinfo_line, 1, sizeof(cpuinfo_line), fcpuinfo)) > 0) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME "/proc/cpuinfo", "%.*s", static_cast<int>(count), cpuinfo_line);
			}
			fclose(fcpuinfo);
		}
	}
#endif

	//
	// endianness
	//
	// https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/64bitPorting/MakingCode64-BitClean/MakingCode64-BitClean.html
#if TARGET_RT_LITTLE_ENDIAN
	s_bBigEndian = false;
#elif TARGET_RT_BIG_ENDIAN
	s_bBigEndian = true;
#else
	static const short kWord = 0x4321;
	s_bBigEndian = ((*(int8_t *)&kWord) != 0x21);
#	if COMPV_OS_WINDOWS
	if (s_bBigEndian) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Big endian on Windows machine. Is it right?");
	}
#	endif
#endif

    //
    // CPUID
    //
#if !defined(__CLR_VER) && COMPV_ARCH_X86
    // https://en.wikipedia.org/wiki/CPUID
    CompVCpu::s_uFlags = kCpuFlagX86;
    char vname[13] = { '\0' };
    uint32_t cpu_info[4] = { 0, 0, 0, 0 };
#define I_EAX 0
#define I_EBX 1
#define I_ECX 2
#define I_EDX 3

    CompVX86CpuId(0x00000000, 0, cpu_info);
    memcpy(&vname[0], &cpu_info[1], 4);
    memcpy(&vname[4], &cpu_info[3], 4);
    memcpy(&vname[8], &cpu_info[2], 4);
    if (stricmp("GenuineIntel", vname) == 0) {
        CompVCpu::s_uFlags |= kCpuFlagIntel;
    }
    else if (stricmp("AuthenticAMD", vname) == 0) {
        CompVCpu::s_uFlags |= kCpuFlagAMD;
    }

    CompVX86CpuId(0x80000000, 0, cpu_info);
    uint32_t info0 = cpu_info[0];
    if (info0 >= 0x00000001) {
        CompVX86CpuId(0x00000001, 0, cpu_info); // EAX = 1h
        CompVCpu::s_uFlags |=
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EDX], 23) ? kCpuFlagMMX : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EDX], 25) ? kCpuFlagSSE : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EDX], 26) ? kCpuFlagSSE2 : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EDX], 15) ? kCpuFlagCMOV : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 0) ? kCpuFlagSSE3 : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 9) ? kCpuFlagSSSE3 : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 19) ? kCpuFlagSSE41 : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 20) ? kCpuFlagSSE42 : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 23) ? kCpuFlagPOPCNT : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 25) ? kCpuFlagAES : 0) |

            ((COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 28) && TestOsSaveYmm()) ? kCpuFlagAVX : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 12) ? kCpuFlagFMA3 : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 30) ? kCpuFlagRDRAND : 0);
    }
    if (info0 >= 0x00000007) {
        CompVX86CpuId(0x00000007, 0, cpu_info); // EAX = 7h

        CompVCpu::s_uFlags |=
            ((cpu_info[1] & 0x00000200) ? kCpuFlagERMS : 0) |
            ((COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 5) && TestOsSaveYmm()) ? kCpuFlagAVX2 : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 16) ? kCpuFlagAVX512_F : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 28) ? kCpuFlagAVX512_CD : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 26) ? kCpuFlagAVX512_PF : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 27) ? kCpuFlagAVX512_ER : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 31) ? kCpuFlagAVX512_VL : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 30) ? kCpuFlagAVX512_BW : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 17) ? kCpuFlagAVX512_DQ : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EBX], 21) ? kCpuFlagAVX512_IFMA : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 1) ? kCpuFlagAVX512_VBMI : 0);

    }
    if (info0 >= 0x80000001) {
        CompVX86CpuId(0x80000001, 0, cpu_info); // EAX = 80000001h

        CompVCpu::s_uFlags |=
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_EDX], 29) ? kCpuFlagX64 : 0) |

            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 6) ? kCpuFlagSSE4a : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 16) ? kCpuFlagFMA4 : 0) |
            (COMPV_CPU_FLAG_IS_SET(cpu_info[I_ECX], 11) ? kCpuFlagXOP : 0);
    }
    if (info0 >= 0x80000005) {
        // CompVX86CpuId(0x80000005, 0, cpu_info); // EAX=80000005h: L1 Cache and TLB Identifiers
        // http://www.intel.fr/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf
        // Section 7.7.3 Deterministic Cache Parameters
        // (# of Ways) * (Partitions) * (Line_size) * (Sets) = (EBX[31:22] + 1) * (EBX[21:12] + 1) * (EBX[11:0] + 1) * (ECX+1)
        //#define kMaskNofWays	0
        //#define kMaskLineSize	0x7FF

        //uint32_t lineSize = (cpu_info[I_EBX] & kMaskLineSize) + 1;
        //if (lineSize) {
        //	printf("Line size: %u", lineSize);
        //}
    }

#elif COMPV_ARCH_ARM || COMPV_ARCH_ARM64
    CompVCpu::s_uFlags = kCpuFlagARM;
#   if COMPV_ARCH_ARM64
    CompVCpu::s_uFlags |= kCpuFlagARM64;
#   endif
    
#	if COMPV_OS_ANDROID
	if (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM64) {
		CompVCpu::s_uFlags |= kCpuFlagARM64;
	}
	uint64_t android_flags = android_getCpuFeatures();
    if (android_flags & ANDROID_CPU_ARM_FEATURE_NEON) {
        CompVCpu::s_uFlags |= kCpuFlagARM_NEON;
		if (CompVCpu::s_uFlags & kCpuFlagARM64) {
			CompVCpu::s_uFlags |= kCpuFlagARM_NEON_FMA | kCpuFlagARM_VFPv4; // required on Aarch64
		}
    }
    if (android_flags & ANDROID_CPU_ARM_FEATURE_NEON_FMA) {
        CompVCpu::s_uFlags |= kCpuFlagARM_NEON_FMA | kCpuFlagARM_VFPv4;
    }
	if (android_flags & ANDROID_CPU_ARM_FEATURE_VFP_FMA) {
		CompVCpu::s_uFlags |= kCpuFlagARM_VFPv4;
	}
	if (android_flags & ANDROID_CPU_ARM_FEATURE_VFPv3) {
		CompVCpu::s_uFlags |= kCpuFlagARM_VFPv3;
	}
#   elif COMPV_OS_APPLE
    int aret;
    size_t size;
    cpu_type_t type;
    cpu_subtype_t subtype;
    
    size = sizeof(type);
    if ((aret = sysctlbyname("hw.cputype", &type, &size, NULL, 0)) == 0) {
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "sysctlbyname(hw.cpusubtype): %d", static_cast<int>(type));
        if ((type & CPU_TYPE_ARM64) == CPU_TYPE_ARM64) {
            // All ARM64 (v7, v8 and upcomming v9) devices support neon and vfpv4
            // Later we check the cpu subtype but it could mistach when apple adds when major version (e.g. CPU_SUBTYPE_ARM64_V9)
            CompVCpu::s_uFlags |= kCpuFlagARM64 | kCpuFlagARM_NEON | kCpuFlagARM_NEON_FMA | kCpuFlagARM_VFPv4 | kCpuFlagARM_VFPv3;
        }
        if ((type & CPU_TYPE_ARM) == CPU_TYPE_ARM) {
            CompVCpu::s_uFlags |= kCpuFlagARM;
        }
    }
    else {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sysctlbyname(hw.cputype) failed: %d", aret);
    }
    size = sizeof(subtype);
    if (sysctlbyname("hw.cpusubtype", &subtype, &size, NULL, 0) == 0) {
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "sysctlbyname(hw.cpusubtype): %d", static_cast<int>(subtype));
        if ((subtype & CPU_SUBTYPE_ARM_V7) == CPU_SUBTYPE_ARM_V7 || (subtype & CPU_SUBTYPE_ARM_V8) == CPU_SUBTYPE_ARM_V8 || (subtype & CPU_SUBTYPE_ARM64_V8) == CPU_SUBTYPE_ARM64_V8) {
            CompVCpu::s_uFlags |= kCpuFlagARM_NEON | kCpuFlagARM_NEON_FMA | kCpuFlagARM_VFPv4 | kCpuFlagARM_VFPv3;
        }
    }
    else {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sysctlbyname(hw.cpusubtype) failed: %d", aret);
    }
    
    
    
#	else
    CompVCpu::s_uFlags |= CompVArmCaps("/proc/cpuinfo");
#endif
#elif COMPV_ARCH_MIPS && defined(__linux__)
    CompVCpu::s_uFlags = kCpuFlagMIPS;
    // Linux mips parse text file for dsp detect.
    CompVCpu::s_uFlags |= CompVMipsCaps("dsp");  // set kCpuFlagMIPS_DSP.
#	if defined(__mips_dspr2)
    CompVCpu::s_uFlags |= kCpuFlagMIPS_DSPR2;
#	endif // __mips_dspr2
#else
    CompVCpu::s_uFlags = 0;
#endif

    // Remove disabled flags
    CompVCpu::s_uFlagsEnabled = (CompVCpu::s_uFlags & ~CompVCpu::s_uFlagsDisabled);

    //
    //	Cores
    //
#if COMPV_OS_WINDOWS
    SYSTEM_INFO SystemInfo;
#	if COMPV_OS_WINDOWS_RT
    GetNativeSystemInfo(&SystemInfo);
#	else
    GetSystemInfo(&SystemInfo);
#	endif
    s_iCores = SystemInfo.dwNumberOfProcessors;
#elif defined(_OPENMP) || defined(_OPENMP) || defined(HAVE_OMP_H)
    s_iCores = omp_get_num_procs();
#elif COMPV_OS_APPLE
    size_t len = sizeof(s_iCores);
    int mib0[2] = { CTL_HW, HW_NCPU };
    sysctl(mib0, 2, &s_iCores, &len, NULL, 0);
#elif COMPV_OS_ANDROID
    s_iCores = static_cast<int32_t>(android_getCpuCount());
#elif defined(__GNUC__)
    s_iCores = static_cast<int32_t>(sysconf(_SC_NPROCESSORS_ONLN));
#else
    COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "coresCount function not implemented ...using 1 as default value");
    s_iCores = 1;
#endif

    //
    // Cache size
    //
#if COMPV_OS_WINDOWS
    DWORD bs = 0;
    if (!GetLogicalProcessorInformation(0, &bs)) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buff = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(bs);
        DWORD i;
        GetLogicalProcessorInformation(buff, &bs);
        for (i = 0; i != bs / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
            if (buff[i].Relationship == RelationCache && buff[i].Cache.Level == 1) {
                s_iCache1LineSize = buff[i].Cache.LineSize;
                s_iCache1Size = buff[i].Cache.Size;
                break;
            }
        }
        if (buff) {
            free(buff);
        }
    }
    else {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "GetLogicalProcessorInformation() failed with error code = %08x", GetLastError());
        s_iCache1LineSize = 64;
        s_iCache1Size = 4096;
    }

#elif COMPV_OS_APPLE
    size_t sizeof_cls = sizeof(s_iCache1LineSize);
    size_t sizeof_cs = sizeof(s_iCache1Size);
    int mib1[2] = { CTL_HW, HW_CACHELINE };
    int mib2[2] = { CTL_HW, HW_L1ICACHESIZE };

    sysctl(mib1, 2, &s_iCache1LineSize, &sizeof_cls, NULL, 0);
    sysctl(mib2, 2, &s_iCache1Size, &sizeof_cs, NULL, 0);
#elif COMPV_OS_ANDROID
    COMPV_DEBUG_INFO_CODE_ONCE("_SC_LEVEL1_DCACHE_LINESIZE and _SC_LEVEL1_DCACHE_SIZE not availabe and Android");
    s_iCache1LineSize = COMPV_CACHE1_LINE_SIZE;
    s_iCache1Size = 4096;
#else
    s_iCache1LineSize = (int32_t)sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    s_iCache1Size = (int32_t)sysconf(_SC_LEVEL1_DCACHE_SIZE);
#endif

	s_bInitialized = true;

    return COMPV_ERROR_CODE_S_OK;
}

const char* CompVCpu::flagsAsString(uint64_t uFlags)
{
    static std::string _flags;
    struct {
        uint64_t f;
        const char* name;
    } flags[] = {
        { kCpuFlagIntel, "(intel)" },
        { kCpuFlagAMD, "(amd)" },

        // These flags are only valid on ARM processors.
        { kCpuFlagARM, "[arm]" },
        { kCpuFlagARM64, "[arm64]" },
        { kCpuFlagARM_NEON, "neon" },
        { kCpuFlagARM_NEON_FMA, "neon_fma"},
        { kCpuFlagARM_VFPv3, "vfpv3" },
		{ kCpuFlagARM_VFPv4, "vfpv4" },
        // -- reserved for future ARM flag.

        // These flags are only valid on x86/x64 processors.
        { kCpuFlagX86, "[x86]" },
        { kCpuFlagX64, "[x64]" },
        { kCpuFlagMMX, "mmx" },
        { kCpuFlagSSE, "sse" },
        { kCpuFlagSSE2, "sse2" },
        { kCpuFlagSSE3, "sse3" },
        { kCpuFlagSSSE3, "ssse3" },
        { kCpuFlagSSE41, "sse41" },
        { kCpuFlagSSE42, "sse42" },
        { kCpuFlagSSE4a, "sse4a" },
        { kCpuFlagAVX, "avx" },
        { kCpuFlagAVX2, "avx2" },
        { kCpuFlagAVX512, "avx512" },
        { kCpuFlagFMA3, "fma3" },
        { kCpuFlagFMA4, "fma4" },
        { kCpuFlagERMS, "erms" },
        { kCpuFlagXOP, "xop" },
        { kCpuFlagLZCNT, "lzcnt" },
        { kCpuFlagPOPCNT, "popcnt" },
        { kCpuFlagCMOV, "cmov" },
        { kCpuFlagAES, "aes" },
        { kCpuFlagRDRAND, "rdrand" },
        { kCpuFlagAVX512_F, "avx512_f" },
        { kCpuFlagAVX512_CD, "avx512_cd" },
        { kCpuFlagAVX512_PF, "avx512_pf" },
        { kCpuFlagAVX512_ER, "avx512_er" },
        { kCpuFlagAVX512_VL, "avx512_vl" },
        { kCpuFlagAVX512_BW, "avx512_bw" },
        { kCpuFlagAVX512_DQ, "avx512_dq" },
        { kCpuFlagAVX512_IFMA, "avx512_ifma" },
        { kCpuFlagAVX512_VBMI, "avx512_vbmi" },

        // These flags are only valid on MIPS processors.
        { kCpuFlagMIPS, "[mips]" },
        { kCpuFlagMIPS_DSP, "mips_dsp" },
        { kCpuFlagMIPS_DSPR2, "mips_dspr2" },
    };

    _flags = "";
    for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
        if ((flags[i].f & uFlags) == flags[i].f) {
            _flags += std::string(flags[i].name) + ";";
        }
    }
    return _flags.empty() ? "none" : _flags.c_str();
}



compv_core_id_t CompVCpu::validCoreId(compv_core_id_t coreId)
{
    if (coreId < 0) {
        return 0;
    }
    else {
        return coreId % CompVCpu::coresCount();
    }
}

uint64_t CompVCpu::cyclesCountGlobal()
{
#if COMPV_OS_WINDOWS
    return __rdtsc();
#elif COMPV_ARCH_X86_ASM
    return compv_utils_rdtsc_x86_asm();
#endif
    COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "cyclesCountGlobal function not implemented ...returning zero");
    return 0;
}

/*
Gets CPU time.
Returned value is in milliseconds
*/
uint64_t CompVCpu::timeProcess()
{
#if COMPV_OS_WINDOWS
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        SYSTEMTIME systemTime;
        if (FileTimeToSystemTime(&userTime, &systemTime)) {
            return (uint64_t)(((systemTime.wHour * 3600) + (systemTime.wMinute * 60) + systemTime.wSecond) * 1000 + systemTime.wMilliseconds);
        }
        else {
            COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "FileTimeToSystemTime() failed with error code = %08x", GetLastError());
        }
    }
    else {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "GetProcessTimes() failed with error code = %08x", GetLastError());
    }
    return 0;
#else
    COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "timeProcess not implemented: use CLOCK_PROCESS_CPUTIME_ID");
    return 0;
#endif
}

COMPV_ERROR_CODE CompVCpu::flagsDisable(uint64_t flags)
{
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Disabled CPU flags: %s", flagsAsString(flags));
    CompVCpu::s_uFlagsDisabled = flags;
    CompVCpu::s_uFlagsEnabled = (CompVCpu::s_uFlags & ~CompVCpu::s_uFlagsDisabled);
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Enabled CPU flags: %s", flagsAsString(CompVCpu::s_uFlagsEnabled));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCpu::flagsEnable(uint64_t flags)
{
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Enabled CPU flags: %s", flagsAsString(flags));
    s_uFlagsDisabled &= ~flags;
    CompVCpu::s_uFlagsEnabled = (CompVCpu::s_uFlags & ~CompVCpu::s_uFlagsDisabled);
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Enabled CPU flags: %s", flagsAsString(CompVCpu::s_uFlagsEnabled));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCpu::setAsmEnabled(bool bEnabled)
{
    if (bEnabled) {
#if COMPV_ASM
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Enabling asm code");
		CompVCpu::s_bAsmEnabled = true;
#else
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Code source was not build with ASM. You can't enable ASM at runtime");
		return COMPV_ERROR_CODE_E_INVALID_CALL;
#endif
    }
    else {
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Disabling asm code");
        CompVCpu::s_bAsmEnabled = false;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCpu::setIntrinsicsEnabled(bool bEnabled)
{
    if (bEnabled) {
#if COMPV_INTRINSIC
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Enabling intrinsic code");
		CompVCpu::s_bIntrinsicsEnabled = true;
#else
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Code source was not build with intrinsics. You can't enable intrinsics at runtime");
		return COMPV_ERROR_CODE_E_INVALID_CALL;
#endif
    }
    else {
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Disabling intrinsic code");
        CompVCpu::s_bIntrinsicsEnabled = false;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCpu::setMathTrigFastEnabled(bool bMathTrigFast)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU math trig. fast = %s", bMathTrigFast ? "true" : "false");
	s_bMathTrigFast = bMathTrigFast;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCpu::setMathFixedPointEnabled(bool bMathFixedPoint)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU math trig. fast = %s", bMathFixedPoint ? "true" : "false");
	s_bMathFixedPoint = bMathFixedPoint;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCpu::setIntelIppEnabled(bool bIntelIpp)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intel IPP = %s", bIntelIpp ? "true" : "false");
#if !COMPV_HAVE_INTEL_IPP
	COMPV_CHECK_EXP_RETURN(bIntelIpp, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Trying to enable Intel IPP but the code not built with support for this feature");
#endif /* COMPV_HAVE_INTEL_IPP */
	s_bIntelIpp = bIntelIpp;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
