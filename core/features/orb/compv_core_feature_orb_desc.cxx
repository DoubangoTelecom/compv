/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
/* @description
* This class implements ORB (Oriented FAST and Rotated BRIEF) feature descriptor.
* Some literature:
* ORB final: https://www.willowgarage.com/sites/default/files/orb_final.pdf
* BRIEF descriptor: https://www.robots.ox.ac.uk/~vgg/rg/papers/CalonderLSF10.pdf
* Measuring Corner Properties: http://users.cs.cf.ac.uk/Paul.Rosin/corner2.pdf (check "Intensity centroid" used in ORB vs "Gradient centroid")
* Image moments: https://en.wikipedia.org/wiki/Image_moment
* Centroid: https://en.wikipedia.org/wiki/Centroid
*/
#include "compv/core/features/orb/compv_core_feature_orb_desc.h"
#include "compv/core/features/orb/compv_core_feature_orb_dete.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_convlt.h"
#include "compv/base/math/compv_math_gauss.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"

#include <algorithm>

#define COMPV_THIS_CLASSNAME "CompVCornerDescORB"

#define _kBrief256Pattern31AX_(q) \
	8*(1<<(q)), 4*(1<<(q)), -11*(1<<(q)), 7*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), 10*(1<<(q)), -13*(1<<(q)), -11*(1<<(q)), 7*(1<<(q)), -4*(1<<(q)), -13*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), 11*(1<<(q)),  \
	4*(1<<(q)), 5*(1<<(q)), 3*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -4*(1<<(q)), -10*(1<<(q)), 5*(1<<(q)), 5*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), -4*(1<<(q)), -8*(1<<(q)), 4*(1<<(q)), 0*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), \
	8*(1<<(q)), 0*(1<<(q)), 7*(1<<(q)), -13*(1<<(q)), 10*(1<<(q)), -6*(1<<(q)), 10*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), 3*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), 2*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), 6*(1<<(q)), -9*(1<<(q)), -2*(1<<(q)), \
	-12*(1<<(q)), 3*(1<<(q)), -7*(1<<(q)), -3*(1<<(q)), 2*(1<<(q)), -11*(1<<(q)), -1*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), -9*(1<<(q)), -12*(1<<(q)), 10*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -4*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), 7*(1<<(q)), \
	-13*(1<<(q)), 1*(1<<(q)), 2*(1<<(q)), -4*(1<<(q)), -1*(1<<(q)), 7*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), -1*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), 5*(1<<(q)), 2*(1<<(q)), 3*(1<<(q)), 2*(1<<(q)), 9*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), 1*(1<<(q)), 6*(1<<(q)), 2*(1<<(q)), \
	6*(1<<(q)), 3*(1<<(q)), 7*(1<<(q)), -11*(1<<(q)), -10*(1<<(q)), -5*(1<<(q)), -10*(1<<(q)), 8*(1<<(q)), 4*(1<<(q)), -10*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -5*(1<<(q)), 7*(1<<(q)), -9*(1<<(q)), -5*(1<<(q)), 8*(1<<(q)), -9*(1<<(q)), 1*(1<<(q)), 7*(1<<(q)), -2*(1<<(q)), 11*(1<<(q)), \
	-12*(1<<(q)), 3*(1<<(q)), 5*(1<<(q)), 0*(1<<(q)), -9*(1<<(q)), 0*(1<<(q)), -1*(1<<(q)), 5*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), -5*(1<<(q)), -4*(1<<(q)), 6*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), 1*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), 2*(1<<(q)), -2*(1<<(q)), 4*(1<<(q)), -6*(1<<(q)), \
	-3*(1<<(q)), 7*(1<<(q)), 4*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -8*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 10*(1<<(q)), -6*(1<<(q)), 8*(1<<(q)), 2*(1<<(q)), -11*(1<<(q)), -12*(1<<(q)), -11*(1<<(q)), 5*(1<<(q)), -2*(1<<(q)), -1*(1<<(q)), -13*(1<<(q)), \
	-10*(1<<(q)), -3*(1<<(q)), 2*(1<<(q)), -9*(1<<(q)), -4*(1<<(q)), -4*(1<<(q)), -6*(1<<(q)), 6*(1<<(q)), -13*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), -1*(1<<(q)), -4*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -8*(1<<(q)), -5*(1<<(q)), -13*(1<<(q)), \
	1*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), -9*(1<<(q)), -1*(1<<(q)), -13*(1<<(q)), 8*(1<<(q)), 2*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), -10*(1<<(q)), 4*(1<<(q)), 3*(1<<(q)), -4*(1<<(q)), 5*(1<<(q)), 4*(1<<(q)), -9*(1<<(q)), 0*(1<<(q)), -12*(1<<(q)), 3*(1<<(q)), \
	-10*(1<<(q)), 8*(1<<(q)), -8*(1<<(q)), 2*(1<<(q)), 10*(1<<(q)), 6*(1<<(q)), -7*(1<<(q)), -3*(1<<(q)), -1*(1<<(q)), -3*(1<<(q)), -8*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), 6*(1<<(q)), 3*(1<<(q)), 11*(1<<(q)), -3*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), \
	6*(1<<(q)), 0*(1<<(q)), -13*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), 2*(1<<(q)), -1*(1<<(q)), 9*(1<<(q)), 11*(1<<(q)), 3*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), 8*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), 7*(1<<(q)), 9*(1<<(q)), 7*(1<<(q)), -1*(1<<(q))
#define _kBrief256Pattern31AY_(q) \
	-3*(1<<(q)), 2*(1<<(q)), 9*(1<<(q)), -12*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), 4*(1<<(q)), -8*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), -5*(1<<(q)), 2*(1<<(q)), 0*(1<<(q)), -6*(1<<(q)), 6*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), \
	-3*(1<<(q)), -7*(1<<(q)), -7*(1<<(q)), 11*(1<<(q)), 12*(1<<(q)), 3*(1<<(q)), 2*(1<<(q)), -12*(1<<(q)), -12*(1<<(q)), -6*(1<<(q)), 0*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), -1*(1<<(q)), -12*(1<<(q)), -5*(1<<(q)), 11*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -2*(1<<(q)), \
	9*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), -5*(1<<(q)), -6*(1<<(q)), 7*(1<<(q)), -3*(1<<(q)), -9*(1<<(q)), 8*(1<<(q)), 0*(1<<(q)), 3*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), -4*(1<<(q)), 0*(1<<(q)), -7*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -10*(1<<(q)), -1*(1<<(q)), -5*(1<<(q)), \
	5*(1<<(q)), -10*(1<<(q)), -7*(1<<(q)), -2*(1<<(q)), 9*(1<<(q)), -13*(1<<(q)), 6*(1<<(q)), -3*(1<<(q)), -13*(1<<(q)), -6*(1<<(q)), -10*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), -13*(1<<(q)), 9*(1<<(q)), -1*(1<<(q)), 6*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), -7*(1<<(q)), \
	-3*(1<<(q)), -6*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), 1*(1<<(q)), -1*(1<<(q)), 1*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), -5*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), -12*(1<<(q)), 8*(1<<(q)), 6*(1<<(q)), -12*(1<<(q)), 4*(1<<(q)), 12*(1<<(q)), 12*(1<<(q)), -9*(1<<(q)), \
	3*(1<<(q)), 3*(1<<(q)), -3*(1<<(q)), 8*(1<<(q)), -5*(1<<(q)), 11*(1<<(q)), -8*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), -2*(1<<(q)), 0*(1<<(q)), -8*(1<<(q)), -6*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), -8*(1<<(q)), \
	-4*(1<<(q)), 1*(1<<(q)), -6*(1<<(q)), -9*(1<<(q)), 7*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), 12*(1<<(q)), 7*(1<<(q)), 2*(1<<(q)), 11*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), 9*(1<<(q)), -7*(1<<(q)), 5*(1<<(q)), 6*(1<<(q)), 6*(1<<(q)), -10*(1<<(q)), 1*(1<<(q)), -2*(1<<(q)), -12*(1<<(q)), \
	-13*(1<<(q)), 1*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), -2*(1<<(q)), 9*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), -4*(1<<(q)), 11*(1<<(q)), 6*(1<<(q)), 4*(1<<(q)), -5*(1<<(q)), -5*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), 0*(1<<(q)), -3*(1<<(q)), \
	-13*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), -2*(1<<(q)), 9*(1<<(q)), -3*(1<<(q)), -13*(1<<(q)), 6*(1<<(q)), 12*(1<<(q)), -11*(1<<(q)), -3*(1<<(q)), 11*(1<<(q)), 11*(1<<(q)), -5*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), 1*(1<<(q)), -12*(1<<(q)), -2*(1<<(q)), \
	5*(1<<(q)), -1*(1<<(q)), 7*(1<<(q)), 5*(1<<(q)), 0*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), 11*(1<<(q)), -3*(1<<(q)), -10*(1<<(q)), 1*(1<<(q)), -11*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -10*(1<<(q)), -8*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), 2*(1<<(q)), -13*(1<<(q)), \
	-13*(1<<(q)), 9*(1<<(q)), 3*(1<<(q)), 1*(1<<(q)), 2*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), -12*(1<<(q)), 2*(1<<(q)), 6*(1<<(q)), 8*(1<<(q)), 10*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -2*(1<<(q)), 2*(1<<(q)), -5*(1<<(q)), -9*(1<<(q)), -1*(1<<(q)), -1*(1<<(q)), \
	0*(1<<(q)), -11*(1<<(q)), -4*(1<<(q)), -6*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), -9*(1<<(q)), 7*(1<<(q)), -6*(1<<(q)), 5*(1<<(q)), -3*(1<<(q)), 0*(1<<(q)), 4*(1<<(q)), -6*(1<<(q)), 0*(1<<(q)), 8*(1<<(q)), 9*(1<<(q)), -4*(1<<(q)), \
	4*(1<<(q)), 3*(1<<(q)), -7*(1<<(q)), 0*(1<<(q)), -6*(1<<(q))
#define _kBrief256Pattern31BX_(q) \
	9*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), 12*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), -2*(1<<(q)), -11*(1<<(q)), -12*(1<<(q)), 11*(1<<(q)), -8*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), -7*(1<<(q)), 12*(1<<(q)), -2*(1<<(q)), -4*(1<<(q)), 12*(1<<(q)), 5*(1<<(q)), \
	10*(1<<(q)), 6*(1<<(q)), -6*(1<<(q)), -1*(1<<(q)), -8*(1<<(q)), -5*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), 6*(1<<(q)), 7*(1<<(q)), 4*(1<<(q)), 11*(1<<(q)), 4*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -7*(1<<(q)), 9*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -4*(1<<(q)), 10*(1<<(q)), \
	1*(1<<(q)), 11*(1<<(q)), -11*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), -8*(1<<(q)), 7*(1<<(q)), 10*(1<<(q)), 1*(1<<(q)), 5*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), -12*(1<<(q)), -11*(1<<(q)), -4*(1<<(q)), 12*(1<<(q)), -7*(1<<(q)), 0*(1<<(q)), \
	-7*(1<<(q)), 8*(1<<(q)), -4*(1<<(q)), -1*(1<<(q)), 5*(1<<(q)), -5*(1<<(q)), 0*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), -9*(1<<(q)), -8*(1<<(q)), 12*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), -3*(1<<(q)), 12*(1<<(q)), -5*(1<<(q)), -12*(1<<(q)), -2*(1<<(q)), 12*(1<<(q)), -11*(1<<(q)), \
	12*(1<<(q)), 3*(1<<(q)), -2*(1<<(q)), 1*(1<<(q)), 8*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -1*(1<<(q)), -10*(1<<(q)), 10*(1<<(q)), 12*(1<<(q)), 7*(1<<(q)), 6*(1<<(q)), 2*(1<<(q)), 4*(1<<(q)), 12*(1<<(q)), 10*(1<<(q)), -7*(1<<(q)), -4*(1<<(q)), 2*(1<<(q)), 7*(1<<(q)), 3*(1<<(q)), \
	11*(1<<(q)), 8*(1<<(q)), 9*(1<<(q)), -6*(1<<(q)), -5*(1<<(q)), -3*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), -8*(1<<(q)), 6*(1<<(q)), -2*(1<<(q)), -5*(1<<(q)), 10*(1<<(q)), -8*(1<<(q)), -5*(1<<(q)), 9*(1<<(q)), -9*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), -1*(1<<(q)), 12*(1<<(q)), \
	-6*(1<<(q)), 7*(1<<(q)), 10*(1<<(q)), 2*(1<<(q)), -5*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), 7*(1<<(q)), 6*(1<<(q)), -8*(1<<(q)), -3*(1<<(q)), -3*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), -5*(1<<(q)), 3*(1<<(q)), 8*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), 9*(1<<(q)), -3*(1<<(q)), -1*(1<<(q)), \
	12*(1<<(q)), 5*(1<<(q)), -9*(1<<(q)), 8*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -7*(1<<(q)), -12*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), 9*(1<<(q)), 2*(1<<(q)), -10*(1<<(q)), -7*(1<<(q)), -10*(1<<(q)), 11*(1<<(q)), -1*(1<<(q)), 0*(1<<(q)), -12*(1<<(q)), -10*(1<<(q)), \
	-2*(1<<(q)), 3*(1<<(q)), -4*(1<<(q)), -3*(1<<(q)), -2*(1<<(q)), -4*(1<<(q)), 6*(1<<(q)), -5*(1<<(q)), 12*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), -8*(1<<(q)), -6*(1<<(q)), -6*(1<<(q)), -4*(1<<(q)), -8*(1<<(q)), 5*(1<<(q)), 10*(1<<(q)), 10*(1<<(q)), \
	10*(1<<(q)), 1*(1<<(q)), -6*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), 10*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -5*(1<<(q)), -8*(1<<(q)), 8*(1<<(q)), 8*(1<<(q)), -3*(1<<(q)), 10*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), 3*(1<<(q)), -6*(1<<(q)), 4*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), \
	-6*(1<<(q)), 3*(1<<(q)), 11*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), -3*(1<<(q)), -1*(1<<(q)), -3*(1<<(q)), -8*(1<<(q)), 12*(1<<(q)), 3*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), -3*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), -11*(1<<(q)), 11*(1<<(q)), \
	1*(1<<(q)), -9*(1<<(q)), -6*(1<<(q)), -8*(1<<(q)), 8*(1<<(q)), 3*(1<<(q)), -1*(1<<(q)), 11*(1<<(q)), 12*(1<<(q)), 3*(1<<(q)), 0*(1<<(q)), 4*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), 8*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), 10*(1<<(q)), 12*(1<<(q)), 0*(1<<(q))
#define _kBrief256Pattern31BY_(q) \
	5*(1<<(q)), -12*(1<<(q)), 2*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), -4*(1<<(q)), -8*(1<<(q)), -9*(1<<(q)), 9*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), 0*(1<<(q)), -3*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), -8*(1<<(q)), 1*(1<<(q)), -3*(1<<(q)), \
	12*(1<<(q)), -2*(1<<(q)), -10*(1<<(q)), 10*(1<<(q)), -3*(1<<(q)), 7*(1<<(q)), 11*(1<<(q)), -7*(1<<(q)), -1*(1<<(q)), -5*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 4*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 3*(1<<(q)), -9*(1<<(q)), \
	7*(1<<(q)), 3*(1<<(q)), -10*(1<<(q)), 0*(1<<(q)), 1*(1<<(q)), 12*(1<<(q)), -4*(1<<(q)), -12*(1<<(q)), -4*(1<<(q)), 8*(1<<(q)), -7*(1<<(q)), -12*(1<<(q)), 6*(1<<(q)), -10*(1<<(q)), 5*(1<<(q)), 12*(1<<(q)), 8*(1<<(q)), 7*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), 5*(1<<(q)), \
	-13*(1<<(q)), 5*(1<<(q)), -7*(1<<(q)), -11*(1<<(q)), -13*(1<<(q)), -1*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), -4*(1<<(q)), -3*(1<<(q)), 12*(1<<(q)), 5*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), 5*(1<<(q)), -6*(1<<(q)), -7*(1<<(q)), -12*(1<<(q)), 12*(1<<(q)), \
	0*(1<<(q)), -13*(1<<(q)), 9*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), 3*(1<<(q)), 5*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), 11*(1<<(q)), 10*(1<<(q)), 3*(1<<(q)), -6*(1<<(q)), -13*(1<<(q)), 3*(1<<(q)), 9*(1<<(q)), -6*(1<<(q)), -8*(1<<(q)), -4*(1<<(q)), -2*(1<<(q)), 0*(1<<(q)), \
	-8*(1<<(q)), 3*(1<<(q)), -4*(1<<(q)), 10*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), -6*(1<<(q)), -11*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), 0*(1<<(q)), -2*(1<<(q)), 1*(1<<(q)), -4*(1<<(q)), -11*(1<<(q)), \
	4*(1<<(q)), 12*(1<<(q)), 8*(1<<(q)), 8*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 7*(1<<(q)), -9*(1<<(q)), -8*(1<<(q)), 9*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), 0*(1<<(q)), 12*(1<<(q)), -2*(1<<(q)), 10*(1<<(q)), -4*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), 3*(1<<(q)), \
	-5*(1<<(q)), 1*(1<<(q)), -11*(1<<(q)), -7*(1<<(q)), -5*(1<<(q)), 6*(1<<(q)), 6*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), -8*(1<<(q)), 9*(1<<(q)), 3*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), 8*(1<<(q)), 3*(1<<(q)), -9*(1<<(q)), -5*(1<<(q)), 8*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), -5*(1<<(q)), \
	11*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 0*(1<<(q)), -10*(1<<(q)), -7*(1<<(q)), 9*(1<<(q)), 11*(1<<(q)), 5*(1<<(q)), 6*(1<<(q)), -2*(1<<(q)), 7*(1<<(q)), -2*(1<<(q)), 7*(1<<(q)), -13*(1<<(q)), -8*(1<<(q)), -9*(1<<(q)), 5*(1<<(q)), 10*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), \
	-1*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), -10*(1<<(q)), -6*(1<<(q)), -6*(1<<(q)), -9*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), -9*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), \
	9*(1<<(q)), 12*(1<<(q)), -5*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), -12*(1<<(q)), 5*(1<<(q)), 9*(1<<(q)), 5*(1<<(q)), 4*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), 11*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 4*(1<<(q)), 6*(1<<(q)), 12*(1<<(q)), 1*(1<<(q)), 1*(1<<(q)), 1*(1<<(q)), -13*(1<<(q)), \
	-13*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -3*(1<<(q)), -2*(1<<(q)), 10*(1<<(q)), -9*(1<<(q)), -1*(1<<(q)), -2*(1<<(q)), -8*(1<<(q)), 5*(1<<(q)), 10*(1<<(q)), 5*(1<<(q)), 5*(1<<(q)), 11*(1<<(q)), -6*(1<<(q)), -12*(1<<(q)), 9*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -2*(1<<(q)), -11*(1<<(q))

COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AX[256] = { _kBrief256Pattern31AX_(0) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AY[256] = { _kBrief256Pattern31AY_(0) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BX[256] = { _kBrief256Pattern31BX_(0) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BY[256] = { _kBrief256Pattern31BY_(0) };
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
// The partten values are mulb cosT and sinT with fxpq(cosT) = fxpq(sinT) = 15 (maxv=0x7fff)
// The equation is simple: [[ COMPV_FXPQ = (fxpq(cosT/sinT) + fxpq(pattern)) ]] => [[ fxpq(pattern) = COMPV_FXPQ - (fxpq(cosT/sinT)) ]]
// With ARM NEON we have COMPV_FXPQ=15 which means we should have fxpq(pattern) = 0
// With X86 we have COMPV_FXPQ=16 which means we should have fxpq(pattern) = 1
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AXFxp[256] = { _kBrief256Pattern31AX_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AYFxp[256] = { _kBrief256Pattern31AY_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BXFxp[256] = { _kBrief256Pattern31BX_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BYFxp[256] = { _kBrief256Pattern31BY_((COMPV_FXPQ - 15)) };
#endif

COMPV_NAMESPACE_BEGIN()

// Default values from the detector
extern int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT;
extern int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT;
extern bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP;
extern int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
extern float COMPV_FEATURE_DETE_ORB_PYRAMID_SF;
extern int COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER;
extern int COMPV_FEATURE_DETE_ORB_PATCH_BITS;
extern COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE;

static const int COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE = 7;
static const float COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA = 1.52f;

#define COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD	(500 >> 3) // number of interestPoints

static void Brief256_31_Float32_C(const uint8_t* img_center, compv_uscalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out);
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
static void Brief256_31_Fxp_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out);
#endif

CompVCornerDescORB::CompVCornerDescORB()
	: CompVCornerDesc(COMPV_ORB_ID)
	, m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
	, m_nPatchBits(COMPV_FEATURE_DETE_ORB_PATCH_BITS)
	, m_bMediaTypeVideo(false)
	, m_funBrief256_31_Float32(Brief256_31_Float32_C)
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
	, m_funBrief256_31_Fxp(Brief256_31_Fxp_C)
#endif
{

}

CompVCornerDescORB::~CompVCornerDescORB()
{
}

// override CompVSettable::set
COMPV_ERROR_CODE CompVCornerDescORB::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVCaps::set(id, valuePtr, valueSize);
	}
}

COMPV_ERROR_CODE CompVCornerDescORB::get(int id, const void** valuePtrPtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtrPtr || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVCaps::get(id, valuePtrPtr, valueSize);
	}
}

COMPV_ERROR_CODE CompVCornerDescORB::convlt(CompVImageScalePyramidPtr pPyramid, int level)
{
	// apply gaussianblur filter on the pyramid
	CompVMatPtr imageAtLevelN;
	COMPV_CHECK_CODE_RETURN(pPyramid->image(level, &imageAtLevelN));
	// The out is the image itself to avoid allocating temp buffer. This means the images in the pyramid are modified
	// and any subsequent call must take care
	uint8_t* outPtr = imageAtLevelN->ptr<uint8_t>();

	if (m_kern_fxp) {
		// Fixed-point (multi-threaded)
		COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1FixedPoint(imageAtLevelN->ptr<const uint8_t>(), imageAtLevelN->cols(), imageAtLevelN->rows(), imageAtLevelN->stride(), m_kern_fxp->ptr<const uint16_t>(), m_kern_fxp->ptr<const uint16_t>(), COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, reinterpret_cast<uint8_t*&>(outPtr)));
	}
	else {
		// Floating-point (multi-threaded)
		COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1(imageAtLevelN->ptr<const uint8_t>(), imageAtLevelN->cols(), imageAtLevelN->rows(), imageAtLevelN->stride(), m_kern_float->ptr<const compv_float32_t>(), m_kern_float->ptr<const compv_float32_t>(), COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, reinterpret_cast<uint8_t*&>(outPtr)));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDescORB::describe(CompVImageScalePyramidPtr pPyramid, CompVInterestPointVector::const_iterator begin, CompVInterestPointVector::const_iterator end, uint8_t* desc)
{
	float fx, fy, angleInRad, sf, fcos, fsin;
	int xi, yi;
	CompVInterestPointVector::const_iterator point;
	CompVMatPtr imageAtLevelN;
	const int nFeaturesBytes = (m_nPatchBits >> 3);
	const int nPatchRadius = (m_nPatchDiameter >> 1);
	const uint8_t* img_center;
	size_t stride;

#if 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
#endif

	// Describe the points
	for (point = begin; point < end; ++point) {
		// Get image at level N
		COMPV_CHECK_CODE_RETURN(pPyramid->image(point->level, &imageAtLevelN));
		stride = imageAtLevelN->stride();
		// Scale
		sf = pPyramid->scaleFactor(point->level);
		fx = (point->x * sf);
		fy = (point->y * sf);
		// Convert the angle from degree to radian
		angleInRad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(point->orient);
		// Get angle's cos and sin
		fcos = ::cos(angleInRad);
		fsin = ::sin(angleInRad);
		// Round the point
		xi = COMPV_MATH_ROUNDFU_2_INT(fx, int);
		yi = COMPV_MATH_ROUNDFU_2_INT(fy, int);
		// Compute description
		{
			// Check if the keypoint is too close to the border
			if ((xi - nPatchRadius) < 0 || (xi + nPatchRadius) >= imageAtLevelN->cols() || (yi - nPatchRadius) < 0 || (yi + nPatchRadius) >= imageAtLevelN->rows()) {
				// Must never happen....unless you are using keypoints from another implementation (e.g. OpenCV)
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Keypoint too close to the border");
				memset(desc, 0, nFeaturesBytes);
			}
			else {
				img_center = imageAtLevelN->ptr<const uint8_t>(yi, xi); // Translate the image to have the keypoint at the center. This is required before applying the rotated patch.
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
				if (CompVEngine::isMathFixedPointEnabled()) {
					// cosT and sinT are within [-1, 1] which means we can just mulb 0x7fff
					int16_t cosTQ15 = COMPV_MATH_ROUNDF_2_INT((fcos * 0x7fff), int16_t);
					int16_t sinTQ15 = COMPV_MATH_ROUNDF_2_INT((fsin * 0x7fff), int16_t);
					m_funBrief256_31_Fxp(img_center, imageAtLevelN->getStride(), &cosTQ15, &sinTQ15, desc);
				}
				else
#endif
				{
					m_funBrief256_31_Float32(img_center, stride, &fcos, &fsin, desc);
				}
				desc += nFeaturesBytes;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// override CompVCornerDesc::process
COMPV_ERROR_CODE CompVCornerDescORB::process(const CompVMatPtr& image_, const CompVInterestPointVector& interestPoints, CompVMatPtrPtr descriptions)
{
	COMPV_CHECK_EXP_RETURN(!image_ || image_->isEmpty() || !descriptions,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Convert the image to grayscale if not already the case
	CompVMatPtr image = image_;
	if (image_->subType() != COMPV_SUBTYPE_PIXELS_Y) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should convert the image to grayscale once and reuse it in all functions");
		COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image_, &m_ptrImageGray), "Failed to convert the image to grayscale");
		image = m_ptrImageGray;
	}

	// For now only Brief256_31 is supported
	COMPV_CHECK_EXP_RETURN(m_nPatchBits != 256 || m_nPatchDiameter != 31, COMPV_ERROR_CODE_E_INVALID_CALL, "Requires Brief256_31");

	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVMatPtr _descriptions;
	CompVImageScalePyramidPtr _pyramid;
	CompVMatPtr imageAtLevelN;
	CompVCornerDetePtr& _attachedDete = attachedDete();
	uint8_t* _descriptionsPtr = NULL;
	size_t threadsCount = 1, levelsCount, threadStartIdx = 0;
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	CompVCornerDescORBPtr This = this;
	bool bLevelZeroBlurred = false;

	const size_t nFeatures = interestPoints.size();
	const size_t nFeaturesBits = m_nPatchBits;
	const size_t nFeaturesBytes = nFeaturesBits >> 3;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMat::newObjStrideless<uint8_t>(&_descriptions, nFeatures, nFeaturesBytes)); // do not align nFeaturesBytes(32) which is already good for AVX, SSE and NEON
	_descriptionsPtr = _descriptions->ptr<uint8_t>();
	if (nFeatures == 0) {
		return COMPV_ERROR_CODE_S_OK;
	}

	// Get the pyramid from the detector or use or own pyramid
	if (_attachedDete) {
		switch (_attachedDete->id()) {
		case COMPV_ORB_ID: {
			const void* valuePtr = NULL;
			COMPV_CHECK_CODE_RETURN(err_ = _attachedDete->get(COMPV_FEATURE_GET_PTR_PYRAMID, &valuePtr, sizeof(CompVImageScalePyramid)));
			_pyramid = reinterpret_cast<CompVImageScalePyramid*>(const_cast<void*>(valuePtr));
			break;
		}
		}
	}
	if (!_pyramid) {
		// This code is called when we fail to get a pyramid from the attached detector or when none is attached.
		// The pyramid should come from the detector. Attach a detector to this descriptor to give it access to the pyramid.
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should attach a detector to the descriptor to avoid computing the pyramid twice. Sad!!");
		COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image)); // multithreaded function
		_pyramid = m_pyramid;
	}

	levelsCount = _pyramid->levels();

	// Check if we have the same image
#if 0
	if (m_bMediaTypeVideo && m_image_blurred_prev) {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING();
		// apply gaussianblur filter on the image at level 0
		COMPV_CHECK_CODE_RETURN(err_ = convlt(_pyramid, 0));
		// get blurred image
		COMPV_CHECK_CODE_RETURN(err_ = _pyramid->image(0, &imageAtLevelN));
		bLevelZeroBlurred = true; // to avoid apply gaussian blur again
		bool bSameImageAsPrev = false;
		COMPV_CHECK_CODE_RETURN(err_ = m_image_blurred_prev->isEquals(imageAtLevelN, bSameImageAsPrev, 15, m_image_blurred_prev->getHeight() - 15, 15, m_image_blurred_prev->getWidth() - 15));
		// TODO(dmi): Do not use equality but SAD with thredshold: "(abs(img1_ptr[x] - img2_ptr[x]) > t"
	}
#endif

	// apply gaussianblur filter on the pyramid
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found"); // TODO(dmi): not need to multi-thread the convolution, already the case
#if 0
	if (threadsCount > 1) {
		// levelStart is used to make sure we won't schedule more than "threadsCount"
		int levelStart, level, levelMax;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](int level) -> COMPV_ERROR_CODE {
			return convlt(_pyramid, level);
		};
		for (levelStart = bLevelZeroBlurred ? 1 : 0, levelMax = threadsCount; levelStart < levelsCount; levelStart += threadsCount, levelMax += threadsCount) {
			for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, level), taskIds));
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
		}
	}
	else {
#endif
		for (int level = bLevelZeroBlurred ? 1 : 0; level < levelsCount; ++level) {
			COMPV_CHECK_CODE_RETURN(err_ = convlt(_pyramid, level));  // multi-threaded
		}
#if 0
	}
#endif

	/* Init "m_funBrief256_31" using current CPU flags */
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
	if (CompVEngine::isMathFixedPointEnabled()) {
		// ARM = FXPQ15, X86 = FXPQ16
		if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Fxp = Brief256_31_Fxpq16_Intrin_SSE2);
		}
	}
	else
#endif
#if 0
		if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Float32 = Brief256_31_Intrin_SSE2);
		}
		if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
			COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_SSE41);
			COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_SSE41);
		}
		if (compv::CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Float32 = Brief256_31_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_AVX2);
			if (compv::CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_FMA3_AVX2);
				COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_FMA3_AVX2);
			}
		}
#endif

	// Describe the points
	CompVAsyncTaskIds describeTaskIds;
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
#if 0
	size_t threadsCountDescribe = 1;
	if (threadsCount > 1) {
		threadsCountDescribe = (interestPoints->size() / COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD);
		threadsCountDescribe = COMPV_MATH_MIN(threadsCountDescribe, threadsCount);
	}
	if (threadsCountDescribe > 1) {
		const CompVInterestPoint* begin = interestPoints->begin();
		size_t total = interestPoints->size();
		size_t count = total / threadsCountDescribe;
		uint8_t* desc = _descriptionsPtr;
		describeTaskIds.reserve(threadsCountDescribe);
		auto funcPtr = [&](const CompVInterestPoint* begin, const CompVInterestPoint* end, uint8_t* desc) -> COMPV_ERROR_CODE {
			return describe(_pyramid, begin, end, desc);
		};
		for (size_t i = 0; count > 0 && i < threadsCountDescribe; ++i) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, begin, begin + count, desc), describeTaskIds));
			begin += count;
			desc += (count * nFeaturesBytes);
			total -= count;
			if (i == (threadsCountDescribe - 2)) {
				count = (total); // the remaining
			}
		}
		// wait() for threads execution later
	}
	else {
#endif
		COMPV_CHECK_CODE_RETURN(err_ = describe(_pyramid, interestPoints.begin(), interestPoints.end(), _descriptionsPtr));
#if 0
	}
#endif

	// TODO(dmi): if MT, call wait() after image cloning
#if 0
	if (m_bMediaTypeVideo) {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING();
		COMPV_CHECK_CODE_RETURN(err_ = _pyramid->getImage(0, &imageAtLevelN));
		COMPV_CHECK_CODE_RETURN(err_ = imageAtLevelN->clone(&m_image_blurred_prev));
	}
#endif

	// Wait for the threads to finish the work
	if (!describeTaskIds.empty()) {
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(describeTaskIds));
	}

	*descriptions = _descriptions;

	return err_;
}

COMPV_ERROR_CODE CompVCornerDescORB::newObj(CompVCornerDescPtrPtr orb)
{
	COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVImageScalePyramid * > pyramid_;
	CompVMatPtr kern_float_;
	CompVMatPtr kern_fxp_;

	// Create Gauss kernel values
#if COMPV_FEATURE_DESC_ORB_FXP_CONVLT
	if (CompVCpu::isMathFixedPointEnabled()) {
		COMPV_CHECK_CODE_RETURN(CompVMathGauss::kernelDim1FixedPoint(&kern_fxp_, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA));
	}
#endif /* COMPV_FEATURE_DESC_ORB_FXP_CONVLT */
	if (!kern_fxp_) {
		COMPV_CHECK_CODE_RETURN(CompVMathGauss::kernelDim1<compv_float32_t>(&kern_float_, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA));
	}
	// Create the pyramid
	COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(&pyramid_, COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE));

	CompVCornerDescORBPtr _orb = new CompVCornerDescORB();
	COMPV_CHECK_EXP_RETURN(!_orb, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	
	_orb->m_pyramid = pyramid_;
	_orb->m_kern_float = kern_float_;
	_orb->m_kern_fxp = kern_fxp_; // Fixed-Point is defined only if isMathFixedPointEnabled() is true

	*orb = *_orb;
	return COMPV_ERROR_CODE_S_OK;
}

static void Brief256_31_Float32_C(const uint8_t* img_center, compv_uscalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	static const uint64_t u64_1 = 1;
	uint64_t* _out = (uint64_t*)out;
	int i, j, x, y;
	uint8_t a, b;
	float xf, yf, cosT = *cos1, sinT = *sin1;

	// 256bits = 32Bytes = 4 uint64
	_out[0] = _out[1] = _out[2] = _out[3] = 0;

	// Applying rotation matrix to each (x, y) point in the patch gives us:
	// xr = x*cosT - y*sinT and yr = x*sinT + y*cosT
	for (i = 0, j = 0; i < 256; ++i) {
		xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
		yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
		x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		a = img_center[(y * img_stride) + x];

		xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
		yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
		x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		b = img_center[(y * img_stride) + x];

		_out[0] |= (a < b) ? (u64_1 << j) : 0;
		if (++j == 64) {
			++_out;
			j = 0;
		}
	}
}

#if COMPV_FEATURE_DESC_ORB_FXP_DESC
static void Brief256_31_Fxp_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	static const uint64_t u64_1 = 1;
	uint64_t* _out = (uint64_t*)out;
	int i, j, x, y;
	uint8_t a, b;
	int cosT = *cos1;
	int sinT = *sin1;

	// 256bits = 32Bytes = 4 uint64
	_out[0] = _out[1] = _out[2] = _out[3] = 0;

	// Applying rotation matrix to each (x, y) point in the patch gives us:
	// xr = x*cosT - y*sinT and yr = x*sinT + y*cosT
	for (i = 0, j = 0; i < 256; ++i) {
		x = (kBrief256Pattern31AXFxp[i] * cosT - kBrief256Pattern31AYFxp[i] * sinT) >> COMPV_FXPQ;
		y = (kBrief256Pattern31AXFxp[i] * sinT + kBrief256Pattern31AYFxp[i] * cosT) >> COMPV_FXPQ;
		a = img_center[(y * img_stride) + x];

		x = (kBrief256Pattern31BXFxp[i] * cosT - kBrief256Pattern31BYFxp[i] * sinT) >> COMPV_FXPQ;
		y = (kBrief256Pattern31BXFxp[i] * sinT + kBrief256Pattern31BYFxp[i] * cosT) >> COMPV_FXPQ;
		b = img_center[(y * img_stride) + x];

		_out[0] |= (a < b) ? (u64_1 << j) : 0;
		if (++j == 64) {
			++_out;
			j = 0;
		}
	}
}
#endif

COMPV_NAMESPACE_END()
