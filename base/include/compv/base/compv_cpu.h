/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_CPU_H_)
#define _COMPV_BASE_CPU_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__GNUC__)
#	define __compv_builtin_prefetch_read(ptr) __builtin_prefetch(ptr, 0, 0)
#	define __compv_builtin_prefetch_write(ptr) __builtin_prefetch(ptr, 1, 0)
#elif COMPV_ARCH_X86 && COMPV_INTRINSIC
#	define __compv_builtin_prefetch_read(ptr) _mm_prefetch(reinterpret_cast<char const*>((ptr)), _MM_HINT_T0) // This is old SSE1, available on all x86 since 1999
#	define __compv_builtin_prefetch_write(ptr) 
#else
#	define __compv_builtin_prefetch_read(ptr) 
#	define __compv_builtin_prefetch_write(ptr) 
#endif

#if defined(__GNUC__)
#	define __compv_builtin_assume_aligned(ptr, alignment) __builtin_assume_aligned((ptr), (alignment))
#else
#	define __compv_builtin_assume_aligned(ptr, alignment) (ptr)
#endif

#define COMPV_CPU_FLAG_BIT(b_) (((uint64_t)1) << (b_))
#define COMPV_CPU_FLAG_IS_SET(flags_, b_) (((uint64_t)(flags_) & COMPV_CPU_FLAG_BIT(b_)) != 0)
// Vendor
static const uint64_t kCpuFlagIntel = COMPV_CPU_FLAG_BIT(0);
static const uint64_t kCpuFlagAMD = COMPV_CPU_FLAG_BIT(1);
// [2-4] -- reserved for future vendors

// These flags are only valid on ARM processors.
static const uint64_t kCpuFlagARM = COMPV_CPU_FLAG_BIT(5);
static const uint64_t kCpuFlagARM64 = COMPV_CPU_FLAG_BIT(6);
static const uint64_t kCpuFlagARM_NEON = COMPV_CPU_FLAG_BIT(7);
static const uint64_t kCpuFlagARM_NEON_FMA = COMPV_CPU_FLAG_BIT(8); // NEON + VFP4 (see compv_android_cpu-features.cxx)
static const uint64_t kCpuFlagARM_VFPv3 = COMPV_CPU_FLAG_BIT(9);
static const uint64_t kCpuFlagARM_VFPv4 = COMPV_CPU_FLAG_BIT(10);
// [11-13] -- reserved for future ARM flag

// These flags are only valid on x86/x64 processors.
static const uint64_t kCpuFlagX86 = COMPV_CPU_FLAG_BIT(14);
static const uint64_t kCpuFlagX64 = COMPV_CPU_FLAG_BIT(15);
static const uint64_t kCpuFlagMMX = COMPV_CPU_FLAG_BIT(16);
static const uint64_t kCpuFlagSSE = COMPV_CPU_FLAG_BIT(17);
static const uint64_t kCpuFlagSSE2 = COMPV_CPU_FLAG_BIT(18);
static const uint64_t kCpuFlagSSE3 = COMPV_CPU_FLAG_BIT(19);
static const uint64_t kCpuFlagSSSE3 = COMPV_CPU_FLAG_BIT(20);
static const uint64_t kCpuFlagSSE41 = COMPV_CPU_FLAG_BIT(21);
static const uint64_t kCpuFlagSSE42 = COMPV_CPU_FLAG_BIT(22);
static const uint64_t kCpuFlagSSE4a = COMPV_CPU_FLAG_BIT(23);
static const uint64_t kCpuFlagAVX = COMPV_CPU_FLAG_BIT(24);
static const uint64_t kCpuFlagAVX2 = COMPV_CPU_FLAG_BIT(25);
static const uint64_t kCpuFlagAVX512 = COMPV_CPU_FLAG_BIT(26);
static const uint64_t kCpuFlagFMA3 = COMPV_CPU_FLAG_BIT(27);
static const uint64_t kCpuFlagFMA4 = COMPV_CPU_FLAG_BIT(28);
static const uint64_t kCpuFlagERMS = COMPV_CPU_FLAG_BIT(29);
static const uint64_t kCpuFlagXOP = COMPV_CPU_FLAG_BIT(30);
static const uint64_t kCpuFlagLZCNT = COMPV_CPU_FLAG_BIT(31); // same as "bsf" which is alwayas available
static const uint64_t kCpuFlagBMI1 = COMPV_CPU_FLAG_BIT(32); // Bit Manipulation Instruction Set 1 (andn, bextr, blsi, blsmsk, blsr, tzcnt)
static const uint64_t kCpuFlagBMI2 = COMPV_CPU_FLAG_BIT(33); // Bit Manipulation Instruction Set 2 (bzhi, mulx, pdep, pext, rorx, sarx, shrx, shlx)
static const uint64_t kCpuFlagPOPCNT = COMPV_CPU_FLAG_BIT(34);
static const uint64_t kCpuFlagCMOV = COMPV_CPU_FLAG_BIT(35); // Conditional move and FCMOV instructions
static const uint64_t kCpuFlagAES = COMPV_CPU_FLAG_BIT(36);
static const uint64_t kCpuFlagRDRAND = COMPV_CPU_FLAG_BIT(37);
static const uint64_t kCpuFlagAVX512_F = COMPV_CPU_FLAG_BIT(38);
static const uint64_t kCpuFlagAVX512_CD = COMPV_CPU_FLAG_BIT(39);
static const uint64_t kCpuFlagAVX512_PF = COMPV_CPU_FLAG_BIT(40);
static const uint64_t kCpuFlagAVX512_ER = COMPV_CPU_FLAG_BIT(41);
static const uint64_t kCpuFlagAVX512_VL = COMPV_CPU_FLAG_BIT(42);
static const uint64_t kCpuFlagAVX512_BW = COMPV_CPU_FLAG_BIT(43);
static const uint64_t kCpuFlagAVX512_DQ = COMPV_CPU_FLAG_BIT(44);
static const uint64_t kCpuFlagAVX512_IFMA = COMPV_CPU_FLAG_BIT(45);
static const uint64_t kCpuFlagAVX512_VBMI = COMPV_CPU_FLAG_BIT(46);
// [46-50] -- reserved for future x86/x64 flags

// These flags are only valid on MIPS processors.
static const uint64_t kCpuFlagMIPS = COMPV_CPU_FLAG_BIT(50);
static const uint64_t kCpuFlagMIPS_DSP = COMPV_CPU_FLAG_BIT(51);
static const uint64_t kCpuFlagMIPS_DSPR2 = COMPV_CPU_FLAG_BIT(52);

// [53-63] -- reserved for future flags

static const uint64_t kCpuFlagNone = ((uint64_t)0);
static const uint64_t kCpuFlagAll = ~kCpuFlagNone;

class COMPV_BASE_API CompVCpu : public CompVObj
{
protected:
    CompVCpu();
public:
    virtual ~CompVCpu();
    static COMPV_ERROR_CODE init();
	static const std::string& hardware() {
		return s_strHardware;
	}
	static const std::string& serial() {
		return s_strSerial;
	}
    static size_t coresCount() {
        return s_iCores;
    }
    static compv_core_id_t validCoreId(compv_core_id_t coreId);
    static uint64_t cyclesCountGlobal();
    static size_t cache1LineSize() { // In Bytes
        return s_iCache1LineSize;
    }
    static size_t cache1Size() { // In Bytes
        return s_iCache1Size;
    }
	static size_t physMemSize() { // In Bytes
		return s_iPhysMemSize;
	}
    static uint64_t timeProcess();
    static uint64_t getFlags() {
        return s_uFlags;
    }
    static uint64_t flagsDisabled() {
        return s_uFlagsDisabled;
    }
    static uint64_t flagsEnabled() {
        return s_uFlagsEnabled;
    }
    static const std::string flagsAsString(uint64_t uFlags);
    static bool isEnabled(uint64_t flag) {
        return (s_uFlagsEnabled & flag) == flag;
    }
    static bool isSupported(uint64_t flag) {
        return (s_uFlags & flag) == flag;
    }
    static COMPV_ERROR_CODE flagsDisable(uint64_t flags);
    static COMPV_ERROR_CODE flagsEnable(uint64_t flags);
    static COMPV_ERROR_CODE setAsmEnabled(bool bEnabled);
    static COMPV_ERROR_CODE setIntrinsicsEnabled(bool bEnabled);
	static COMPV_ERROR_CODE setMathTrigFastEnabled(bool bMathTrigFast);
	static COMPV_ERROR_CODE setMathFixedPointEnabled(bool bMathFixedPoint);
	static COMPV_ERROR_CODE setIntelIppEnabled(bool bIntelIpp);
	static bool isBigEndian() {
		COMPV_CHECK_EXP_NOP(!s_bInitialized, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
		return s_bBigEndian;
	}
	static bool isLittleEndian() { return !isBigEndian(); }
    static bool isAsmEnabled() { return CompVCpu::s_bAsmEnabled; }
    static bool isIntrinsicsEnabled() { return CompVCpu::s_bIntrinsicsEnabled; }
	static bool isMathTrigFastEnabled() { return s_bMathTrigFast; }
	static bool isMathFixedPointEnabled() { return s_bMathFixedPoint; }
	static bool isIntelIppEnabled() { return s_bIntelIpp; }
	static bool isInitialized() { return s_bInitialized; }
	
private:
	static bool s_bInitialized;
	static std::string s_strHardware;
	static std::string s_strSerial;
    static uint64_t s_uFlags;
    static uint64_t s_uFlagsDisabled;
    static uint64_t s_uFlagsEnabled;
	static bool s_bBigEndian;
    static bool s_bAsmEnabled;
    static bool s_bIntrinsicsEnabled;
	static bool s_bMathTrigFast;
	static bool s_bMathFixedPoint;
	static bool s_bIntelIpp;
    static size_t s_iCores;
    static size_t s_iCache1LineSize; // In Bytes
    static size_t s_iCache1Size; // In Bytes
	static size_t s_iPhysMemSize; // In Bytes
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_CPU_H_ */
