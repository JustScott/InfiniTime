#pragma once

#include "Screen.h"
#include "Symbols.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"
#include <array>
#include <string>

namespace Pinetime::Applications {
  namespace Screens {
    class Calculator : public Screen {
    public:
      ~Calculator() override;

      Calculator(Controllers::MotorController& motorController);

      void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

      bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

    private:
      lv_obj_t *result, *returnButton, *buttonMatrix;

      char text[31];
      uint8_t position = 0;

      void eval();

      Controllers::MotorController& motorController;
    };
  }

  template <>
  struct AppTraits<Apps::Calculator> {
    static constexpr Apps app = Apps::Calculator;
    static constexpr const char* icon = Screens::Symbols::calculator;

    static Screens::Screen* Create(AppControllers& controllers) {
      return new Screens::Calculator(controllers.motorController);
    };
  };
}
