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

/* @description
This class implement FAST (Features from Accelerated Segment Test) algorithm.
Some literature about FAST:
	- http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/AV1011/AV1FeaturefromAcceleratedSegmentTest.pdf
	- https://en.wikipedia.org/wiki/Features_from_accelerated_segment_test
	- http://www.edwardrosten.com/work/fast.html
	- http://web.eecs.umich.edu/~silvio/teaching/EECS598_2010/slides/11_16_Hao.pdf
*/
#include "compv/features/fast/compv_feature_fast_dete.h"
#include "compv/compv_mathutils.h"

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>

COMPV_NAMESPACE_BEGIN()

CompVFeatureDeteFAST::CompVFeatureDeteFAST()
    : m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
    , m_iN(COMPV_FEATURE_DETE_FAST_N_DEFAULT)
    , m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
{
}

CompVFeatureDeteFAST::~CompVFeatureDeteFAST()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDeteFAST::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_SET_INT32_FAST_THRESHOLD: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        int32_t threshold = *((int32_t*)valuePtr);
        m_iThreshold = COMPV_MATH_CLIP3(0, 255, threshold);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_SET_BOOL_FAST_NON_MAXIMA_SUPP: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_bNonMaximaSupp = *((bool*)valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

typedef struct {
    int x, y;
} xy;
typedef unsigned char byte;


int fast9_corner_score(const byte* p, const int pixel[], int bstart)
{
    int bmin = bstart;
    int bmax = 255;
    int b = (bmax + bmin) / 2;

    /*Compute the score using binary search*/
    for (;;) {
        int cb = *p + b;
        int c_b = *p - b;


        if (p[pixel[0]] > cb)
            if (p[pixel[1]] > cb)
                if (p[pixel[2]] > cb)
                    if (p[pixel[3]] > cb)
                        if (p[pixel[4]] > cb)
                            if (p[pixel[5]] > cb)
                                if (p[pixel[6]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[7]] < c_b)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[14]] < c_b)
                                            if (p[pixel[8]] < c_b)
                                                if (p[pixel[9]] < c_b)
                                                    if (p[pixel[10]] < c_b)
                                                        if (p[pixel[11]] < c_b)
                                                            if (p[pixel[12]] < c_b)
                                                                if (p[pixel[13]] < c_b)
                                                                    if (p[pixel[15]] < c_b) {
                                                                        goto is_a_corner;
                                                                    }
                                                                    else {
                                                                        goto is_not_a_corner;
                                                                    }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[6]] < c_b)
                                    if (p[pixel[15]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[13]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b)
                                                            if (p[pixel[11]] < c_b)
                                                                if (p[pixel[12]] < c_b)
                                                                    if (p[pixel[14]] < c_b) {
                                                                        goto is_a_corner;
                                                                    }
                                                                    else {
                                                                        goto is_not_a_corner;
                                                                    }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b)
                                                            if (p[pixel[13]] < c_b)
                                                                if (p[pixel[14]] < c_b) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[13]] < c_b)
                                    if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b)
                                                            if (p[pixel[14]] < c_b)
                                                                if (p[pixel[15]] < c_b) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[5]] < c_b)
                                if (p[pixel[14]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb)
                                                                if (p[pixel[11]] > cb) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[12]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b)
                                                            if (p[pixel[11]] < c_b)
                                                                if (p[pixel[13]] < c_b) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[14]] < c_b)
                                    if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b)
                                                            if (p[pixel[13]] < c_b)
                                                                if (p[pixel[6]] < c_b) {
                                                                    goto is_a_corner;
                                                                }
                                                                else if (p[pixel[15]] < c_b) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[6]] < c_b)
                                    if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b)
                                                            if (p[pixel[13]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[12]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb)
                                                            if (p[pixel[11]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[12]] < c_b)
                                if (p[pixel[7]] < c_b)
                                    if (p[pixel[8]] < c_b)
                                        if (p[pixel[9]] < c_b)
                                            if (p[pixel[10]] < c_b)
                                                if (p[pixel[11]] < c_b)
                                                    if (p[pixel[13]] < c_b)
                                                        if (p[pixel[14]] < c_b)
                                                            if (p[pixel[6]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else if (p[pixel[15]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[4]] < c_b)
                            if (p[pixel[13]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[11]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b)
                                                            if (p[pixel[12]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[13]] < c_b)
                                if (p[pixel[7]] < c_b)
                                    if (p[pixel[8]] < c_b)
                                        if (p[pixel[9]] < c_b)
                                            if (p[pixel[10]] < c_b)
                                                if (p[pixel[11]] < c_b)
                                                    if (p[pixel[12]] < c_b)
                                                        if (p[pixel[6]] < c_b)
                                                            if (p[pixel[5]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else if (p[pixel[14]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else if (p[pixel[14]] < c_b)
                                                            if (p[pixel[15]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[5]] < c_b)
                                if (p[pixel[6]] < c_b)
                                    if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[11]] > cb)
                            if (p[pixel[12]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[11]] < c_b)
                            if (p[pixel[7]] < c_b)
                                if (p[pixel[8]] < c_b)
                                    if (p[pixel[9]] < c_b)
                                        if (p[pixel[10]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b)
                                                    if (p[pixel[6]] < c_b)
                                                        if (p[pixel[5]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else if (p[pixel[14]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else if (p[pixel[14]] < c_b)
                                                        if (p[pixel[15]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[3]] < c_b)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[10]] < c_b)
                            if (p[pixel[7]] < c_b)
                                if (p[pixel[8]] < c_b)
                                    if (p[pixel[9]] < c_b)
                                        if (p[pixel[11]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[5]] < c_b)
                                                    if (p[pixel[4]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else if (p[pixel[12]] < c_b)
                                                        if (p[pixel[13]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b)
                                                        if (p[pixel[14]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b)
                                                        if (p[pixel[15]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[10]] > cb)
                        if (p[pixel[11]] > cb)
                            if (p[pixel[12]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[4]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[10]] < c_b)
                        if (p[pixel[7]] < c_b)
                            if (p[pixel[8]] < c_b)
                                if (p[pixel[9]] < c_b)
                                    if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[5]] < c_b)
                                                    if (p[pixel[4]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else if (p[pixel[13]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b)
                                                    if (p[pixel[15]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[2]] < c_b)
                    if (p[pixel[9]] > cb)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[3]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[9]] < c_b)
                        if (p[pixel[7]] < c_b)
                            if (p[pixel[8]] < c_b)
                                if (p[pixel[10]] < c_b)
                                    if (p[pixel[6]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[4]] < c_b)
                                                if (p[pixel[3]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else if (p[pixel[11]] < c_b)
                                                    if (p[pixel[12]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[11]] < c_b)
                                                if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b)
                                                    if (p[pixel[15]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[9]] > cb)
                    if (p[pixel[10]] > cb)
                        if (p[pixel[11]] > cb)
                            if (p[pixel[12]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[4]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[3]] > cb)
                                if (p[pixel[4]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[9]] < c_b)
                    if (p[pixel[7]] < c_b)
                        if (p[pixel[8]] < c_b)
                            if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[6]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[4]] < c_b)
                                                if (p[pixel[3]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else if (p[pixel[12]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else if (p[pixel[1]] < c_b)
                if (p[pixel[8]] > cb)
                    if (p[pixel[9]] > cb)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[3]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[2]] > cb)
                                if (p[pixel[3]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[8]] < c_b)
                    if (p[pixel[7]] < c_b)
                        if (p[pixel[9]] < c_b)
                            if (p[pixel[6]] < c_b)
                                if (p[pixel[5]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[3]] < c_b)
                                            if (p[pixel[2]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[10]] < c_b)
                                                if (p[pixel[11]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[10]] < c_b)
                                            if (p[pixel[11]] < c_b)
                                                if (p[pixel[12]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[10]] < c_b)
                                        if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[10]] < c_b)
                                    if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else if (p[pixel[8]] > cb)
                if (p[pixel[9]] > cb)
                    if (p[pixel[10]] > cb)
                        if (p[pixel[11]] > cb)
                            if (p[pixel[12]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[4]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[3]] > cb)
                                if (p[pixel[4]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[2]] > cb)
                            if (p[pixel[3]] > cb)
                                if (p[pixel[4]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else if (p[pixel[8]] < c_b)
                if (p[pixel[7]] < c_b)
                    if (p[pixel[9]] < c_b)
                        if (p[pixel[10]] < c_b)
                            if (p[pixel[6]] < c_b)
                                if (p[pixel[5]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[3]] < c_b)
                                            if (p[pixel[2]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[11]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else {
                goto is_not_a_corner;
            }
        else if (p[pixel[0]] < c_b)
            if (p[pixel[1]] > cb)
                if (p[pixel[8]] > cb)
                    if (p[pixel[7]] > cb)
                        if (p[pixel[9]] > cb)
                            if (p[pixel[6]] > cb)
                                if (p[pixel[5]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[3]] > cb)
                                            if (p[pixel[2]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[10]] > cb)
                                                if (p[pixel[11]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[10]] > cb)
                                            if (p[pixel[11]] > cb)
                                                if (p[pixel[12]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[10]] > cb)
                                        if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[10]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[8]] < c_b)
                    if (p[pixel[9]] < c_b)
                        if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[3]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[2]] < c_b)
                                if (p[pixel[3]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else if (p[pixel[1]] < c_b)
                if (p[pixel[2]] > cb)
                    if (p[pixel[9]] > cb)
                        if (p[pixel[7]] > cb)
                            if (p[pixel[8]] > cb)
                                if (p[pixel[10]] > cb)
                                    if (p[pixel[6]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[4]] > cb)
                                                if (p[pixel[3]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else if (p[pixel[11]] > cb)
                                                    if (p[pixel[12]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[11]] > cb)
                                                if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb)
                                                    if (p[pixel[15]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[9]] < c_b)
                        if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[3]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[2]] < c_b)
                    if (p[pixel[3]] > cb)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[7]] > cb)
                                if (p[pixel[8]] > cb)
                                    if (p[pixel[9]] > cb)
                                        if (p[pixel[11]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[5]] > cb)
                                                    if (p[pixel[4]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else if (p[pixel[12]] > cb)
                                                        if (p[pixel[13]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb)
                                                        if (p[pixel[14]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb)
                                                        if (p[pixel[15]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[3]] < c_b)
                        if (p[pixel[4]] > cb)
                            if (p[pixel[13]] > cb)
                                if (p[pixel[7]] > cb)
                                    if (p[pixel[8]] > cb)
                                        if (p[pixel[9]] > cb)
                                            if (p[pixel[10]] > cb)
                                                if (p[pixel[11]] > cb)
                                                    if (p[pixel[12]] > cb)
                                                        if (p[pixel[6]] > cb)
                                                            if (p[pixel[5]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else if (p[pixel[14]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else if (p[pixel[14]] > cb)
                                                            if (p[pixel[15]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[13]] < c_b)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb)
                                                            if (p[pixel[12]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[5]] > cb)
                                if (p[pixel[6]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[4]] < c_b)
                            if (p[pixel[5]] > cb)
                                if (p[pixel[14]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb)
                                                            if (p[pixel[13]] > cb)
                                                                if (p[pixel[6]] > cb) {
                                                                    goto is_a_corner;
                                                                }
                                                                else if (p[pixel[15]] > cb) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[14]] < c_b)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb)
                                                            if (p[pixel[11]] > cb)
                                                                if (p[pixel[13]] > cb) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b)
                                                                if (p[pixel[11]] < c_b) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[6]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb)
                                                            if (p[pixel[13]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[5]] < c_b)
                                if (p[pixel[6]] > cb)
                                    if (p[pixel[15]] < c_b)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[7]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb)
                                                            if (p[pixel[11]] > cb)
                                                                if (p[pixel[12]] > cb)
                                                                    if (p[pixel[14]] > cb) {
                                                                        goto is_a_corner;
                                                                    }
                                                                    else {
                                                                        goto is_not_a_corner;
                                                                    }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb)
                                                            if (p[pixel[13]] > cb)
                                                                if (p[pixel[14]] > cb) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[6]] < c_b)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[8]] > cb)
                                                if (p[pixel[9]] > cb)
                                                    if (p[pixel[10]] > cb)
                                                        if (p[pixel[11]] > cb)
                                                            if (p[pixel[12]] > cb)
                                                                if (p[pixel[13]] > cb)
                                                                    if (p[pixel[15]] > cb) {
                                                                        goto is_a_corner;
                                                                    }
                                                                    else {
                                                                        goto is_not_a_corner;
                                                                    }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[13]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb)
                                                            if (p[pixel[14]] > cb)
                                                                if (p[pixel[15]] > cb) {
                                                                    goto is_a_corner;
                                                                }
                                                                else {
                                                                    goto is_not_a_corner;
                                                                }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[12]] > cb)
                                if (p[pixel[7]] > cb)
                                    if (p[pixel[8]] > cb)
                                        if (p[pixel[9]] > cb)
                                            if (p[pixel[10]] > cb)
                                                if (p[pixel[11]] > cb)
                                                    if (p[pixel[13]] > cb)
                                                        if (p[pixel[14]] > cb)
                                                            if (p[pixel[6]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else if (p[pixel[15]] > cb) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[12]] < c_b)
                                if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b)
                                                            if (p[pixel[11]] < c_b) {
                                                                goto is_a_corner;
                                                            }
                                                            else {
                                                                goto is_not_a_corner;
                                                            }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[11]] > cb)
                            if (p[pixel[7]] > cb)
                                if (p[pixel[8]] > cb)
                                    if (p[pixel[9]] > cb)
                                        if (p[pixel[10]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb)
                                                    if (p[pixel[6]] > cb)
                                                        if (p[pixel[5]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else if (p[pixel[14]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else if (p[pixel[14]] > cb)
                                                        if (p[pixel[15]] > cb) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[11]] < c_b)
                            if (p[pixel[12]] < c_b)
                                if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b) {
                                                            goto is_a_corner;
                                                        }
                                                        else {
                                                            goto is_not_a_corner;
                                                        }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[10]] > cb)
                        if (p[pixel[7]] > cb)
                            if (p[pixel[8]] > cb)
                                if (p[pixel[9]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[5]] > cb)
                                                    if (p[pixel[4]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else if (p[pixel[13]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb)
                                                    if (p[pixel[15]] > cb) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[10]] < c_b)
                        if (p[pixel[11]] < c_b)
                            if (p[pixel[12]] < c_b)
                                if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[4]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b) {
                                                        goto is_a_corner;
                                                    }
                                                    else {
                                                        goto is_not_a_corner;
                                                    }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[9]] > cb)
                    if (p[pixel[7]] > cb)
                        if (p[pixel[8]] > cb)
                            if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[6]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[4]] > cb)
                                                if (p[pixel[3]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else if (p[pixel[12]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else if (p[pixel[9]] < c_b)
                    if (p[pixel[10]] < c_b)
                        if (p[pixel[11]] < c_b)
                            if (p[pixel[12]] < c_b)
                                if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[4]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[3]] < c_b)
                                if (p[pixel[4]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b)
                                                if (p[pixel[8]] < c_b) {
                                                    goto is_a_corner;
                                                }
                                                else {
                                                    goto is_not_a_corner;
                                                }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else if (p[pixel[8]] > cb)
                if (p[pixel[7]] > cb)
                    if (p[pixel[9]] > cb)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[6]] > cb)
                                if (p[pixel[5]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[3]] > cb)
                                            if (p[pixel[2]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else if (p[pixel[11]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else if (p[pixel[8]] < c_b)
                if (p[pixel[9]] < c_b)
                    if (p[pixel[10]] < c_b)
                        if (p[pixel[11]] < c_b)
                            if (p[pixel[12]] < c_b)
                                if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[4]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[3]] < c_b)
                                if (p[pixel[4]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[2]] < c_b)
                            if (p[pixel[3]] < c_b)
                                if (p[pixel[4]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[7]] < c_b) {
                                                goto is_a_corner;
                                            }
                                            else {
                                                goto is_not_a_corner;
                                            }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else {
                goto is_not_a_corner;
            }
        else if (p[pixel[7]] > cb)
            if (p[pixel[8]] > cb)
                if (p[pixel[9]] > cb)
                    if (p[pixel[6]] > cb)
                        if (p[pixel[5]] > cb)
                            if (p[pixel[4]] > cb)
                                if (p[pixel[3]] > cb)
                                    if (p[pixel[2]] > cb)
                                        if (p[pixel[1]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[10]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[10]] > cb)
                                        if (p[pixel[11]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[10]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[10]] > cb)
                        if (p[pixel[11]] > cb)
                            if (p[pixel[12]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[15]] > cb) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else {
                goto is_not_a_corner;
            }
        else if (p[pixel[7]] < c_b)
            if (p[pixel[8]] < c_b)
                if (p[pixel[9]] < c_b)
                    if (p[pixel[6]] < c_b)
                        if (p[pixel[5]] < c_b)
                            if (p[pixel[4]] < c_b)
                                if (p[pixel[3]] < c_b)
                                    if (p[pixel[2]] < c_b)
                                        if (p[pixel[1]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else if (p[pixel[10]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else if (p[pixel[10]] < c_b)
                                        if (p[pixel[11]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else if (p[pixel[10]] < c_b)
                                    if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else if (p[pixel[10]] < c_b)
                        if (p[pixel[11]] < c_b)
                            if (p[pixel[12]] < c_b)
                                if (p[pixel[13]] < c_b)
                                    if (p[pixel[14]] < c_b)
                                        if (p[pixel[15]] < c_b) {
                                            goto is_a_corner;
                                        }
                                        else {
                                            goto is_not_a_corner;
                                        }
                                    else {
                                        goto is_not_a_corner;
                                    }
                                else {
                                    goto is_not_a_corner;
                                }
                            else {
                                goto is_not_a_corner;
                            }
                        else {
                            goto is_not_a_corner;
                        }
                    else {
                        goto is_not_a_corner;
                    }
                else {
                    goto is_not_a_corner;
                }
            else {
                goto is_not_a_corner;
            }
        else {
            goto is_not_a_corner;
        }

is_a_corner:
        bmin = b;
        goto end_if;

is_not_a_corner:
        bmax = b;
        goto end_if;

end_if:

        if (bmin == bmax - 1 || bmin == bmax) {
            return bmin;
        }
        b = (bmin + bmax) / 2;
    }
}

static void make_offsets(int pixel[], int row_stride)
{
    pixel[0] = 0 + row_stride * 3;
    pixel[1] = 1 + row_stride * 3;
    pixel[2] = 2 + row_stride * 2;
    pixel[3] = 3 + row_stride * 1;
    pixel[4] = 3 + row_stride * 0;
    pixel[5] = 3 + row_stride * -1;
    pixel[6] = 2 + row_stride * -2;
    pixel[7] = 1 + row_stride * -3;
    pixel[8] = 0 + row_stride * -3;
    pixel[9] = -1 + row_stride * -3;
    pixel[10] = -2 + row_stride * -2;
    pixel[11] = -3 + row_stride * -1;
    pixel[12] = -3 + row_stride * 0;
    pixel[13] = -3 + row_stride * 1;
    pixel[14] = -2 + row_stride * 2;
    pixel[15] = -1 + row_stride * 3;
}



int* fast9_score(const byte* i, int stride, xy* corners, int num_corners, int b)
{
    int* scores = (int*)malloc(sizeof(int)* num_corners);
    int n;

    int pixel[16];
    make_offsets(pixel, stride);

    for (n = 0; n < num_corners; n++) {
        scores[n] = fast9_corner_score(i + corners[n].y*stride + corners[n].x, pixel, b);
    }

    return scores;
}


xy* fast9_detect(const byte* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
{
    int num_corners = 0;
    xy* ret_corners;
    int rsize = 512;
    int pixel[16];
    int x, y;

    ret_corners = (xy*)malloc(sizeof(xy)*rsize);
    make_offsets(pixel, stride);

    for (y = 3; y < ysize - 3; y++)
        for (x = 3; x < xsize - 3; x++) {
            const byte* p = im + y*stride + x;

            int cb = *p + b;
            int c_b = *p - b;
            if (p[pixel[0]] > cb)
                if (p[pixel[1]] > cb)
                    if (p[pixel[2]] > cb)
                        if (p[pixel[3]] > cb)
                            if (p[pixel[4]] > cb)
                                if (p[pixel[5]] > cb)
                                    if (p[pixel[6]] > cb)
                                        if (p[pixel[7]] > cb)
                                            if (p[pixel[8]] > cb) {
                                            }
                                            else if (p[pixel[15]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[7]] < c_b)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[14]] < c_b)
                                                if (p[pixel[8]] < c_b)
                                                    if (p[pixel[9]] < c_b)
                                                        if (p[pixel[10]] < c_b)
                                                            if (p[pixel[11]] < c_b)
                                                                if (p[pixel[12]] < c_b)
                                                                    if (p[pixel[13]] < c_b)
                                                                        if (p[pixel[15]] < c_b) {
                                                                        }
                                                                        else {
                                                                            continue;
                                                                        }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[6]] < c_b)
                                        if (p[pixel[15]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[13]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b)
                                                                if (p[pixel[11]] < c_b)
                                                                    if (p[pixel[12]] < c_b)
                                                                        if (p[pixel[14]] < c_b) {
                                                                        }
                                                                        else {
                                                                            continue;
                                                                        }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[7]] < c_b)
                                            if (p[pixel[8]] < c_b)
                                                if (p[pixel[9]] < c_b)
                                                    if (p[pixel[10]] < c_b)
                                                        if (p[pixel[11]] < c_b)
                                                            if (p[pixel[12]] < c_b)
                                                                if (p[pixel[13]] < c_b)
                                                                    if (p[pixel[14]] < c_b) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[13]] < c_b)
                                        if (p[pixel[7]] < c_b)
                                            if (p[pixel[8]] < c_b)
                                                if (p[pixel[9]] < c_b)
                                                    if (p[pixel[10]] < c_b)
                                                        if (p[pixel[11]] < c_b)
                                                            if (p[pixel[12]] < c_b)
                                                                if (p[pixel[14]] < c_b)
                                                                    if (p[pixel[15]] < c_b) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[5]] < c_b)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb)
                                                            if (p[pixel[9]] > cb)
                                                                if (p[pixel[10]] > cb)
                                                                    if (p[pixel[11]] > cb) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[12]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b)
                                                                if (p[pixel[11]] < c_b)
                                                                    if (p[pixel[13]] < c_b) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[14]] < c_b)
                                        if (p[pixel[7]] < c_b)
                                            if (p[pixel[8]] < c_b)
                                                if (p[pixel[9]] < c_b)
                                                    if (p[pixel[10]] < c_b)
                                                        if (p[pixel[11]] < c_b)
                                                            if (p[pixel[12]] < c_b)
                                                                if (p[pixel[13]] < c_b)
                                                                    if (p[pixel[6]] < c_b) {
                                                                    }
                                                                    else if (p[pixel[15]] < c_b) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[6]] < c_b)
                                        if (p[pixel[7]] < c_b)
                                            if (p[pixel[8]] < c_b)
                                                if (p[pixel[9]] < c_b)
                                                    if (p[pixel[10]] < c_b)
                                                        if (p[pixel[11]] < c_b)
                                                            if (p[pixel[12]] < c_b)
                                                                if (p[pixel[13]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb)
                                                                if (p[pixel[11]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[12]] < c_b)
                                    if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[13]] < c_b)
                                                            if (p[pixel[14]] < c_b)
                                                                if (p[pixel[6]] < c_b) {
                                                                }
                                                                else if (p[pixel[15]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[4]] < c_b)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb)
                                                            if (p[pixel[9]] > cb)
                                                                if (p[pixel[10]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb)
                                                            if (p[pixel[9]] > cb)
                                                                if (p[pixel[10]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[11]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b)
                                                                if (p[pixel[12]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[13]] < c_b)
                                    if (p[pixel[7]] < c_b)
                                        if (p[pixel[8]] < c_b)
                                            if (p[pixel[9]] < c_b)
                                                if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b)
                                                            if (p[pixel[6]] < c_b)
                                                                if (p[pixel[5]] < c_b) {
                                                                }
                                                                else if (p[pixel[14]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else if (p[pixel[14]] < c_b)
                                                                if (p[pixel[15]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[5]] < c_b)
                                    if (p[pixel[6]] < c_b)
                                        if (p[pixel[7]] < c_b)
                                            if (p[pixel[8]] < c_b)
                                                if (p[pixel[9]] < c_b)
                                                    if (p[pixel[10]] < c_b)
                                                        if (p[pixel[11]] < c_b)
                                                            if (p[pixel[12]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[11]] < c_b)
                                if (p[pixel[7]] < c_b)
                                    if (p[pixel[8]] < c_b)
                                        if (p[pixel[9]] < c_b)
                                            if (p[pixel[10]] < c_b)
                                                if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b)
                                                        if (p[pixel[6]] < c_b)
                                                            if (p[pixel[5]] < c_b) {
                                                            }
                                                            else if (p[pixel[14]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else if (p[pixel[14]] < c_b)
                                                            if (p[pixel[15]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[3]] < c_b)
                            if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb)
                                                            if (p[pixel[9]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb)
                                                            if (p[pixel[9]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[4]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb)
                                                            if (p[pixel[9]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[10]] < c_b)
                                if (p[pixel[7]] < c_b)
                                    if (p[pixel[8]] < c_b)
                                        if (p[pixel[9]] < c_b)
                                            if (p[pixel[11]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[5]] < c_b)
                                                        if (p[pixel[4]] < c_b) {
                                                        }
                                                        else if (p[pixel[12]] < c_b)
                                                            if (p[pixel[13]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else if (p[pixel[12]] < c_b)
                                                        if (p[pixel[13]] < c_b)
                                                            if (p[pixel[14]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b)
                                                        if (p[pixel[14]] < c_b)
                                                            if (p[pixel[15]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[10]] < c_b)
                            if (p[pixel[7]] < c_b)
                                if (p[pixel[8]] < c_b)
                                    if (p[pixel[9]] < c_b)
                                        if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[5]] < c_b)
                                                        if (p[pixel[4]] < c_b) {
                                                        }
                                                        else if (p[pixel[13]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else if (p[pixel[13]] < c_b)
                                                        if (p[pixel[14]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b)
                                                        if (p[pixel[15]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[2]] < c_b)
                        if (p[pixel[9]] > cb)
                            if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[4]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[3]] > cb)
                                        if (p[pixel[4]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb)
                                                        if (p[pixel[8]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[9]] < c_b)
                            if (p[pixel[7]] < c_b)
                                if (p[pixel[8]] < c_b)
                                    if (p[pixel[10]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[4]] < c_b)
                                                    if (p[pixel[3]] < c_b) {
                                                    }
                                                    else if (p[pixel[11]] < c_b)
                                                        if (p[pixel[12]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[11]] < c_b)
                                                    if (p[pixel[12]] < c_b)
                                                        if (p[pixel[13]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[11]] < c_b)
                                                if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b)
                                                        if (p[pixel[14]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b)
                                                        if (p[pixel[15]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[9]] > cb)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[3]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[9]] < c_b)
                        if (p[pixel[7]] < c_b)
                            if (p[pixel[8]] < c_b)
                                if (p[pixel[10]] < c_b)
                                    if (p[pixel[11]] < c_b)
                                        if (p[pixel[6]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[4]] < c_b)
                                                    if (p[pixel[3]] < c_b) {
                                                    }
                                                    else if (p[pixel[12]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b)
                                                    if (p[pixel[15]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else if (p[pixel[1]] < c_b)
                    if (p[pixel[8]] > cb)
                        if (p[pixel[9]] > cb)
                            if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[4]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[3]] > cb)
                                        if (p[pixel[4]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[2]] > cb)
                                    if (p[pixel[3]] > cb)
                                        if (p[pixel[4]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[7]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[8]] < c_b)
                        if (p[pixel[7]] < c_b)
                            if (p[pixel[9]] < c_b)
                                if (p[pixel[6]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[4]] < c_b)
                                            if (p[pixel[3]] < c_b)
                                                if (p[pixel[2]] < c_b) {
                                                }
                                                else if (p[pixel[10]] < c_b)
                                                    if (p[pixel[11]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[10]] < c_b)
                                                if (p[pixel[11]] < c_b)
                                                    if (p[pixel[12]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[10]] < c_b)
                                            if (p[pixel[11]] < c_b)
                                                if (p[pixel[12]] < c_b)
                                                    if (p[pixel[13]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[10]] < c_b)
                                        if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b)
                                                    if (p[pixel[14]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[10]] < c_b)
                                    if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b)
                                                    if (p[pixel[15]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else if (p[pixel[8]] > cb)
                    if (p[pixel[9]] > cb)
                        if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[3]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[2]] > cb)
                                if (p[pixel[3]] > cb)
                                    if (p[pixel[4]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else if (p[pixel[8]] < c_b)
                    if (p[pixel[7]] < c_b)
                        if (p[pixel[9]] < c_b)
                            if (p[pixel[10]] < c_b)
                                if (p[pixel[6]] < c_b)
                                    if (p[pixel[5]] < c_b)
                                        if (p[pixel[4]] < c_b)
                                            if (p[pixel[3]] < c_b)
                                                if (p[pixel[2]] < c_b) {
                                                }
                                                else if (p[pixel[11]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[11]] < c_b)
                                                if (p[pixel[12]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b)
                                                if (p[pixel[13]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else {
                    continue;
                }
            else if (p[pixel[0]] < c_b)
                if (p[pixel[1]] > cb)
                    if (p[pixel[8]] > cb)
                        if (p[pixel[7]] > cb)
                            if (p[pixel[9]] > cb)
                                if (p[pixel[6]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[4]] > cb)
                                            if (p[pixel[3]] > cb)
                                                if (p[pixel[2]] > cb) {
                                                }
                                                else if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[10]] > cb)
                                                if (p[pixel[11]] > cb)
                                                    if (p[pixel[12]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[10]] > cb)
                                            if (p[pixel[11]] > cb)
                                                if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[10]] > cb)
                                        if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[10]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb)
                                                    if (p[pixel[15]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[8]] < c_b)
                        if (p[pixel[9]] < c_b)
                            if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[4]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[3]] < c_b)
                                        if (p[pixel[4]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[2]] < c_b)
                                    if (p[pixel[3]] < c_b)
                                        if (p[pixel[4]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else if (p[pixel[1]] < c_b)
                    if (p[pixel[2]] > cb)
                        if (p[pixel[9]] > cb)
                            if (p[pixel[7]] > cb)
                                if (p[pixel[8]] > cb)
                                    if (p[pixel[10]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[4]] > cb)
                                                    if (p[pixel[3]] > cb) {
                                                    }
                                                    else if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[11]] > cb)
                                                    if (p[pixel[12]] > cb)
                                                        if (p[pixel[13]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[11]] > cb)
                                                if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb)
                                                        if (p[pixel[14]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb)
                                                        if (p[pixel[15]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[9]] < c_b)
                            if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[4]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[3]] < c_b)
                                        if (p[pixel[4]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[2]] < c_b)
                        if (p[pixel[3]] > cb)
                            if (p[pixel[10]] > cb)
                                if (p[pixel[7]] > cb)
                                    if (p[pixel[8]] > cb)
                                        if (p[pixel[9]] > cb)
                                            if (p[pixel[11]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[5]] > cb)
                                                        if (p[pixel[4]] > cb) {
                                                        }
                                                        else if (p[pixel[12]] > cb)
                                                            if (p[pixel[13]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else if (p[pixel[12]] > cb)
                                                        if (p[pixel[13]] > cb)
                                                            if (p[pixel[14]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb)
                                                        if (p[pixel[14]] > cb)
                                                            if (p[pixel[15]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b)
                                                            if (p[pixel[9]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b)
                                                            if (p[pixel[9]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[4]] < c_b)
                                            if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b)
                                                            if (p[pixel[9]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[3]] < c_b)
                            if (p[pixel[4]] > cb)
                                if (p[pixel[13]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[12]] > cb)
                                                            if (p[pixel[6]] > cb)
                                                                if (p[pixel[5]] > cb) {
                                                                }
                                                                else if (p[pixel[14]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else if (p[pixel[14]] > cb)
                                                                if (p[pixel[15]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[13]] < c_b)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[5]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb)
                                                                if (p[pixel[12]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b)
                                                            if (p[pixel[9]] < c_b)
                                                                if (p[pixel[10]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[5]] < c_b)
                                                if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b)
                                                            if (p[pixel[9]] < c_b)
                                                                if (p[pixel[10]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[5]] > cb)
                                    if (p[pixel[6]] > cb)
                                        if (p[pixel[7]] > cb)
                                            if (p[pixel[8]] > cb)
                                                if (p[pixel[9]] > cb)
                                                    if (p[pixel[10]] > cb)
                                                        if (p[pixel[11]] > cb)
                                                            if (p[pixel[12]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[4]] < c_b)
                                if (p[pixel[5]] > cb)
                                    if (p[pixel[14]] > cb)
                                        if (p[pixel[7]] > cb)
                                            if (p[pixel[8]] > cb)
                                                if (p[pixel[9]] > cb)
                                                    if (p[pixel[10]] > cb)
                                                        if (p[pixel[11]] > cb)
                                                            if (p[pixel[12]] > cb)
                                                                if (p[pixel[13]] > cb)
                                                                    if (p[pixel[6]] > cb) {
                                                                    }
                                                                    else if (p[pixel[15]] > cb) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[14]] < c_b)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[6]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb)
                                                                if (p[pixel[11]] > cb)
                                                                    if (p[pixel[13]] > cb) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else if (p[pixel[6]] < c_b)
                                                    if (p[pixel[7]] < c_b)
                                                        if (p[pixel[8]] < c_b)
                                                            if (p[pixel[9]] < c_b)
                                                                if (p[pixel[10]] < c_b)
                                                                    if (p[pixel[11]] < c_b) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[6]] > cb)
                                        if (p[pixel[7]] > cb)
                                            if (p[pixel[8]] > cb)
                                                if (p[pixel[9]] > cb)
                                                    if (p[pixel[10]] > cb)
                                                        if (p[pixel[11]] > cb)
                                                            if (p[pixel[12]] > cb)
                                                                if (p[pixel[13]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[5]] < c_b)
                                    if (p[pixel[6]] > cb)
                                        if (p[pixel[15]] < c_b)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[7]] > cb)
                                                    if (p[pixel[8]] > cb)
                                                        if (p[pixel[9]] > cb)
                                                            if (p[pixel[10]] > cb)
                                                                if (p[pixel[11]] > cb)
                                                                    if (p[pixel[12]] > cb)
                                                                        if (p[pixel[14]] > cb) {
                                                                        }
                                                                        else {
                                                                            continue;
                                                                        }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[13]] < c_b)
                                                if (p[pixel[14]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[7]] > cb)
                                            if (p[pixel[8]] > cb)
                                                if (p[pixel[9]] > cb)
                                                    if (p[pixel[10]] > cb)
                                                        if (p[pixel[11]] > cb)
                                                            if (p[pixel[12]] > cb)
                                                                if (p[pixel[13]] > cb)
                                                                    if (p[pixel[14]] > cb) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[6]] < c_b)
                                        if (p[pixel[7]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[8]] > cb)
                                                    if (p[pixel[9]] > cb)
                                                        if (p[pixel[10]] > cb)
                                                            if (p[pixel[11]] > cb)
                                                                if (p[pixel[12]] > cb)
                                                                    if (p[pixel[13]] > cb)
                                                                        if (p[pixel[15]] > cb) {
                                                                        }
                                                                        else {
                                                                            continue;
                                                                        }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[14]] < c_b)
                                                if (p[pixel[15]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[7]] < c_b)
                                            if (p[pixel[8]] < c_b) {
                                            }
                                            else if (p[pixel[15]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[13]] > cb)
                                        if (p[pixel[7]] > cb)
                                            if (p[pixel[8]] > cb)
                                                if (p[pixel[9]] > cb)
                                                    if (p[pixel[10]] > cb)
                                                        if (p[pixel[11]] > cb)
                                                            if (p[pixel[12]] > cb)
                                                                if (p[pixel[14]] > cb)
                                                                    if (p[pixel[15]] > cb) {
                                                                    }
                                                                    else {
                                                                        continue;
                                                                    }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[12]] > cb)
                                    if (p[pixel[7]] > cb)
                                        if (p[pixel[8]] > cb)
                                            if (p[pixel[9]] > cb)
                                                if (p[pixel[10]] > cb)
                                                    if (p[pixel[11]] > cb)
                                                        if (p[pixel[13]] > cb)
                                                            if (p[pixel[14]] > cb)
                                                                if (p[pixel[6]] > cb) {
                                                                }
                                                                else if (p[pixel[15]] > cb) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b)
                                                                if (p[pixel[11]] < c_b) {
                                                                }
                                                                else {
                                                                    continue;
                                                                }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[11]] > cb)
                                if (p[pixel[7]] > cb)
                                    if (p[pixel[8]] > cb)
                                        if (p[pixel[9]] > cb)
                                            if (p[pixel[10]] > cb)
                                                if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb)
                                                        if (p[pixel[6]] > cb)
                                                            if (p[pixel[5]] > cb) {
                                                            }
                                                            else if (p[pixel[14]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else if (p[pixel[14]] > cb)
                                                            if (p[pixel[15]] > cb) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b)
                                                            if (p[pixel[10]] < c_b) {
                                                            }
                                                            else {
                                                                continue;
                                                            }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[10]] > cb)
                            if (p[pixel[7]] > cb)
                                if (p[pixel[8]] > cb)
                                    if (p[pixel[9]] > cb)
                                        if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[6]] > cb)
                                                    if (p[pixel[5]] > cb)
                                                        if (p[pixel[4]] > cb) {
                                                        }
                                                        else if (p[pixel[13]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else if (p[pixel[13]] > cb)
                                                        if (p[pixel[14]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb)
                                                        if (p[pixel[15]] > cb) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b)
                                                        if (p[pixel[9]] < c_b) {
                                                        }
                                                        else {
                                                            continue;
                                                        }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[9]] > cb)
                        if (p[pixel[7]] > cb)
                            if (p[pixel[8]] > cb)
                                if (p[pixel[10]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[6]] > cb)
                                            if (p[pixel[5]] > cb)
                                                if (p[pixel[4]] > cb)
                                                    if (p[pixel[3]] > cb) {
                                                    }
                                                    else if (p[pixel[12]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else if (p[pixel[12]] > cb)
                                                    if (p[pixel[13]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb)
                                                    if (p[pixel[14]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb)
                                                    if (p[pixel[15]] > cb) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else if (p[pixel[9]] < c_b)
                        if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[3]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b)
                                                    if (p[pixel[8]] < c_b) {
                                                    }
                                                    else {
                                                        continue;
                                                    }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else if (p[pixel[8]] > cb)
                    if (p[pixel[7]] > cb)
                        if (p[pixel[9]] > cb)
                            if (p[pixel[10]] > cb)
                                if (p[pixel[6]] > cb)
                                    if (p[pixel[5]] > cb)
                                        if (p[pixel[4]] > cb)
                                            if (p[pixel[3]] > cb)
                                                if (p[pixel[2]] > cb) {
                                                }
                                                else if (p[pixel[11]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else if (p[pixel[11]] > cb)
                                                if (p[pixel[12]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb)
                                                if (p[pixel[13]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb)
                                                if (p[pixel[14]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb)
                                                if (p[pixel[15]] > cb) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else if (p[pixel[8]] < c_b)
                    if (p[pixel[9]] < c_b)
                        if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[3]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[2]] < c_b)
                                if (p[pixel[3]] < c_b)
                                    if (p[pixel[4]] < c_b)
                                        if (p[pixel[5]] < c_b)
                                            if (p[pixel[6]] < c_b)
                                                if (p[pixel[7]] < c_b) {
                                                }
                                                else {
                                                    continue;
                                                }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else {
                    continue;
                }
            else if (p[pixel[7]] > cb)
                if (p[pixel[8]] > cb)
                    if (p[pixel[9]] > cb)
                        if (p[pixel[6]] > cb)
                            if (p[pixel[5]] > cb)
                                if (p[pixel[4]] > cb)
                                    if (p[pixel[3]] > cb)
                                        if (p[pixel[2]] > cb)
                                            if (p[pixel[1]] > cb) {
                                            }
                                            else if (p[pixel[10]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[10]] > cb)
                                            if (p[pixel[11]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[10]] > cb)
                                        if (p[pixel[11]] > cb)
                                            if (p[pixel[12]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[10]] > cb)
                                    if (p[pixel[11]] > cb)
                                        if (p[pixel[12]] > cb)
                                            if (p[pixel[13]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[10]] > cb)
                                if (p[pixel[11]] > cb)
                                    if (p[pixel[12]] > cb)
                                        if (p[pixel[13]] > cb)
                                            if (p[pixel[14]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[10]] > cb)
                            if (p[pixel[11]] > cb)
                                if (p[pixel[12]] > cb)
                                    if (p[pixel[13]] > cb)
                                        if (p[pixel[14]] > cb)
                                            if (p[pixel[15]] > cb) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else {
                    continue;
                }
            else if (p[pixel[7]] < c_b)
                if (p[pixel[8]] < c_b)
                    if (p[pixel[9]] < c_b)
                        if (p[pixel[6]] < c_b)
                            if (p[pixel[5]] < c_b)
                                if (p[pixel[4]] < c_b)
                                    if (p[pixel[3]] < c_b)
                                        if (p[pixel[2]] < c_b)
                                            if (p[pixel[1]] < c_b) {
                                            }
                                            else if (p[pixel[10]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else if (p[pixel[10]] < c_b)
                                            if (p[pixel[11]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else if (p[pixel[10]] < c_b)
                                        if (p[pixel[11]] < c_b)
                                            if (p[pixel[12]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else if (p[pixel[10]] < c_b)
                                    if (p[pixel[11]] < c_b)
                                        if (p[pixel[12]] < c_b)
                                            if (p[pixel[13]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else if (p[pixel[10]] < c_b)
                                if (p[pixel[11]] < c_b)
                                    if (p[pixel[12]] < c_b)
                                        if (p[pixel[13]] < c_b)
                                            if (p[pixel[14]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else if (p[pixel[10]] < c_b)
                            if (p[pixel[11]] < c_b)
                                if (p[pixel[12]] < c_b)
                                    if (p[pixel[13]] < c_b)
                                        if (p[pixel[14]] < c_b)
                                            if (p[pixel[15]] < c_b) {
                                            }
                                            else {
                                                continue;
                                            }
                                        else {
                                            continue;
                                        }
                                    else {
                                        continue;
                                    }
                                else {
                                    continue;
                                }
                            else {
                                continue;
                            }
                        else {
                            continue;
                        }
                    else {
                        continue;
                    }
                else {
                    continue;
                }
            else {
                continue;
            }
            if (num_corners == rsize) {
                rsize *= 2;
                ret_corners = (xy*)realloc(ret_corners, sizeof(xy)*rsize);
            }
            ret_corners[num_corners].x = x;
            ret_corners[num_corners].y = y;
            num_corners++;

        }

    *ret_num_corners = num_corners;
    return ret_corners;

}

#define Compare(X, Y) ((X)>=(Y))

xy* nonmax_suppression(const xy* corners, const int* scores, int num_corners, int* ret_num_nonmax)
{
    int num_nonmax = 0;
    int last_row;
    int* row_start;
    int i, j;
    xy* ret_nonmax;
    const int sz = (int)num_corners;

    /*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
    int point_above = 0;
    int point_below = 0;


    if (num_corners < 1) {
        *ret_num_nonmax = 0;
        return 0;
    }

    ret_nonmax = (xy*)malloc(num_corners * sizeof(xy));

    /* Find where each row begins
    (the corners are output in raster scan order). A beginning of -1 signifies
    that there are no corners on that row. */
    last_row = corners[num_corners - 1].y;
    row_start = (int*)malloc((last_row + 1)*sizeof(int));

    for (i = 0; i < last_row + 1; i++) {
        row_start[i] = -1;
    }

    {
        int prev_row = -1;
        for (i = 0; i< num_corners; i++)
            if (corners[i].y != prev_row) {
                row_start[corners[i].y] = i;
                prev_row = corners[i].y;
            }
    }



    for (i = 0; i < sz; i++) {
        int score = scores[i];
        xy pos = corners[i];

        /*Check left */
        if (i > 0)
            if (corners[i - 1].x == pos.x - 1 && corners[i - 1].y == pos.y && Compare(scores[i - 1], score)) {
                continue;
            }

        /*Check right*/
        if (i < (sz - 1))
            if (corners[i + 1].x == pos.x + 1 && corners[i + 1].y == pos.y && Compare(scores[i + 1], score)) {
                continue;
            }

        /*Check above (if there is a valid row above)*/
        if (pos.y != 0 && row_start[pos.y - 1] != -1) {
            /*Make sure that current point_above is one
            row above.*/
            if (corners[point_above].y < pos.y - 1) {
                point_above = row_start[pos.y - 1];
            }

            /*Make point_above point to the first of the pixels above the current point,
            if it exists.*/
            for (; corners[point_above].y < pos.y && corners[point_above].x < pos.x - 1; point_above++) {
            }


            for (j = point_above; corners[j].y < pos.y && corners[j].x <= pos.x + 1; j++) {
                int x = corners[j].x;
                if ((x == pos.x - 1 || x == pos.x || x == pos.x + 1) && Compare(scores[j], score)) {
                    goto cont;
                }
            }

        }

        /*Check below (if there is anything below)*/
        if (pos.y != last_row && row_start[pos.y + 1] != -1 && point_below < sz) { /*Nothing below*/
            if (corners[point_below].y < pos.y + 1) {
                point_below = row_start[pos.y + 1];
            }

            /* Make point below point to one of the pixels belowthe current point, if it
            exists.*/
            for (; point_below < sz && corners[point_below].y == pos.y + 1 && corners[point_below].x < pos.x - 1; point_below++) {
            }

            for (j = point_below; j < sz && corners[j].y == pos.y + 1 && corners[j].x <= pos.x + 1; j++) {
                int x = corners[j].x;
                if ((x == pos.x - 1 || x == pos.x || x == pos.x + 1) && Compare(scores[j], score)) {
                    goto cont;
                }
            }
        }

        ret_nonmax[num_nonmax++] = corners[i];
cont:
        ;
    }

    free(row_start);
    *ret_num_nonmax = num_nonmax;
    return ret_nonmax;
}


static bool isCorner(const uint8_t* Ip, int threshold, int N, const int* pixels16)
{
    int ndarker, ndrighter, p16Idx;
    uint8_t drighter = CompVMathUtils::clampPixel8(*Ip + threshold);
    uint8_t darker = CompVMathUtils::clampPixel8(*Ip - threshold);
    uint8_t Ix;
    for (int m = 0; m < 16; ++m) {
        ndarker = 0;
        ndrighter = 0;
        for (int k = 0; k < 16; ++k) {
            p16Idx = (m + k) % 16;
            Ix = Ip[pixels16[p16Idx]];
            ndrighter += (Ix > drighter) ? 1 : -ndrighter;
            ndarker += (Ix < darker) ? 1 : -ndarker;
            if (ndarker == N || ndrighter == N) {
                return true;
            }
        }
    }
    return false;
}

template<typename V >
struct second_t {
    typename V::second_type operator()(const V& p) const {
        return p.second;
    }
};

template<typename U >
second_t<typename U::value_type > second(const U& m)
{
    return second_t<typename U::value_type >();
}


COMPV_ERROR_CODE CompVFeatureDeteFAST::process(const CompVObjWrapper<CompVImage*>& image, std::vector<CompVInterestPoint >& interestPoints)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    const uint8_t* dataPtr = (const uint8_t*)image->getDataPtr();
    int width = (int)image->getWidth();
    int height = (int)image->getHeight();
    int stride = (int)image->getStride();
    int pad = (stride - width);
    int row, candIdx;
    bool isInterestPoint;
    int32_t strength = 0;
    std::map<int, CompVInterestPoint> mapPoints;

    // clear old points
    interestPoints.clear();

    const int pixels16[16] = {
        -(stride * 3) + 0, // 1
        -(stride * 3) + 1, // 2
        -(stride * 2) + 2, // 3
        -(stride * 1) + 3, // 4
        +(stride * 0) + 3, // 5
        +(stride * 1) + 3, // 6
        +(stride * 2) + 2, // 7
        +(stride * 3) + 1, // 8
        +(stride * 3) + 0, // 9
        +(stride * 3) - 1, // 10
        +(stride * 2) - 2, // 11
        +(stride * 1) - 3, // 12
        +(stride * 0) - 3, // 13
        -(stride * 1) - 3, // 14
        -(stride * 2) - 2, // 15
        -(stride * 3) - 1, // 16
    };

    int num_corners = 0, ret_num_corners = 0;
    xy* corners = fast9_detect((const byte*)dataPtr, width, height, stride, m_iThreshold, &num_corners);
    int* scores = fast9_score((const byte*)dataPtr, stride, corners, num_corners, m_iThreshold);
    xy* nonmax = nonmax_suppression(corners, scores, num_corners, &ret_num_corners);
#if 0
    for (int i = 0; i < num_corners; ++i) {
        interestPoints.push_back(CompVInterestPoint(corners[i].x, corners[i].y));
    }
    return err_;
#elif 0
    for (int i = 0; i < ret_num_corners; ++i) {
        interestPoints.push_back(CompVInterestPoint(nonmax[i].x, nonmax[i].y));
    }
    return err_;
#endif

    for (int j = 3; j < height - 3; ++j) {
        row = j * stride;
        for (int col = 3; col < width - 3; ++col) {
            candIdx = row + col;
            isInterestPoint = isCorner(&dataPtr[candIdx], m_iThreshold, m_iN, &pixels16[0]);
            if (isInterestPoint) {
                CompVInterestPoint interestPoint(col, j, (float)m_iThreshold);
                if (m_bNonMaximaSupp) {
                    // for each pixel p the corner strength is defined as the maximum value of t that makes p a corner
                    // FIXME(dmi): add FAST_STRENGTH_TYPE_HIGHEST_THRESHOLD, FAST_STRENGTH_TYPE_SAD
                    while (isCorner(&dataPtr[candIdx], (int32_t)(interestPoint.strength + 1), m_iN, &pixels16[0])) {
                        ++interestPoint.strength;
                    }
                }
                mapPoints[candIdx] = interestPoint;
                interestPoints.push_back(interestPoint);
            }
        }
    }

    COMPV_ASSERT(num_corners == interestPoints.size());

#define MapExists(idx) (mapPoints.find((idx)) != mapPoints.end())

    // Non Maximal Suppression for removing adjacent corners
    if (m_bNonMaximaSupp) {
        for (int i = 0; i < interestPoints.size() - 1; ++i) {
            const CompVInterestPoint* point = &interestPoints[i];
            //COMPV_DEBUG_INFO("score=%d,%d", scores[i], interestPoints[i].strength);
            int current = (point->y * stride) + point->x;
            int left = point->x > 3 ? current - 1 : -1;
            int right = point->x < (width - 3 - 1) ? current + 1 : -1;
            int top = point->y > 3 ? ((point->y - 1) * stride) : -1;
            int bottom = point->y < (height - 3 - 1) ? ((point->y + 1) * stride) : -1;
            //COMPV_ASSERT(scores[i] == interestPoints[i].strength);
            int erase[5] = { -1, -1, -1, -1, -1 };
            float strengths[5] = { -1, -1, -1, -1, -1 }; // TODO(): FAST strength is "uint8_t" if not SAD

            erase[0] = current;
            strengths[0] = MapExists(current) ? mapPoints[current].strength : -1;

            // left
            if (left > 0 && MapExists(left) /*&& MapExists(current)*/) {
                erase[1] = left;
                strengths[1] = mapPoints[left].strength;
            }
            // right
            if (right > 0 && MapExists(right) /*&& MapExists(current)*/) {
                erase[2] = right;
                strengths[2] = mapPoints[right].strength;
            }
            // top
            if (top > 0 && MapExists(top) /*&& MapExists(current)*/) {
                erase[3] = top;
                strengths[3] = mapPoints[top].strength;
            }
            // bottom
            if (bottom > 0 && MapExists(bottom) /*&& MapExists(current)*/) {
                erase[4] = top;
                strengths[4] = mapPoints[bottom].strength;
            }
            int count = (erase[0] != -1) + (erase[1] != -1) + (erase[2] != -1) + (erase[3] != -1) + (erase[4] != -1);
            if (count > 1) {
                // at least 2 points
                // compute max
                float strength = strengths[0], idx = 0;
                if (strengths[1] > strength) {
                    strength = strengths[1], idx = 1;
                }
                if (strengths[2] > strength) {
                    strength = strengths[2], idx = 2;
                }
                if (strengths[3] > strength) {
                    strength = strengths[3], idx = 3;
                }
                if (strengths[4] > strength) {
                    strength = strengths[4], idx = 4;
                }
                // erase other values
                if (idx != 0 && erase[0] != -1) {
                    mapPoints.erase(erase[0]);
                }
                if (idx != 1 && erase[1] != -1) {
                    mapPoints.erase(erase[1]);
                }
                if (idx != 2 && erase[2] != -1) {
                    mapPoints.erase(erase[2]);
                }
                if (idx != 3 && erase[3] != -1) {
                    mapPoints.erase(erase[3]);
                }
            }
        }

        // clear old points
        interestPoints.clear();
        // copy from map to vector
        std::transform(mapPoints.begin(), mapPoints.end(), std::back_inserter(interestPoints), second(mapPoints));
    }

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDeteFAST::newObj(CompVObjWrapper<CompVFeatureDete* >* fast)
{
    COMPV_CHECK_EXP_RETURN(fast == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVFeatureDeteFAST* >_fast = new CompVFeatureDeteFAST();
    if (!_fast) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *fast = *_fast;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
