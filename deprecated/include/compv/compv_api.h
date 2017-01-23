/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_API_H_)
#define _COMPV_API_H_ //!\\ Must not change this name, used as guard in private header files

#include "compv/compv_array.h"
#include "compv/compv_box.h"
#include "compv/compv_convlt.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"
#include "compv/compv_engine.h"
#include "compv/compv_gauss.h"
#include "compv/compv_hamming.h"
#include "compv/compv_interestpoint.h"
#include "compv/compv_list.h"
#include "compv/math/compv_math_trig.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_md5.h"
#include "compv/compv_mem.h"
#include "compv/compv_obj.h"
#include "compv/compv_fileutils.h"

#include "compv/calib/compv_calib_homography.h"

#include "compv/features/compv_feature.h"

#include "compv/matchers/compv_matcher.h"

#include "compv/math/compv_math.h"
#include "compv/math/compv_math_convlt.h"
#include "compv/math/compv_math_eigen.h"
#include "compv/math/compv_math_matrix.h"
#include "compv/math/compv_math_stats.h"
#include "compv/math/compv_math_transform.h"

#include "compv/image/compv_image.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"

#include "compv/parallel/compv_asynctask.h"
#include "compv/parallel/compv_asynctask11.h"
#include "compv/parallel/compv_condvar.h"
#include "compv/parallel/compv_mutex.h"
#include "compv/parallel/compv_semaphore.h"
#include "compv/parallel/compv_thread.h"
#include "compv/parallel/compv_threaddisp.h"
#include "compv/parallel/compv_threaddisp11.h"

#include "compv/time/compv_time.h"
#include "compv/time/compv_timer.h"

#endif /* _COMPV_API_H_ */
