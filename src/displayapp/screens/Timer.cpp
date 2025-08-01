#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  if (event == LV_EVENT_PRESSED) {
    screen->ButtonPressed();
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    screen->MaskReset();
  } else if (event == LV_EVENT_SHORT_CLICKED) {
    screen->ToggleRunning();
  }
}

Timer::Timer(Controllers::Timer& timerController, Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : timer {timerController}, motorController {motorController}, wakeLock(systemTask) {

  lv_obj_t* colonMinutesSeconds = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonMinutesSeconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(colonMinutesSeconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(colonMinutesSeconds, ":");
  lv_obj_align(colonMinutesSeconds, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 40, 78);

  lv_obj_t* colonHoursMinutes = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonHoursMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(colonHoursMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(colonHoursMinutes, ":");
  lv_obj_align(colonHoursMinutes, lv_scr_act(), LV_ALIGN_IN_TOP_MID, -41, 78);

  hourCounter.Create();
  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align(hourCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 10, 26);
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_MID, 0, 26);
  lv_obj_align(secondCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, -10, 26);

  highlightObjectMask = lv_objmask_create(lv_scr_act(), nullptr);
  lv_obj_set_size(highlightObjectMask, 240, 50);
  lv_obj_align(highlightObjectMask, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_draw_mask_line_param_t tmpMaskLine;

  lv_draw_mask_line_points_init(&tmpMaskLine, 0, 0, 0, 240, LV_DRAW_MASK_LINE_SIDE_LEFT);
  highlightMask = lv_objmask_add_mask(highlightObjectMask, &tmpMaskLine);

  lv_obj_t* btnHighlight = lv_obj_create(highlightObjectMask, nullptr);
  lv_obj_set_style_local_radius(btnHighlight, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(btnHighlight, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_obj_set_size(btnHighlight, LV_HOR_RES, 50);
  lv_obj_align(btnHighlight, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  btnObjectMask = lv_objmask_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnObjectMask, 240, 50);
  lv_obj_align(btnObjectMask, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_draw_mask_line_points_init(&tmpMaskLine, 0, 0, 0, 240, LV_DRAW_MASK_LINE_SIDE_RIGHT);
  btnMask = lv_objmask_add_mask(btnObjectMask, &tmpMaskLine);

  btnPlayPause = lv_btn_create(btnObjectMask, nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_style_local_radius(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_set_size(btnPlayPause, LV_HOR_RES, 50);

  // Create the label as a child of the button so it stays centered by default
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);

  if (motorController.IsRinging()) {
    SetTimerRinging();
  } else if (timer.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::ButtonPressed() {
  pressTime = xTaskGetTickCount();
  buttonPressing = true;
}

void Timer::MaskReset() {
  buttonPressing = false;
  // A click event is processed before a release event,
  // so the release event would override the "Pause" text without this check
  if (!timer.IsRunning() && !isRinging) {
    lv_label_set_text_static(txtPlayPause, "Start");
  }
  maskPosition = 0;
  UpdateMask();
}

void Timer::UpdateMask() {
  lv_draw_mask_line_param_t maskLine;

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_LEFT);
  lv_objmask_update_mask(highlightObjectMask, highlightMask, &maskLine);

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_RIGHT);
  lv_objmask_update_mask(btnObjectMask, btnMask, &maskLine);
}

void Timer::Refresh() {
  if (isRinging) {
    DisplayTime();
    if (motorController.IsRinging()) {
      if (displaySeconds.Get().count() > 10) {
        // Stop buzzing after 10 seconds, but continue the counter
        motorController.StopRinging();
        wakeLock.Release();
      } else {
        // Keep the screen awake during the first 10 seconds
        wakeLock.Lock();
      }
    }
    // Reset timer after 1 minute
    if (displaySeconds.Get().count() > 60) {
      Reset();
    }
  } else if (timer.IsRunning()) {
    DisplayTime();
  } else if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    lv_label_set_text_static(txtPlayPause, "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      Reset();
    } else {
      UpdateMask();
    }
  }
}

void Timer::DisplayTime() {
  displaySeconds = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
  if (displaySeconds.IsUpdated()) {
    hourCounter.SetValue(displaySeconds.Get().count() / 3600);
    minuteCounter.SetValue((displaySeconds.Get().count() % 3600) / 60);
    secondCounter.SetValue(displaySeconds.Get().count() % 60);
  }
}

void Timer::SetTimerRunning() {
  hourCounter.HideControls();
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Pause");
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
}

void Timer::SetTimerStopped() {
  wakeLock.Release();
  isRinging = false;
  hourCounter.ShowControls();
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause, "Start");
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
}

void Timer::SetTimerRinging() {
  isRinging = true;
  hourCounter.HideControls();
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Reset");
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  timer.SetExpiredTime();
}

void Timer::ToggleRunning() {
  if (isRinging) {
    motorController.StopRinging();
    Reset();
  } else if (timer.IsRunning()) {
    DisplayTime();
    timer.StopTimer();
    SetTimerStopped();
  } else if (secondCounter.GetValue() + minuteCounter.GetValue() + hourCounter.GetValue() > 0) {
    std::chrono::milliseconds timerDuration = std::chrono::hours(hourCounter.GetValue()) + std::chrono::minutes(minuteCounter.GetValue()) +
                                              std::chrono::seconds(secondCounter.GetValue());
    timer.StartTimer(timerDuration);
    Refresh();
    SetTimerRunning();
  }
}

void Timer::Reset() {
  timer.ResetExpiredTime();
  hourCounter.SetValue(0);
  minuteCounter.SetValue(0);
  secondCounter.SetValue(0);
  SetTimerStopped();
}
