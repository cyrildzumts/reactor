#ifndef COMMON_H
#define COMMON_H

#include "config.h"
#include <log.h>
#include "generator.h"
#include <chrono>
#include <utility>




using namespace std::chrono_literals;

typedef std::chrono::time_point<std::chrono::system_clock> time_point_ms_t;

    #ifdef TIME_UNIT_MS
        #define TIME_UNIT "ms"
        typedef  std::chrono::milliseconds duration_ms_t;
    #else
        #define TIME_UNIT "us"
        typedef  std::chrono::microseconds duration_ms_t;
    #endif //TIME_UNIT
#endif // COMMON_H

