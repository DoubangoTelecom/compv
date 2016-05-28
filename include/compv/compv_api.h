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

#include "compv/features/compv_feature.h"

#include "compv/matchers/compv_matcher.h"

#include "compv/math/compv_math_eigen.h"
#include "compv/math/compv_math_svd.h"

#include "compv/image/compv_image.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"

#include "compv/parallel/compv_asynctask.h"
#include "compv/parallel/compv_condvar.h"
#include "compv/parallel/compv_mutex.h"
#include "compv/parallel/compv_semaphore.h"
#include "compv/parallel/compv_thread.h"
#include "compv/parallel/compv_threaddisp.h"

#include "compv/time/compv_time.h"
#include "compv/time/compv_timer.h"

#endif /* _COMPV_API_H_ */
