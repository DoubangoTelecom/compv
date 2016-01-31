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
#if !defined(_COMPV_FEATURES_FAST12_DETE_H_)
#define _COMPV_FEATURES_FAST12_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_debug.h"

#include <algorithm>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif
#if !defined(_COMPV_FEATURES_FAST_DETE_H_)
#error("This code must be included in compv_feature_fast_dete.cxx")
#endif

COMPV_NAMESPACE_BEGIN()

static compv_scalar_t Fast12Strengths_C(const int16_t(&dbrighters)[16], const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, const uint16_t(&FastXFlags)[16])
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	compv_scalar_t ndarker, nbrighter;
	int strength = 0;

	/***  Auto-generated code starts here **/

	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 4095) == 4095) {
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
	}
	if ((fdarkers & 4095) == 4095) {
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
	}
	else if (nbrighter == 255) { goto next0; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next0: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 8190) == 8190) {
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
	}
	if ((fdarkers & 8190) == 8190) {
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
	}
	else if (nbrighter == 255) { goto next1; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next1: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 16380) == 16380) {
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
	}
	if ((fdarkers & 16380) == 16380) {
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
	}
	else if (nbrighter == 255) { goto next2; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next2: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 32760) == 32760) {
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
	}
	if ((fdarkers & 32760) == 32760) {
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
	}
	else if (nbrighter == 255) { goto next3; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next3: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 65520) == 65520) {
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
	}
	if ((fdarkers & 65520) == 65520) {
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
	}
	else if (nbrighter == 255) { goto next4; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next4: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 65505) == 65505) {
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
	}
	if ((fdarkers & 65505) == 65505) {
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
	}
	else if (nbrighter == 255) { goto next5; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next5: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 65475) == 65475) {
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
	}
	if ((fdarkers & 65475) == 65475) {
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
	}
	else if (nbrighter == 255) { goto next6; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next6: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 65415) == 65415) {
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
	}
	if ((fdarkers & 65415) == 65415) {
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
	}
	else if (nbrighter == 255) { goto next7; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next7: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 65295) == 65295) {
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
	}
	if ((fdarkers & 65295) == 65295) {
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
	}
	else if (nbrighter == 255) { goto next8; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next8: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 65055) == 65055) {
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
	}
	if ((fdarkers & 65055) == 65055) {
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
	}
	else if (nbrighter == 255) { goto next9; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next9: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 64575) == 64575) {
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
	}
	if ((fdarkers & 64575) == 64575) {
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
	}
	else if (nbrighter == 255) { goto next10; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next10: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 63615) == 63615) {
		if (dbrighters[11] < nbrighter) nbrighter = dbrighters[11];
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
	}
	if ((fdarkers & 63615) == 63615) {
		if (ddarkers[11] < ndarker) ndarker = ddarkers[11];
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
	}
	else if (nbrighter == 255) { goto next11; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next11: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 61695) == 61695) {
		if (dbrighters[12] < nbrighter) nbrighter = dbrighters[12];
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
	}
	if ((fdarkers & 61695) == 61695) {
		if (ddarkers[12] < ndarker) ndarker = ddarkers[12];
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
	}
	else if (nbrighter == 255) { goto next12; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next12: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 57855) == 57855) {
		if (dbrighters[13] < nbrighter) nbrighter = dbrighters[13];
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
	}
	if ((fdarkers & 57855) == 57855) {
		if (ddarkers[13] < ndarker) ndarker = ddarkers[13];
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
	}
	else if (nbrighter == 255) { goto next13; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next13: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 50175) == 50175) {
		if (dbrighters[14] < nbrighter) nbrighter = dbrighters[14];
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
	}
	if ((fdarkers & 50175) == 50175) {
		if (ddarkers[14] < ndarker) ndarker = ddarkers[14];
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
	}
	else if (nbrighter == 255) { goto next14; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next14: void();
	ndarker = 255;
	nbrighter = 255;
	if ((fbrighters & 34815) == 34815) {
		if (dbrighters[15] < nbrighter) nbrighter = dbrighters[15];
		if (dbrighters[0] < nbrighter) nbrighter = dbrighters[0];
		if (dbrighters[1] < nbrighter) nbrighter = dbrighters[1];
		if (dbrighters[2] < nbrighter) nbrighter = dbrighters[2];
		if (dbrighters[3] < nbrighter) nbrighter = dbrighters[3];
		if (dbrighters[4] < nbrighter) nbrighter = dbrighters[4];
		if (dbrighters[5] < nbrighter) nbrighter = dbrighters[5];
		if (dbrighters[6] < nbrighter) nbrighter = dbrighters[6];
		if (dbrighters[7] < nbrighter) nbrighter = dbrighters[7];
		if (dbrighters[8] < nbrighter) nbrighter = dbrighters[8];
		if (dbrighters[9] < nbrighter) nbrighter = dbrighters[9];
		if (dbrighters[10] < nbrighter) nbrighter = dbrighters[10];
	}
	if ((fdarkers & 34815) == 34815) {
		if (ddarkers[15] < ndarker) ndarker = ddarkers[15];
		if (ddarkers[0] < ndarker) ndarker = ddarkers[0];
		if (ddarkers[1] < ndarker) ndarker = ddarkers[1];
		if (ddarkers[2] < ndarker) ndarker = ddarkers[2];
		if (ddarkers[3] < ndarker) ndarker = ddarkers[3];
		if (ddarkers[4] < ndarker) ndarker = ddarkers[4];
		if (ddarkers[5] < ndarker) ndarker = ddarkers[5];
		if (ddarkers[6] < ndarker) ndarker = ddarkers[6];
		if (ddarkers[7] < ndarker) ndarker = ddarkers[7];
		if (ddarkers[8] < ndarker) ndarker = ddarkers[8];
		if (ddarkers[9] < ndarker) ndarker = ddarkers[9];
		if (ddarkers[10] < ndarker) ndarker = ddarkers[10];
	}
	else if (nbrighter == 255) { goto next15; }
	strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next15: void();

	/***  Auto-generated code ends here **/

	return compv_scalar_t(strength);
}

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FAST12_DETE_H_ */