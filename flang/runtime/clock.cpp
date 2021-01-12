//===-- runtime/clock.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Implement time measurement intrinsic functions

#include "clock.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <time.h>
// TODO: windows, localtime_r/gettimeofday do not exists/ are different.
#ifndef _WIN32
#include <sys/time.h>

namespace Fortran::runtime {

void RTNAME(DateAndTime)(char *date, char *time, char *zone,
    std::size_t dateChars, std::size_t timeChars, std::size_t zoneChars) {
  timeval t;
  ::gettimeofday(&t, nullptr);
  time_t timer{t.tv_sec};
  tm localTime;
  ::localtime_r(&timer, &localTime);

  static constexpr int buffSize{16};
  char buffer[buffSize];
  auto copyBufferAndPad{
      [&](char *dest, std::size_t destChars, std::size_t len) {
        auto copyLen{std::min(len, destChars)};
        std::memcpy(dest, buffer, copyLen);
        for (auto i{copyLen}; i < destChars; ++i) {
          dest[i] = ' ';
        }
      }};
  if (date) {
    auto len = ::strftime(buffer, buffSize, "%Y%m%d", &localTime);
    copyBufferAndPad(date, dateChars, len);
  }
  if (time) {
    std::intmax_t ms{t.tv_usec / 1000};
    auto len{::snprintf(buffer, buffSize, "%02d%02d%02d.%03jd",
        localTime.tm_hour, localTime.tm_min, localTime.tm_sec, ms)};
    copyBufferAndPad(time, timeChars, len);
  }
  if (zone) {
    // Note: this may leave the buffer empty on many platforms. Classic flang
    // has a much more complex way of doing this (see __io_timezone in classic
    // flang).
    auto len{::strftime(buffer, buffSize, "%z", &localTime)};
    copyBufferAndPad(zone, zoneChars, len);
  }
}

long long RTNAME(SystemClockCount)(){
  timeval t;
  ::gettimeofday(&t, nullptr);
  time_t timer{t.tv_sec};
  tm localTime;
  ::localtime_r(&timer, &localTime);

  // 24 hours * 60 minutes * 60 seconds
  const int maxDayTime{86400};
  const int clockResolution{1000};
  auto timePointInADay{ localTime.tm_hour*3600 + localTime.tm_min*60 + localTime.tm_sec };
    // trip back to 0 if the value overflows of count_max.
    long long count =
        (timePointInADay > maxDayTime) ? 0 : (timePointInADay * clockResolution);
    return count;
}

long long RTNAME(SystemClockRate)(){
   constexpr long long clockResolution{1000};
   constexpr long long count_rate = clockResolution;
  return count_rate;
}

long long RTNAME(SystemClockMax)(){
  // 24 hours * 60 minutes * 60 *seconds
  constexpr int maxDayTime{86400};
  constexpr int clockResolution{1000};
  return maxDayTime * clockResolution;
}

void RTNAME(SystemClock)(int *count, int *count_rate, int *count_max) {
}

} // namespace Fortran::runtime

#else /* Windows */
// TODO: implement windows version (probably best to try merging implementations
// as much as possible).
namespace Fortran::runtime {
void RTNAME(DateAndTime)(
    char *, char *, char *, std::size_t, std::size_t, std::size_t) {
  // TODO
}
} // namespace Fortran::runtime
#endif
