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
#include "compv/compv_thread.h"
#include "compv/compv_debug.h"

#if defined(COMPV_ARCH_X86)
extern "C" int32_t compv_utils_thread_get_core_id_x86_asm();
#endif

COMPV_NAMESPACE_BEGIN()

#define kModuleNameThread "Thread"

CompVThread::CompVThread()
{

}

CompVThread::~CompVThread()
{

}


vcomp_core_id_t CompVThread::getCoreId()
{
#if _WIN32_WINNT >= 0x0600
	return (int32_t)GetCurrentProcessorNumber();
#elif _WIN32_WINNT >= 0x0601
	PROCESSOR_NUMBER ProcNumber;
	GetCurrentProcessorNumberEx(&ProcNumber);
	return  ProcNumber.Number;
#elif defined(COMPV_ARCH_X86_ASM)
	return compv_utils_thread_get_core_id_x86_asm();
#else
	COMPV_DEBUG_ERROR_EX(kModuleNameThread, "Not Implemented yet");
	return 0;
#endif
}

COMPV_NAMESPACE_END()