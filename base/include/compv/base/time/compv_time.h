/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_TIME_TIME_H_)
#define _COMPV_BASE_TIME_TIME_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

struct timeval;
struct timezone;
struct timespec;

COMPV_NAMESPACE_BEGIN()

#define COMPV_TIME_S_2_MS(S) ((S)*1000)
#define COMPV_TIME_MS_2_S(MS) ((MS)/1000)

class COMPV_BASE_API CompVTime : public CompVObj
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

#endif /* _COMPV_BASE_TIME_TIME_H_ */
