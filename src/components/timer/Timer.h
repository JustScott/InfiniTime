#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace Controllers {
    class Timer {
    public:
      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

      void StartTimer(std::chrono::milliseconds duration);

      void StopTimer();

      std::chrono::milliseconds GetTimeRemaining() const;

      bool IsRunning() const;

      uint8_t timerOverflowIntervals = 0;

      TimerHandle_t timer;

      const uint32_t maxTimerMS = 3'600'000; // 1 hour

      void SetExpiredTime();

      void ResetExpiredTime();
    private:
      TickType_t expired = 0;
    };
  }
}
