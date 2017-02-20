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

#include "compv/core/features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_sse2.h"
#include "compv/core/features/orb/intrin/x86/compv_core_feature_orb_desc_intrin_sse41.h"

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

static const COMPV_ALIGN_DEFAULT() float kCompVBrief256Pattern31AX[256] = { _kBrief256Pattern31AX_(0) };
static const COMPV_ALIGN_DEFAULT() float kCompVBrief256Pattern31AY[256] = { _kBrief256Pattern31AY_(0) };
static const COMPV_ALIGN_DEFAULT() float kCompVBrief256Pattern31BX[256] = { _kBrief256Pattern31BX_(0) };
static const COMPV_ALIGN_DEFAULT() float kCompVBrief256Pattern31BY[256] = { _kBrief256Pattern31BY_(0) };

#if COMPV_FEATURE_DESC_ORB_FXP_DESC
// The partten values are mulb cosT and sinT with fxpq(cosT) = fxpq(sinT) = 15 (maxv=0x7fff)
// The equation is simple: [[ COMPV_FXPQ = (fxpq(cosT/sinT) + fxpq(pattern)) ]] => [[ fxpq(pattern) = COMPV_FXPQ - (fxpq(cosT/sinT)) ]]
// With ARM NEON we have COMPV_FXPQ=15 which means we should have fxpq(pattern) = 0
// With X86 we have COMPV_FXPQ=16 which means we should have fxpq(pattern) = 1
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kCompVBrief256Pattern31AXFxp[256] = { _kBrief256Pattern31AX_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kCompVBrief256Pattern31AYFxp[256] = { _kBrief256Pattern31AY_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kCompVBrief256Pattern31BXFxp[256] = { _kBrief256Pattern31BX_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() int16_t kCompVBrief256Pattern31BYFxp[256] = { _kBrief256Pattern31BY_((COMPV_FXPQ - 15)) };
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

static void CompVOrbBrief256_31_32f_C(const uint8_t* img_center, compv_uscalar_t img_stride,
	const float* cos1, const float* sin1,
	const float* kBrief256Pattern31AX, const float* kBrief256Pattern31AY,
	const float* kBrief256Pattern31BX, const float* kBrief256Pattern31BY,
	void* out);
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
static void Brief256_31_Fxp_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out);
#endif

CompVCornerDescORB::CompVCornerDescORB()
	: CompVCornerDesc(COMPV_ORB_ID)
	, m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
	, m_nPatchBits(COMPV_FEATURE_DETE_ORB_PATCH_BITS)
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

COMPV_ERROR_CODE CompVCornerDescORB::describe(CompVImageScalePyramidPtr pPyramid, CompVInterestPointVector::const_iterator begin, CompVInterestPointVector::const_iterator end, uint8_t* desc, size_t desc_stride)
{
	float fx, fy, angleInRad, sf, fcos, fsin;
	int xi, yi, width, height;
	CompVInterestPointVector::const_iterator point;
	CompVMatPtr imageAtLevelN;
	const int nFeaturesBytes = (m_nPatchBits >> 3);
	const int nPatchRadius = (m_nPatchDiameter >> 1);
	const uint8_t* img_center;
	size_t img_stride;
	void(*Brief256_31_32f)(
		const uint8_t* img_center, compv_uscalar_t img_stride,
		const float* cos1, const float* sin1,
		const float* kBrief256Pattern31AX, const float* kBrief256Pattern31AY,
		const float* kBrief256Pattern31BX, const float* kBrief256Pattern31BY,
		COMPV_ALIGNED(x) void* out) = CompVOrbBrief256_31_32f_C;

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

	if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(Brief256_31_32f = CompVOrbBrief256_31_32f_Intrin_SSE2);
	}
	if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(Brief256_31_32f = CompVOrbBrief256_31_32f_Intrin_SSE41);
		//COMPV_EXEC_IFDEF_ASM_X86(Brief256_31_32f = CompVOrbBrief256_31_32f_Asm_X86_SSE41);
		//COMPV_EXEC_IFDEF_ASM_X64(Brief256_31_32f = CompVOrbBrief256_31_32f_Asm_X64_SSE41);
	}
	if (CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
		//COMPV_EXEC_IFDEF_INTRIN_X86(Brief256_31_32f = CompVOrbBrief256_31_32f_Intrin_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X86(Brief256_31_32f = CompVOrbBrief256_31_32f_Asm_X86_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X64(Brief256_31_32f = CompVOrbBrief256_31_32f_Asm_X64_AVX2);
		if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
			//COMPV_EXEC_IFDEF_ASM_X86(Brief256_31_32f = CompVOrbBrief256_31_32f_Asm_X86_FMA3_AVX2);
			//COMPV_EXEC_IFDEF_ASM_X64(Brief256_31_32f = CompVOrbBrief256_31_32f_Asm_X64_FMA3_AVX2);
		}
	}

#if 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
#endif

	// Describe the points
	for (point = begin; point < end; ++point) {
		// Get image at level N
		COMPV_CHECK_CODE_RETURN(pPyramid->image(point->level, &imageAtLevelN));
		img_stride = imageAtLevelN->stride();
		width = static_cast<int>(imageAtLevelN->cols());
		height = static_cast<int>(imageAtLevelN->rows());
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
			if ((xi - nPatchRadius) < 0 || (xi + nPatchRadius) >= width || (yi - nPatchRadius) < 0 || (yi + nPatchRadius) >= height) {
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
					Brief256_31_32f(img_center, img_stride, &fcos, &fsin, 
						kCompVBrief256Pattern31AX, kCompVBrief256Pattern31AY,
						kCompVBrief256Pattern31BX, kCompVBrief256Pattern31BY, 
						desc);
				}
				desc += desc_stride;
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
	CompVCornerDescORBPtr This = this;
	const bool bLevelZeroBlurred = false;

	const size_t nFeatures = interestPoints.size();
	const size_t nFeaturesBits = m_nPatchBits;
	const size_t nFeaturesBytes = nFeaturesBits >> 3;
	// Using 'Brief256_31' the descriptor's rows is 256b (32B) width. This is less than the cache1 line size which
	// is equal to 64B. Stridding the data we'll have 64B stride which means a cache lines will be loaded for each line.
	// To make the data cache-friendly we'll use a 32B stride which means one cache line for two rows.
	// Strideless data is also good for AVX512 as we'll be able to load two rows for each read.
	if (nFeaturesBytes == 32 && (CompVCpu::isEnabled(compv::kCpuFlagAVX512) || CompVCpu::isEnabled(compv::kCpuFlagARM_NEON))) { // TODO(dmi): On core i7 strideless data doesn't provide better perfs, this is why we also test check AVX512 for now
		COMPV_CHECK_CODE_RETURN(err_ = CompVMat::newObj<uint8_t>(&_descriptions, nFeatures, 32, CompVMem::bestAlignment(), 32));
	}
	else {
		COMPV_CHECK_CODE_RETURN(err_ = CompVMat::newObjAligned<uint8_t>(&_descriptions, nFeatures, nFeaturesBytes));
	}
	_descriptionsPtr = _descriptions->ptr<uint8_t>();
	size_t _descriptionsStride = _descriptions->stride();
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

	const size_t levelsCount = _pyramid->levels();

	// Get Max number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;

	// Get number of threads for the convolution process
	const size_t threadsCountBlur = (levelsCount > (maxThreads >> 2)) ? COMPV_MATH_MIN(maxThreads, levelsCount) : 1;

	// Gaussian blur on the pyramid
	if (threadsCountBlur > 1) {
		// levelStart is used to make sure we won't schedule more than "threadsCount"
		size_t levelStart, level, levelMax;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCountBlur);
		auto funcPtr = [&](size_t _level) -> COMPV_ERROR_CODE {
			return convlt(_pyramid, static_cast<int>(_level));
		};
		for (levelStart = bLevelZeroBlurred ? 1 : 0, levelMax = threadsCountBlur; levelStart < levelsCount; levelStart += threadsCountBlur, levelMax += threadsCountBlur) {
			for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, level), taskIds));
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
		}
	}
	else {
		// apply gaussianblur filter on the pyramid (the convolution function is already multi-threaded)
		for (size_t level = bLevelZeroBlurred ? 1 : 0; level < levelsCount; ++level) {
			COMPV_CHECK_CODE_RETURN(err_ = convlt(_pyramid, static_cast<int>(level)));  //!\\ already multi-threaded
		}
	}	

	// Get number of threads for the describe process
	const size_t threadsCountDiscribe = COMPV_MATH_MIN(maxThreads, (interestPoints.size() / COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD));

	// Describe the points
	if (threadsCountDiscribe > 1) {
		CompVAsyncTaskIds taskIds;
		CompVInterestPointVector::const_iterator begin = interestPoints.begin();
		const size_t counts = static_cast<size_t>(interestPoints.size() / threadsCountDiscribe);
		const size_t countsDescriptionsStride = (counts * _descriptionsStride);
		const size_t lastCount = interestPoints.size() - ((threadsCountDiscribe - 1) * counts);
		uint8_t* desc = _descriptionsPtr;
		taskIds.reserve(threadsCountDiscribe);
		auto funcPtr = [&](CompVInterestPointVector::const_iterator begin_, CompVInterestPointVector::const_iterator end_, uint8_t* desc_) -> COMPV_ERROR_CODE {
			return describe(_pyramid, begin_, end_, desc_, _descriptionsStride);
		};
		for (size_t i = 0; i < threadsCountDiscribe - 1; ++i) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, begin, begin + counts, desc), taskIds));
			begin += counts;
			desc += countsDescriptionsStride;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, begin, begin + lastCount, desc), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else {
		COMPV_CHECK_CODE_RETURN(err_ = describe(_pyramid, interestPoints.begin(), interestPoints.end(), _descriptionsPtr, _descriptionsStride));
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

static void CompVOrbBrief256_31_32f_C(
	const uint8_t* img_center, compv_uscalar_t img_stride, 
	const float* cos1, const float* sin1, 
	const float* kBrief256Pattern31AX, const float* kBrief256Pattern31AY,
	const float* kBrief256Pattern31BX, const float* kBrief256Pattern31BY,
	void* out
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	static const uint64_t u64_1 = 1;
	uint64_t* _out = reinterpret_cast<uint64_t*>(out);
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
