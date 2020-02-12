/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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
    static int timeOfDay(struct timeval *tv, struct timezone *tz);
    static uint64_t timeOfDayMillis();
    static uint64_t millis(const struct timeval *tv);
    static uint64_t epochMillis();
    static uint64_t nowMillis();
    static uint64_t ntpMillis();
    static uint64_t ntpMillis(const struct timeval *tv);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_TIME_TIME_H_ */
