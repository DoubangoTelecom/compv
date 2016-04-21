/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#if !defined(_COMPV_CPU_H_)
#define _COMPV_CPU_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

#define COMPV_CPU_FLAG_BIT(b_) (((uint64_t)1) << (b_))
#define COMPV_CPU_FLAG_IS_SET(flags_, b_) (((uint64_t)(flags_) & COMPV_CPU_FLAG_BIT(b_)) != 0)
// Vendor
static const uint64_t kCpuFlagIntel = COMPV_CPU_FLAG_BIT(0);
static const uint64_t kCpuFlagAMD = COMPV_CPU_FLAG_BIT(1);

// These flags are only valid on ARM processors.
static const uint64_t kCpuFlagARM = COMPV_CPU_FLAG_BIT(5);
static const uint64_t kCpuFlagARM64 = COMPV_CPU_FLAG_BIT(6);
static const uint64_t kCpuFlagNEON = COMPV_CPU_FLAG_BIT(7);
// 0x8 reserved for future ARM flag.

// These flags are only valid on x86/x64 processors.
static const uint64_t kCpuFlagX86 = COMPV_CPU_FLAG_BIT(10);
static const uint64_t kCpuFlagX64 = COMPV_CPU_FLAG_BIT(11);
static const uint64_t kCpuFlagMMX = COMPV_CPU_FLAG_BIT(12);
static const uint64_t kCpuFlagSSE = COMPV_CPU_FLAG_BIT(13);
static const uint64_t kCpuFlagSSE2 = COMPV_CPU_FLAG_BIT(14);
static const uint64_t kCpuFlagSSE3 = COMPV_CPU_FLAG_BIT(15);
static const uint64_t kCpuFlagSSSE3 = COMPV_CPU_FLAG_BIT(16);
static const uint64_t kCpuFlagSSE41 = COMPV_CPU_FLAG_BIT(17);
static const uint64_t kCpuFlagSSE42 = COMPV_CPU_FLAG_BIT(18);
static const uint64_t kCpuFlagSSE4a = COMPV_CPU_FLAG_BIT(19);
static const uint64_t kCpuFlagAVX = COMPV_CPU_FLAG_BIT(20);
static const uint64_t kCpuFlagAVX2 = COMPV_CPU_FLAG_BIT(21);
static const uint64_t kCpuFlagAVX512 = COMPV_CPU_FLAG_BIT(22);
static const uint64_t kCpuFlagFMA3 = COMPV_CPU_FLAG_BIT(23);
static const uint64_t kCpuFlagFMA4 = COMPV_CPU_FLAG_BIT(24);
static const uint64_t kCpuFlagERMS = COMPV_CPU_FLAG_BIT(25);
static const uint64_t kCpuFlagXOP = COMPV_CPU_FLAG_BIT(26);
static const uint64_t kCpuFlagLZCNT = COMPV_CPU_FLAG_BIT(27);
static const uint64_t kCpuFlagPOPCNT = COMPV_CPU_FLAG_BIT(28);
static const uint64_t kCpuFlagCMOV = COMPV_CPU_FLAG_BIT(29); // Conditional move and FCMOV instructions
static const uint64_t kCpuFlagAES = COMPV_CPU_FLAG_BIT(30);
static const uint64_t kCpuFlagRDRAND = COMPV_CPU_FLAG_BIT(31);
static const uint64_t kCpuFlagAVX512_F = COMPV_CPU_FLAG_BIT(32);
static const uint64_t kCpuFlagAVX512_CD = COMPV_CPU_FLAG_BIT(33);
static const uint64_t kCpuFlagAVX512_PF = COMPV_CPU_FLAG_BIT(34);
static const uint64_t kCpuFlagAVX512_ER = COMPV_CPU_FLAG_BIT(35);
static const uint64_t kCpuFlagAVX512_VL = COMPV_CPU_FLAG_BIT(36);
static const uint64_t kCpuFlagAVX512_BW = COMPV_CPU_FLAG_BIT(37);
static const uint64_t kCpuFlagAVX512_DQ = COMPV_CPU_FLAG_BIT(38);
static const uint64_t kCpuFlagAVX512_IFMA = COMPV_CPU_FLAG_BIT(39);
static const uint64_t kCpuFlagAVX512_VBMI = COMPV_CPU_FLAG_BIT(49);

// These flags are only valid on MIPS processors.
static const uint64_t kCpuFlagMIPS = COMPV_CPU_FLAG_BIT(50);
static const uint64_t kCpuFlagMIPS_DSP = COMPV_CPU_FLAG_BIT(51);
static const uint64_t kCpuFlagMIPS_DSPR2 = COMPV_CPU_FLAG_BIT(52);

static const uint64_t kCpuFlagNone = ((uint64_t)0);
static const uint64_t kCpuFlagAll = ~kCpuFlagNone;

class COMPV_API CompVCpu : public CompVObj
{
protected:
    CompVCpu();
public:
    virtual ~CompVCpu();
    static COMPV_ERROR_CODE init();
    static int32_t getCoresCount() {
        return s_iCores;
    }
    static compv_core_id_t getValidCoreId(compv_core_id_t coreId);
    static uint64_t getCyclesCountGlobal();
    static int32_t getCache1LineSize() {
        return s_iCache1LineSize;
    }
    static int32_t getCache1Size() {
        return s_iCache1Size;
    }
    static uint64_t getTimeProcess();
    static uint64_t getFlags() {
        return s_uFlags;
    }
    static uint64_t getFlagsDisabled() {
        return s_uFlagsDisabled;
    }
    static uint64_t getFlagsEnabled() {
        return s_uFlagsEnabled;
    }
    static const char* getFlagsAsString(uint64_t uFlags);
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
    static bool isAsmEnabled() {
        return CompVCpu::s_bAsmEnabled;
    }
    static bool isIntrinsicsEnabled() {
        return CompVCpu::s_bIntrinsicsEnabled;
    }

private:
    static uint64_t s_uFlags;
    static uint64_t s_uFlagsDisabled;
    static uint64_t s_uFlagsEnabled;
    static bool s_bAsmEnabled;
    static bool s_bIntrinsicsEnabled;
    static int32_t s_iCores;
    static int32_t s_iCache1LineSize;
    static int32_t s_iCache1Size;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CPU_H_ */
