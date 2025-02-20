/*  Copyright (C) 2025 JustScott

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class SleepController {
    public:
      SleepController(Controllers::DateTime& dateTimeController, Controllers::FS& fs);

      void ScheduleAlarm();
      //      uint32_t SecondsToAlarm();

    private:
      static constexpr uint8_t sleepFormatVersion = 1;

      // TODO: Add all settings on other pages to this struct as well
      struct SleepSettings {
        uint8_t version = 1;
        uint8_t alarmHours = 7;
        uint8_t alarmMinutes = 0;
        uint8_t timerHours = 7;
        uint8_t timerMinutes = 0;
        bool isEnabled = false;
      };

      bool sleepSettingsChanged = false;

      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      SleepSettings sleepSettings;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> alarmTime;

      SleepSettings sleep;

      void LoadSettingsFromFile();
      void SaveSettingsToFile() const;
    };
  }
}
