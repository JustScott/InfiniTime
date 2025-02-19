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

#include "displayapp/screens/SleepTime.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/Screen.h"
#include "components/settings/Settings.h"

#include <nrf_log.h>

using namespace Pinetime::Applications::Screens;

static void buttonEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<SleepTime*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

using namespace Pinetime::Applications::Screens;

SleepTime::SleepTime(Controllers::Settings::ClockType clockType) {
  static constexpr lv_color_t bgColor = Colors::bgAlt;

  // TODO: Check settingsController, and do hourCounter.SetMax(12 or 23)
  // TODO: Load counter values from sleepController (from file)
  hourCounter.Create();
  lv_obj_align(hourCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  hourCounter.SetValue(10);

  minuteCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  minuteCounter.SetValue(10);

  colonLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  buttonTimeOfDayToggle = lv_btn_create(lv_scr_act(), nullptr);
  buttonTimeOfDayToggle->user_data = this;
  lv_obj_set_event_cb(buttonTimeOfDayToggle, buttonEventHandler);
  lv_obj_set_size(buttonTimeOfDayToggle, 55, 55);
  lv_obj_align(buttonTimeOfDayToggle, lv_scr_act(), LV_ALIGN_CENTER, 0, 36);
  lv_obj_set_style_local_border_width(buttonTimeOfDayToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_border_color(buttonTimeOfDayToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  labelTimeOfDayToggle = lv_label_create(buttonTimeOfDayToggle, nullptr);
  lv_obj_set_style_local_bg_color(buttonTimeOfDayToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);

  if (clockType == Controllers::Settings::ClockType::H12) {
    if (timeOfDay == TimeOfDay::AM) {
      lv_label_set_text_static(labelTimeOfDayToggle, "AM");
    } else {
      lv_label_set_text_static(labelTimeOfDayToggle, "PM");
    }
  }

  buttonTimeInfo = lv_btn_create(lv_scr_act(), nullptr);
  buttonTimeInfo->user_data = this;
  lv_obj_set_event_cb(buttonTimeInfo, buttonEventHandler);
  lv_obj_set_size(buttonTimeInfo, 55, 55);
  lv_obj_align(buttonTimeInfo, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_obj_set_style_local_bg_color(buttonTimeInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);
  lv_obj_set_style_local_border_width(buttonTimeInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_border_color(buttonTimeInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  labelTimeInfo = lv_label_create(buttonTimeInfo, nullptr);
  lv_label_set_text_static(labelTimeInfo, "i");

  buttonAlarmTimerToggle = lv_btn_create(lv_scr_act(), nullptr);
  buttonAlarmTimerToggle->user_data = this;
  lv_obj_set_event_cb(buttonAlarmTimerToggle, buttonEventHandler);
  lv_obj_set_size(buttonAlarmTimerToggle, 115, 50);
  lv_obj_align(buttonAlarmTimerToggle, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -4, -4);
  lv_obj_set_style_local_bg_color(buttonAlarmTimerToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);

  labelAlarmTimerToggle = lv_label_create(buttonAlarmTimerToggle, nullptr);
  // TODO: Load from sleepController (also stored in file)
  if (timeType == TimeType::Alarm) {
    lv_label_set_text_static(labelAlarmTimerToggle, "Alarm");
  } else {
    lv_label_set_text_static(labelAlarmTimerToggle, "Timer");
  }

  buttonSwitchSleepOnOff = lv_switch_create(lv_scr_act(), nullptr);
  buttonSwitchSleepOnOff->user_data = this;
  lv_obj_set_event_cb(buttonSwitchSleepOnOff, buttonEventHandler);
  lv_obj_set_size(buttonSwitchSleepOnOff, 100, 50);
  // Align to the center of 115px from edge
  lv_obj_align(buttonSwitchSleepOnOff, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 4, -4);
  lv_obj_set_style_local_bg_color(buttonSwitchSleepOnOff, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, bgColor);

  // TODO: Check state from sleepController (from file), no
  //       animation on open
  lv_switch_off(buttonSwitchSleepOnOff, LV_ANIM_OFF);

  // Has to be at end of widgets to overlay over the rest
  buttonExitEndTimeInfo = lv_btn_create(lv_scr_act(), nullptr);
  buttonExitEndTimeInfo->user_data = this;
  lv_obj_set_event_cb(buttonExitEndTimeInfo, buttonEventHandler);
  lv_obj_set_height(buttonExitEndTimeInfo, 150);
  lv_obj_set_width(buttonExitEndTimeInfo, 200);
  lv_obj_align(buttonExitEndTimeInfo, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(buttonExitEndTimeInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);

  labelEndTimeInfo = lv_label_create(buttonExitEndTimeInfo, nullptr);

  HideEndTimeInfo();
}

SleepTime::~SleepTime() {
  lv_obj_clean(lv_scr_act());
}

void SleepTime::Refresh() {
}

void SleepTime::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == buttonTimeOfDayToggle) {
      // Implement with controller
      // if (sleepController.TimeOfDay() == sleepController::TimeOfDay::AM) {
      //  sleepController.ChangeTimeOfDay(sleepController::TimeOfDay::PM);
      //  lv_label_set_text_static(txtRecur, "PM");
      //} else {
      //  sleepController.ChangeTimeOfDay(sleepController::TimeOfDay::AM);
      //  lv_label_set_text_static(txtRecur, "AM");
      //}

      if (timeOfDay == TimeOfDay::AM) {
        timeOfDay = TimeOfDay::PM;
        lv_label_set_text_static(labelTimeOfDayToggle, "PM");
      } else {
        timeOfDay = TimeOfDay::AM;
        lv_label_set_text_static(labelTimeOfDayToggle, "AM");
      }
    } else if (obj == buttonTimeInfo) {
      ShowEndTimeInfo();
    } else if (obj == buttonExitEndTimeInfo) {
      HideEndTimeInfo();
    } else if (obj == buttonAlarmTimerToggle) {
      NRF_LOG_INFO("Clicked Toggle");

      if (timeType == TimeType::Alarm) {
        NRF_LOG_INFO("Switching to Timer");
        timeType = TimeType::Timer;
        NRF_LOG_INFO("Variable switched");
        lv_label_set_text_static(labelAlarmTimerToggle, "Timer");
        NRF_LOG_INFO("Label Switched");
      } else {
        NRF_LOG_INFO("Switching to Alarm");
        timeType = TimeType::Alarm;
        lv_label_set_text_static(labelAlarmTimerToggle, "Alarm");
      }
    } else if (obj == buttonSwitchSleepOnOff) {
      // TODO: Call a function here that hides widgets other can counters,
      //       colon, and switch, and turns on the relevant top icon
      if (lv_switch_get_state(buttonSwitchSleepOnOff)) {
        lv_switch_off(buttonSwitchSleepOnOff, LV_ANIM_OFF);
      } else {
        lv_switch_on(buttonSwitchSleepOnOff, LV_ANIM_OFF);
      }
    }
  }
}

void SleepTime::ShowEndTimeInfo() {
  // TODO: Only show alarm time for timer
  // TODO: Only show timer for alarm
  //   - Both will be shown above timer when it gets start, with live seconds
  // TODO: Implement 12H and 24H support
  // TODO: Only show needed time remaining, ex: "25s" instead of "0h 0m 25s"
  //       and make it a live countdown... might need to make this line its
  //       own function and attach task to it that only runs if not nullptr
  lv_label_set_text_fmt(labelEndTimeInfo, "%s 11:32 AM\n\n%s 1h 32m", Screens::Symbols::stopWatch, Screens::Symbols::hourGlass);

  lv_obj_set_hidden(labelEndTimeInfo, false);
  lv_obj_set_hidden(buttonExitEndTimeInfo, false);
}

void SleepTime::HideEndTimeInfo() {
  lv_obj_set_hidden(labelEndTimeInfo, true);
  lv_obj_set_hidden(buttonExitEndTimeInfo, true);
}

// TODO: Add function to hide widgets when alarm start and
//       show the widgets when stopped
// TODO: Add function to show the live timer with the timer
//       above it or the alarm time above it
