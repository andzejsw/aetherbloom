#ifndef TIME_H
#define TIME_H

#ifdef _WIN32
#include <windows.h>

static LARGE_INTEGER _frequency;
static bool _frequency_initialized = false;

#define NS_PER_SECOND (1000000000ULL)
#define NS_PER_MS (1000000ULL)

#define NOW() (\
    _frequency_initialized || (QueryPerformanceFrequency(&_frequency), _frequency_initialized = true), \
    (u64)((QueryPerformanceCounter((LARGE_INTEGER*)&_frequency.QuadPart), (_frequency.QuadPart * NS_PER_SECOND) / _frequency.QuadPart)) \
)

#else // Linux/Unix
#include <time.h>

#define NS_PER_SECOND (1000000000)
#define NS_PER_MS (1000000)

#define NOW() ({\
    struct timespec ts;\
    timespec_get(&ts, TIME_UTC);\
    ((ts.tv_sec * NS_PER_SECOND) + ts.tv_nsec);})
#endif

#endif