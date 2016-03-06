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
#if !defined(_COMPV_TIME_TIME_H_)
#define _COMPV_TIME_TIME_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

struct timeval;
struct timezone;
struct timespec;

COMPV_NAMESPACE_BEGIN()

#define COMPV_TIME_S_2_MS(S) ((S)*1000)
#define COMPV_TIME_MS_2_S(MS) ((MS)/1000)

class COMPV_API CompVTime : public CompVObj
{
public:
	static int getTimeOfDay(struct timeval *tv, struct timezone *tz);
	static uint64_t getTimeOfDayMillis();
	static uint64_t getMillis(const struct timeval *tv);
	static uint64_t getEpochMillis();
	static uint64_t getNowMills();
	static uint64_t getNtpMillis();
	static uint64_t getNtpMillis(const struct timeval *tv);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_TIME_TIME_H_ */
