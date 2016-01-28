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

// TODO(dmi):
// Allow setting max number of features to retain
// Add support for ragel fast9 and fast12.

#include "compv/features/fast/compv_feature_fast_dete.h"
#include "compv/compv_mem.h"
#include "compv/compv_mathutils.h"

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <iostream>
#include <string>

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT	10
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP		true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES		-1 // maximum number of features to retain (<0 means all)

static int32_t COMPV_INLINE __continuousCount(int32_t fasType) {
	switch (fasType)
	{
	case COMPV_FAST_TYPE_9: return 9;
	case COMPV_FAST_TYPE_12: return 12;
	default:COMPV_DEBUG_ERROR("Invalid fastType:%d", fasType); return 9;
	}
}

static bool COMPV_INLINE __compareStrengthDec(const CompVInterestPoint& i, const  CompVInterestPoint& j) {
	return (i.strength > j.strength);
}
static bool COMPV_INLINE __isNonMaximal(const CompVInterestPoint & point) {
	return point.x < 0;
}

CompVFeatureDeteFAST::CompVFeatureDeteFAST()
	: CompVFeatureDete(COMPV_FAST_ID)
	, m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
	, m_iType(COMPV_FAST_TYPE_9)
	, m_iNumContinuous(__continuousCount(COMPV_FAST_TYPE_9))
	, m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
	, m_iMaxFeatures(COMPV_FEATURE_DETE_FAST_MAX_FEATURTES)
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
	case COMPV_FAST_SET_INT32_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int32_t threshold = *((int32_t*)valuePtr);
		m_iThreshold = COMPV_MATH_CLIP3(0, 255, threshold);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_FAST_SET_INT32_MAX_FEATURES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_iMaxFeatures = *((int32_t*)valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_FAST_SET_INT32_FAST_TYPE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int32_t iType = *((int32_t*)valuePtr);
		COMPV_CHECK_EXP_RETURN(iType != COMPV_FAST_TYPE_9 && iType != COMPV_FAST_TYPE_12, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_iType = iType;
		m_iNumContinuous = __continuousCount(iType);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_bNonMaximaSupp = *((bool*)valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

static void __strengthFast9(const uint8_t* Ix, uint8_t brighter, uint8_t darker, int threshold, int* strength)
{
	int strength_ = 0;
	int ndarker, nbrighter, ddarker, dbrighter;

	*strength = 0;

	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[0] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[1] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[2] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[3] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[4] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[5] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[6] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[7] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[8] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[0])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[1])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[2])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[3])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[4])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[5])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[6])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[7])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[8])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next0; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next0: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[1] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[2] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[3] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[4] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[5] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[6] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[7] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[8] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[9] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[1])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[2])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[3])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[4])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[5])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[6])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[7])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[8])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[9])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next1; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next1: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[2] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[3] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[4] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[5] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[6] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[7] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[8] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[9] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[10] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[2])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[3])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[4])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[5])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[6])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[7])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[8])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[9])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[10])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next2; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next2: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[3] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[4] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[5] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[6] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[7] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[8] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[9] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[10] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[11] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[3])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[4])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[5])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[6])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[7])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[8])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[9])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[10])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[11])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next3; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next3: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[4] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[5] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[6] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[7] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[8] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[9] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[10] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[11] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[12] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[4])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[5])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[6])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[7])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[8])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[9])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[10])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[11])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[12])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next4; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next4: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[5] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[6] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[7] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[8] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[9] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[10] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[11] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[12] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[13] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[5])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[6])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[7])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[8])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[9])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[10])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[11])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[12])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[13])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next5; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next5: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[6] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[7] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[8] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[9] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[10] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[11] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[12] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[13] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[14] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[6])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[7])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[8])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[9])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[10])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[11])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[12])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[13])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[14])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next6; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next6: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[7] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[8] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[9] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[10] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[11] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[12] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[13] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[14] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[15] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[7])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[8])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[9])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[10])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[11])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[12])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[13])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[14])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[15])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next7; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next7: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[8] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[9] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[10] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[11] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[12] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[13] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[14] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[15] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[0] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[8])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[9])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[10])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[11])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[12])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[13])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[14])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[15])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[0])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next8; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next8: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[9] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[10] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[11] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[12] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[13] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[14] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[15] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[0] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[1] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[9])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[10])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[11])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[12])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[13])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[14])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[15])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[0])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[1])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next9; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next9: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[10] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[11] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[12] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[13] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[14] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[15] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[0] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[1] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[2] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[10])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[11])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[12])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[13])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[14])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[15])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[0])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[1])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[2])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next10; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next10: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[11] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[12] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[13] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[14] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[15] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[0] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[1] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[2] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[3] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[11])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[12])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[13])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[14])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[15])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[0])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[1])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[2])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[3])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next11; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next11: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[12] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[13] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[14] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[15] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[0] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[1] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[2] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[3] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[4] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[12])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[13])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[14])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[15])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[0])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[1])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[2])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[3])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[4])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next12; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next12: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[13] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[14] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[15] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[0] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[1] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[2] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[3] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[4] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[5] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[13])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[14])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[15])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[0])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[1])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[2])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[3])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[4])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[5])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next13; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next13: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[14] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[15] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[0] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[1] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[2] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[3] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[4] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[5] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[6] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[14])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[15])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[0])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[1])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[2])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[3])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[4])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[5])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[6])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next14; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next14: void();
	ndarker = 255;
	nbrighter = 255;
	if ((dbrighter = (Ix[15] - brighter)) > 0) {
		if (dbrighter < nbrighter) nbrighter = dbrighter;
		if ((dbrighter = (Ix[0] - brighter)) > 0) {
			if (dbrighter < nbrighter) nbrighter = dbrighter;
			if ((dbrighter = (Ix[1] - brighter)) > 0) {
				if (dbrighter < nbrighter) nbrighter = dbrighter;
				if ((dbrighter = (Ix[2] - brighter)) > 0) {
					if (dbrighter < nbrighter) nbrighter = dbrighter;
					if ((dbrighter = (Ix[3] - brighter)) > 0) {
						if (dbrighter < nbrighter) nbrighter = dbrighter;
						if ((dbrighter = (Ix[4] - brighter)) > 0) {
							if (dbrighter < nbrighter) nbrighter = dbrighter;
							if ((dbrighter = (Ix[5] - brighter)) > 0) {
								if (dbrighter < nbrighter) nbrighter = dbrighter;
								if ((dbrighter = (Ix[6] - brighter)) > 0) {
									if (dbrighter < nbrighter) nbrighter = dbrighter;
									if ((dbrighter = (Ix[7] - brighter)) > 0) {
										if (dbrighter < nbrighter) nbrighter = dbrighter;
									}
									else nbrighter = 255;
								}
								else nbrighter = 255;
							}
							else nbrighter = 255;
						}
						else nbrighter = 255;
					}
					else nbrighter = 255;
				}
				else nbrighter = 255;
			}
			else nbrighter = 255;
		}
		else nbrighter = 255;
	}
	else nbrighter = 255;
	if ((ddarker = (darker - Ix[15])) > 0) {
		if (ddarker < ndarker) ndarker = ddarker;
		if ((ddarker = (darker - Ix[0])) > 0) {
			if (ddarker < ndarker) ndarker = ddarker;
			if ((ddarker = (darker - Ix[1])) > 0) {
				if (ddarker < ndarker) ndarker = ddarker;
				if ((ddarker = (darker - Ix[2])) > 0) {
					if (ddarker < ndarker) ndarker = ddarker;
					if ((ddarker = (darker - Ix[3])) > 0) {
						if (ddarker < ndarker) ndarker = ddarker;
						if ((ddarker = (darker - Ix[4])) > 0) {
							if (ddarker < ndarker) ndarker = ddarker;
							if ((ddarker = (darker - Ix[5])) > 0) {
								if (ddarker < ndarker) ndarker = ddarker;
								if ((ddarker = (darker - Ix[6])) > 0) {
									if (ddarker < ndarker) ndarker = ddarker;
									if ((ddarker = (darker - Ix[7])) > 0) {
										if (ddarker < ndarker) ndarker = ddarker;
									}
									else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
								}
								else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
							}
							else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
						}
						else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
					}
					else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
				}
				else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
			}
			else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
		}
		else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
	}
	else { if (nbrighter == 255) { goto next15; } ndarker = 255; }
	strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));
next15: void();
	if (strength_ > 0) { *strength = threshold + strength_ - 1; }
}

// FIXME
static void __strengthPrintCode(int N)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();

	int m, n, c;

	std::string code = "";

	for (m = 0; m < 16; ++m) {
		code += ("ndarker = 255;\n");
		code += ("nbrighter = 255;\n");
		c = N + m;

		// Brighter loop
		for (n = m; n < c; ++n) {
			code += ("if ((dbrighter = (Ix[" + std::to_string(n & 15) + "] - brighter)) > 0) {\n");
			code += ("if (dbrighter < nbrighter) nbrighter = dbrighter;\n");
		}
		for (n = m; n < c; ++n) {
			code += ("}\n else nbrighter = 255; \n");
		}

		// Darker loop
		for (n = m; n < c; ++n) {
			code += ("if ((ddarker = (darker - Ix[" + std::to_string(n & 15) + "])) > 0) {\n");
			code += ("if (ddarker < ndarker) ndarker = ddarker;\n");
		}
		for (n = m; n < c; ++n) {
			code += ("}\n else { if (nbrighter == 255) { goto next" + std::to_string(m) + "; } ndarker = 255; }\n"); // FIXME: remove goto compute_strength
		}

		// compute strength
		code += ("strength_ = COMPV_MATH_MAX_INT(strength_, COMPV_MATH_MIN_INT(ndarker, nbrighter));\n");

		// next
		code += ("next" + std::to_string(m) + ": void(); \n");
	}

	code += ("if (strength_ > 0) { *strength = threshold + strength_ - 1; }\n");

	FILE*file = fopen("./fast9.txt", "w+");
	if (file) {
		fwrite(code.c_str(), 1, code.length(), file);
		fclose(file);
	}
}

// FIXME: remove
static void __strength(const uint8_t* Ix, uint8_t brighter, uint8_t darker, int threshold, int N, int* strength)
{
	int ndarker, nbrighter, ddarker, dbrighter;
	int strength_ = 0, m, n, c;

	*strength = 0;

	for (m = 0; m < 16; ++m) {
		ndarker = 255;
		nbrighter = 255;
		c = N + m;
		// Brighter loop
		for (n = m; n < c; ++n) {
			dbrighter = (Ix[n & 15] - brighter);

			if (dbrighter <= 0) {
				nbrighter = 255;
				break;
			}
			if (dbrighter < nbrighter) nbrighter = dbrighter;
		}

		// Darker loop
		for (n = m; n < c; ++n) {
			ddarker = (darker - Ix[n & 15]);

			if (ddarker <= 0) {
				if (nbrighter == 255) {
					goto next;
				}
				ndarker = 255;
				break;
			}
			if (ddarker < ndarker) ndarker = ddarker;
		}
		strength_ = max(strength_, min(ndarker, nbrighter));
	next:
		void();
	}

	if (strength_ > 0) {
		*strength = threshold + strength_ - 1;
	}
}

// override CompVFeatureDete::process
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
	int pad = (stride - width), padPlus6 = 3 + pad + 3;
	int candIdx, sum, col, j;
	int32_t strength = 0;
	// TODO(dmi): make the allocations once
	float* strengthsMap = NULL;
	uint8_t brighter, darker;
	uint8_t IP[17];
	const uint8_t* IP0;

	if (m_bNonMaximaSupp) {
		strengthsMap = (float*)CompVMem::calloc(stride * height, sizeof(float)); // Must use calloc to fill the strengths with null values
		COMPV_CHECK_EXP_RETURN(!strengthsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

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

	// FIXME: remove
	//__strengthPrintCode(m_iNumContinuous);

	IP0 = dataPtr + (3 * stride) + 3;

	for (j = 3; j < height - 3; ++j) {
		for (col = 3; col < width - 3; ++col) {
			/*  Speed-Test */
			// compute brighter and darker
			brighter = CompVMathUtils::clampPixel8(*IP0 + m_iThreshold);
			darker = CompVMathUtils::clampPixel8(*IP0 - m_iThreshold);
			// compare I1 and I9
			IP[1] = IP0[pixels16[0]];
			IP[9] = IP0[pixels16[8]];
			sum = (IP[1] > brighter || IP[1] < darker) + (IP[9] > brighter || IP[9] < darker);
			// compare I5 and I13
			if (m_iNumContinuous != 12 || sum > 0) {
				IP[5] = IP0[pixels16[4]];
				IP[13] = IP0[pixels16[12]];
				sum += (IP[5] > brighter || IP[5] < darker) + (IP[13] > brighter || IP[13] < darker);
			}

			if ((sum >= 2 && (m_iNumContinuous != 12 || sum >= 3))) {
				IP[2] = IP0[pixels16[1]];
				IP[3] = IP0[pixels16[2]];
				IP[4] = IP0[pixels16[3]];
				IP[6] = IP0[pixels16[5]];
				IP[7] = IP0[pixels16[6]];
				IP[8] = IP0[pixels16[7]];
				IP[10] = IP0[pixels16[9]];
				IP[11] = IP0[pixels16[10]];
				IP[12] = IP0[pixels16[11]];
				IP[14] = IP0[pixels16[13]];
				IP[15] = IP0[pixels16[14]];
				IP[16] = IP0[pixels16[15]];
				//__strength(IP + 1, brighter, darker, m_iThreshold, m_iNumContinuous, &strength);
				__strengthFast9(IP + 1, brighter, darker, m_iThreshold, &strength);
				if (strength > 0) {
					interestPoints.push_back(CompVInterestPoint(col, j, (float)strength));
					if (strengthsMap) {
						candIdx = (j * stride) + col;
						strengthsMap[candIdx] = (float)strength;
					}
				}
			}
			IP0 += 1;
		}
		IP0 += padPlus6;
	}

	// Non Maximal Suppression for removing adjacent corners
	if (m_bNonMaximaSupp) {
		int32_t currentIdx;
		for (size_t i = 0; i < interestPoints.size(); ++i) {
			CompVInterestPoint* point = &interestPoints[i];
			currentIdx = (point->y * stride) + point->x;
			// No need to chech index as the point always has coords in (+3, +3)
			if (strengthsMap[currentIdx - 1] >= point->strength) { // left
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + 1] >= point->strength) { // right
				point->x = -1;
			}
			else if (strengthsMap[currentIdx - stride - 1] >= point->strength) { // left-top
				point->x = -1;
			}
			else if (strengthsMap[currentIdx - stride] >= point->strength) { // top
				point->x = -1;
			}
			else if (strengthsMap[currentIdx - stride + 1] >= point->strength) { // right-top
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + stride - 1] >= point->strength) { // left-bottom
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + stride] >= point->strength) { // bottom
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + stride + 1] >= point->strength) { // right-bottom
				point->x = -1;
			}
		}

		// Remove non maximal points
		interestPoints.erase(std::remove_if(interestPoints.begin(), interestPoints.end(), __isNonMaximal), interestPoints.end());
	}

	// Retain best "m_iMaxFeatures" features
	// TODO(dmi): use retainBest
	if (m_iMaxFeatures > 0 && (int32_t)interestPoints.size() > m_iMaxFeatures) {
		std::sort(interestPoints.begin(), interestPoints.end(), __compareStrengthDec);
		interestPoints.resize(m_iMaxFeatures);
	}


	CompVMem::free((void**)&strengthsMap);

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
