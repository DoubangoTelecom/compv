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
#include "compv/compv_hamming.h"
#include "compv/compv_bits.h"
#include "compv/compv_debug.h"

#if COMPV_ARCH_X86 && COMPV_ASM
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X64 && COMPV_ASM
#endif /* COMPV_ARCH_X64 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

/*
dataPtr: The pointer to the data for which we want to compute the hamming distance. Hamming distance will be compute for each width-bytes.
width: The number of bytes to use to compute each hamming distance value.
stride: The stride value.
height: The number of rows.
patch1xnPtr: The pointer to the patch. The patch is used as sliding window over the rows to compute the hamming distances. It must be a (1 x width) array.
distPtr: The results for each row. It must be a (1 x height) array.
Algorithm:
for (row = 0; row < height; ++row) 
	distPtr[row] = hamming(dataPtr[row], key1xnPtr);
*/
COMPV_ERROR_CODE CompVHamming::distance(const uint8_t* dataPtr, int width, int stride, int height, const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	COMPV_CHECK_EXP_RETURN(!dataPtr || !width || width > stride || !height || !patch1xnPtr || !distPtr, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	int i, pad = (stride - width);
	uint64_t pop;
	int32_t cnt;

	// FIXME: M$ specific

	for (int j = 0; j < height; ++j) {
		i = width;
		cnt = 0;
		while (i > 7) {
			pop = *((uint64_t*)dataPtr) ^ *((const uint64_t*)&patch1xnPtr[width - i]);
			cnt += (int32_t)__popcnt64(pop);
			dataPtr += 8;
			i -= 8;
		}
		if (i > 3) {
			pop = *((uint32_t*)dataPtr) ^ *((const uint32_t*)&patch1xnPtr[width - i]);
			cnt += (int32_t)__popcnt((uint32_t)pop);
			dataPtr += 4;
			i -= 4;
		}
		if (i > 1) {
			pop = *((uint16_t*)dataPtr) ^ *((const uint16_t*)&patch1xnPtr[width - i]);
			cnt += (int32_t)__popcnt16((uint16_t)pop);
			dataPtr += 2;
			i -= 2;
		}
		if (i) {
			pop = *((uint8_t*)dataPtr) ^ *((const uint8_t*)&patch1xnPtr[width - i]);
			cnt += (int32_t)kPopcnt256[pop];
			dataPtr += 1;
			i -= 1;
		}

		// FIXME: remove
		COMPV_ASSERT(i == 0);

		dataPtr += pad;
		distPtr[j] = cnt;
	}
	
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
