/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  J.E. Gain  (jgain@cs.uct.ac.za)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/

// timer.h: fairly accurate timing routines

#ifndef TimerC
#define TimerC

#ifdef _WIN32
#include <winsock.h>
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

static int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

    return 0;
}
#else
#include <sys/time.h>
#endif


class Timer
{

private:
    struct timeval tbegin, tend;
    struct timezone zone;

public:

    /// Start timer with call to timeofday
    void start();

    /// Stop timer with call to timeofday
    void stop();

    /// Get the current elapsed time between the latest calls to start and stop
    float peek();
};

#endif
