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

static COMPV_INLINE void Fast12Strengths1_C(COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x1, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x1, const compv::compv_scalar_t fbrighters1, const compv::compv_scalar_t fdarkers1, uint8_t* strengths1, compv::compv_scalar_t N)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    compv_scalar_t ndarker, nbrighter;
    int strength = 0;

    /***  Auto-generated code starts here **/

    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 4095) == 4095) {
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
    }
    if ((fdarkers1 & 4095) == 4095) {
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
    }
    else if (nbrighter == 255) {
        goto next0;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next0:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 8190) == 8190) {
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
    }
    if ((fdarkers1 & 8190) == 8190) {
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
    }
    else if (nbrighter == 255) {
        goto next1;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next1:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 16380) == 16380) {
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
    }
    if ((fdarkers1 & 16380) == 16380) {
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
    }
    else if (nbrighter == 255) {
        goto next2;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next2:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 32760) == 32760) {
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
    }
    if ((fdarkers1 & 32760) == 32760) {
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
    }
    else if (nbrighter == 255) {
        goto next3;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next3:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 65520) == 65520) {
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
    }
    if ((fdarkers1 & 65520) == 65520) {
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
    }
    else if (nbrighter == 255) {
        goto next4;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next4:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 65505) == 65505) {
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
    }
    if ((fdarkers1 & 65505) == 65505) {
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
    }
    else if (nbrighter == 255) {
        goto next5;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next5:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 65475) == 65475) {
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
    }
    if ((fdarkers1 & 65475) == 65475) {
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
    }
    else if (nbrighter == 255) {
        goto next6;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next6:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 65415) == 65415) {
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
    }
    if ((fdarkers1 & 65415) == 65415) {
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
    }
    else if (nbrighter == 255) {
        goto next7;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next7:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 65295) == 65295) {
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
    }
    if ((fdarkers1 & 65295) == 65295) {
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
    }
    else if (nbrighter == 255) {
        goto next8;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next8:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 65055) == 65055) {
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
    }
    if ((fdarkers1 & 65055) == 65055) {
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
    }
    else if (nbrighter == 255) {
        goto next9;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next9:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 64575) == 64575) {
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
    }
    if ((fdarkers1 & 64575) == 64575) {
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
    }
    else if (nbrighter == 255) {
        goto next10;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next10:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 63615) == 63615) {
        if (dbrighters16x1[11] < nbrighter) {
            nbrighter = dbrighters16x1[11];
        }
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
    }
    if ((fdarkers1 & 63615) == 63615) {
        if (ddarkers16x1[11] < ndarker) {
            ndarker = ddarkers16x1[11];
        }
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
    }
    else if (nbrighter == 255) {
        goto next11;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next11:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 61695) == 61695) {
        if (dbrighters16x1[12] < nbrighter) {
            nbrighter = dbrighters16x1[12];
        }
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
    }
    if ((fdarkers1 & 61695) == 61695) {
        if (ddarkers16x1[12] < ndarker) {
            ndarker = ddarkers16x1[12];
        }
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
    }
    else if (nbrighter == 255) {
        goto next12;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next12:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 57855) == 57855) {
        if (dbrighters16x1[13] < nbrighter) {
            nbrighter = dbrighters16x1[13];
        }
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
    }
    if ((fdarkers1 & 57855) == 57855) {
        if (ddarkers16x1[13] < ndarker) {
            ndarker = ddarkers16x1[13];
        }
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
    }
    else if (nbrighter == 255) {
        goto next13;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next13:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 50175) == 50175) {
        if (dbrighters16x1[14] < nbrighter) {
            nbrighter = dbrighters16x1[14];
        }
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
    }
    if ((fdarkers1 & 50175) == 50175) {
        if (ddarkers16x1[14] < ndarker) {
            ndarker = ddarkers16x1[14];
        }
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
    }
    else if (nbrighter == 255) {
        goto next14;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next14:
    void();
    ndarker = 255;
    nbrighter = 255;
    if ((fbrighters1 & 34815) == 34815) {
        if (dbrighters16x1[15] < nbrighter) {
            nbrighter = dbrighters16x1[15];
        }
        if (dbrighters16x1[0] < nbrighter) {
            nbrighter = dbrighters16x1[0];
        }
        if (dbrighters16x1[1] < nbrighter) {
            nbrighter = dbrighters16x1[1];
        }
        if (dbrighters16x1[2] < nbrighter) {
            nbrighter = dbrighters16x1[2];
        }
        if (dbrighters16x1[3] < nbrighter) {
            nbrighter = dbrighters16x1[3];
        }
        if (dbrighters16x1[4] < nbrighter) {
            nbrighter = dbrighters16x1[4];
        }
        if (dbrighters16x1[5] < nbrighter) {
            nbrighter = dbrighters16x1[5];
        }
        if (dbrighters16x1[6] < nbrighter) {
            nbrighter = dbrighters16x1[6];
        }
        if (dbrighters16x1[7] < nbrighter) {
            nbrighter = dbrighters16x1[7];
        }
        if (dbrighters16x1[8] < nbrighter) {
            nbrighter = dbrighters16x1[8];
        }
        if (dbrighters16x1[9] < nbrighter) {
            nbrighter = dbrighters16x1[9];
        }
        if (dbrighters16x1[10] < nbrighter) {
            nbrighter = dbrighters16x1[10];
        }
    }
    if ((fdarkers1 & 34815) == 34815) {
        if (ddarkers16x1[15] < ndarker) {
            ndarker = ddarkers16x1[15];
        }
        if (ddarkers16x1[0] < ndarker) {
            ndarker = ddarkers16x1[0];
        }
        if (ddarkers16x1[1] < ndarker) {
            ndarker = ddarkers16x1[1];
        }
        if (ddarkers16x1[2] < ndarker) {
            ndarker = ddarkers16x1[2];
        }
        if (ddarkers16x1[3] < ndarker) {
            ndarker = ddarkers16x1[3];
        }
        if (ddarkers16x1[4] < ndarker) {
            ndarker = ddarkers16x1[4];
        }
        if (ddarkers16x1[5] < ndarker) {
            ndarker = ddarkers16x1[5];
        }
        if (ddarkers16x1[6] < ndarker) {
            ndarker = ddarkers16x1[6];
        }
        if (ddarkers16x1[7] < ndarker) {
            ndarker = ddarkers16x1[7];
        }
        if (ddarkers16x1[8] < ndarker) {
            ndarker = ddarkers16x1[8];
        }
        if (ddarkers16x1[9] < ndarker) {
            ndarker = ddarkers16x1[9];
        }
        if (ddarkers16x1[10] < ndarker) {
            ndarker = ddarkers16x1[10];
        }
    }
    else if (nbrighter == 255) {
        goto next15;
    }
    strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
next15:
    void();

    /***  Auto-generated code ends here **/

    *strengths1 = (uint8_t)strength;
}

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FAST12_DETE_H_ */