/*
Copyright (c) 2023 Erik Rigtorp <erik@rigtorp.se>
SPDX-License-Identifier: MIT

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <atomic>
#include <chrono>

namespace rigtorp {

template <typename Clock = std::chrono::steady_clock> class TokenBucket {
public:
  TokenBucket() = default;

  TokenBucket(const uint64_t rate, const uint64_t burstSize)
      : timePerToken_(std::chrono::nanoseconds(std::chrono::seconds(1)) / rate),
        timePerBurst_(burstSize * timePerToken_) {}

  bool consume(const uint64_t tokens) {
    const auto now = Clock::now();
    const auto timeNeeded = tokens * timePerToken_;
    const auto minTime = now - timePerBurst_;
    auto oldTime = time_.load(std::memory_order_relaxed);

    for (;;) {
      auto newTime = oldTime;
      if (minTime > newTime) {
        newTime = minTime;
      }
      newTime += timeNeeded;
      if (newTime > now) {
        return false;
      }
      if (time_.compare_exchange_weak(oldTime, newTime,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        return true;
      }
    }

    return false;
  }

private:
  std::atomic<typename Clock::time_point> time_ = {Clock::time_point::min()};
  std::chrono::nanoseconds timePerToken_;
  std::chrono::nanoseconds timePerBurst_;
};

};