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

#include "displayapp/apps/Apps.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/widgets/Counter.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SleepTime : public Screen {
      public:
        SleepTime(Controllers::Settings::ClockType clockType);
        ~SleepTime() override;

        void Refresh() override;

        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

      private:
        Controllers::Settings::ClockType clockType;

        void ShowEndTimeInfo();
        void HideEndTimeInfo();
        void ApplyExternalWidgetFactors();
        void ToggleSleepMode();

        enum class TimeOfDay : uint8_t { AM, PM };
        TimeOfDay timeOfDay = TimeOfDay::AM;

        enum class TimeType : uint8_t { Alarm, Timer };
        TimeType timeType = TimeType::Alarm;

        lv_obj_t* colonLabel;
        lv_obj_t* buttonTimeOfDayToggle;
        lv_obj_t* labelTimeOfDayToggle;
        lv_obj_t* buttonTimeInfo;
        lv_obj_t* labelTimeInfo;
        lv_obj_t* buttonExitEndTimeInfo;
        lv_obj_t* labelEndTimeInfo;
        lv_obj_t* buttonAlarmTimerToggle;
        lv_obj_t* labelAlarmTimerToggle;
        lv_obj_t* buttonSwitchSleepOnOff;
        lv_obj_t* labelActiveTimerAMPM;

        Widgets::Counter hourCounter = Widgets::Counter(0, 23, jetbrains_mono_76);
        Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);
      };
    }

    template <>
    struct AppTraits<Apps::SleepTime> {
      static constexpr Apps app = Apps::SleepTime;
      static constexpr const char* icon = Screens::Symbols::crescentMoon;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::SleepTime(controllers.settingsController.GetClockType());
      }
    };
  }
}
