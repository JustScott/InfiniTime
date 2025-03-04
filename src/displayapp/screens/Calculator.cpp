#include "Calculator.h"
#include <string>
#include <stack>
#include <cfloat>
#include <cmath>
#include <map>
#include <memory>

using namespace Pinetime::Applications::Screens;

// Anonymous Namespace for all the structs
namespace {
  struct CalcTreeNode {
    virtual double calculate() = 0;
  };

  struct NumNode : CalcTreeNode {
    double value;

    double calculate() override {
      return value;
    };
  };

  struct BinOp : CalcTreeNode {
    std::shared_ptr<CalcTreeNode> left;
    std::shared_ptr<CalcTreeNode> right;

    char op;

    double calculate() override {
      // make sure we have actual numbers
      if (!right || !left) {
        errno = EINVAL;
        return 0.0;
      }

      double rightVal = right->calculate();
      double leftVal = left->calculate();
      switch (op) {
        case '^':
          // detect overflow
          if (log2(leftVal) + rightVal > 31) {
            errno = ERANGE;
            return 0.0;
          }
          return pow(leftVal, rightVal);
        case 'x':
          // detect over/underflowflow
          if ((DBL_MAX / abs(rightVal)) < abs(leftVal)) {
            errno = ERANGE;
            return 0.0;
          }
          return leftVal * rightVal;
        case '/':
          // detect under/overflow
          if ((DBL_MAX * abs(rightVal)) < abs(leftVal)) {
            errno = ERANGE;
            return 0.0;
          }
          // detect divison by zero
          if (rightVal == 0.0) {
            errno = EDOM;
            return 0.0;
          }
          return leftVal / rightVal;
        case '+':
          // detect overflow
          if ((DBL_MAX - rightVal) < leftVal) {
            errno = ERANGE;
            return 0.0;
          }
          return leftVal + rightVal;
        case '-':
          // detect underflow
          if ((DBL_MIN + rightVal) > leftVal) {
            errno = ERANGE;
            return 0.0;
          }
          return leftVal - rightVal;
      }
      errno = EINVAL;
      return 0.0;
    };
  };

  uint8_t getPrecedence(char op) {
    switch (op) {
      case '^':
        return 4;
      case 'x':
      case '/':
        return 3;
      case '+':
      case '-':
        return 2;
    }
    return 0;
  }

  bool leftAssociative(char op) {
    switch (op) {
      case '^':
        return false;
      case 'x':
      case '/':
      case '+':
      case '-':
        return true;
    }
    return false;
  }

}

static void eventHandler(lv_obj_t* obj, lv_event_t event) {
  auto calc = static_cast<Calculator*>(obj->user_data);
  calc->OnButtonEvent(obj, event);
}

Calculator::~Calculator() {
  lv_obj_clean(lv_scr_act());
}

static const char* buttonMap1[] = {
  "7", "8", "9", "/", "\n",
  "4", "5", "6", "x", "\n",
  "1", "2", "3", "-", "\n",
  ".", "0", "=", "+", "",
};

static const char* buttonMap2[] = {
  "7", "8", "9", "(", "\n",
  "4", "5", "6", ")", "\n",
  "1", "2", "3", "^", "\n",
  ".", "0", "=", "+", "",
};

Calculator::Calculator(Controllers::MotorController& motorController) : motorController {motorController} {
  result = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(result, LV_LABEL_LONG_BREAK);
  lv_label_set_text(result, "0");
  lv_obj_set_size(result, 180, 60);
  lv_obj_set_pos(result, 0, 0);

  returnButton = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(returnButton, 52, 52);
  lv_obj_set_pos(returnButton, 186, 0);
  lv_obj_t* returnLabel;
  returnLabel = lv_label_create(returnButton, nullptr);
  lv_label_set_text(returnLabel, "<=");
  lv_obj_align(returnLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
  returnButton->user_data = this;
  lv_obj_set_event_cb(returnButton, eventHandler);

  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  lv_btnmatrix_set_map(buttonMatrix, buttonMap1);
  lv_obj_set_size(buttonMatrix, 240, 180);
  lv_obj_set_pos(buttonMatrix, 0, 60);
  lv_obj_set_style_local_pad_all(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, eventHandler);
}

void Calculator::eval() {
  std::stack<char> input {};
  for (int8_t i = position - 1; i >= 0; i--) {
    input.push(text[i]);
  }
  std::stack<std::shared_ptr<CalcTreeNode>> output {};
  std::stack<char> operators {};
  bool expectingNumber = true;
  int8_t sign = +1;
  while (!input.empty()) {
    if (input.top() == '.') {
      input.push('0');
    }
    if (isdigit(input.top())) {
      char numberStr[31];
      uint8_t strln = 0;
      uint8_t pointpos = 0;
      while (!input.empty() && (isdigit(input.top()) || input.top() == '.')) {
        if (input.top() == '.') {
          if (pointpos != 0) {
            motorController.RunForDuration(10);
            return;
          }
          pointpos = strln;
        } else {
          numberStr[strln] = input.top();
          strln++;
        }
        input.pop();
      }
      // replacement for strtod() since using that increased .txt by 76858 bzt
      if (pointpos == 0) {
        pointpos = strln;
      }
      double num = 0;
      for (uint8_t i = 0; i < pointpos; i++) {
        num += (numberStr[i] - '0') * pow(10, pointpos - i - 1);
      }
      for (uint8_t i = 0; i < strln - pointpos; i++) {
        num += (numberStr[i + pointpos] - '0') / pow(10, i + 1);
      }

      auto number = std::make_shared<NumNode>();
      number->value = sign * num;
      output.push(number);

      sign = +1;
      expectingNumber = false;
      continue;
    }

    if (expectingNumber && input.top() == '+') {
      input.pop();
      continue;
    }
    if (expectingNumber && input.top() == '-') {
      sign *= -1;
      input.pop();
      continue;
    }

    char next = input.top();
    input.pop();

    switch (next) {
      case '+':
      case '-':
      case '/':
      case 'x':
      case '^':
        // while ((there is an operator at the top of the operator stack)
        while (!operators.empty()
               // and (the operator at the top of the operator stack is not a left parenthesis))
               && operators.top() != '('
               // and ((the operator at the top of the operator stack has greater precedence)
               && (getPrecedence(operators.top()) > getPrecedence(next)
                   // or (the operator at the top of the operator stack has equal precedence and the token is left associative))
                   || (getPrecedence(operators.top()) == getPrecedence(next) && leftAssociative(next)))) {
          // need two elements on the output stack to add a binary operator
          if (output.size() < 2) {
            motorController.RunForDuration(10);
            return;
          }
          auto node = std::make_shared<BinOp>();
          node->right = output.top();
          output.pop();
          node->left = output.top();
          output.pop();
          node->op = operators.top();
          operators.pop();
          output.push(node);
        }
        operators.push(next);
        expectingNumber = true;
        break;
      case '(':
        // we expect there to be a binary operator here but found a left parenthesis. this occurs in terms like this: a+b(c). This should be
        // interpreted as a+b*(c)
        if (!expectingNumber) {
          operators.push('x');
        }
        operators.push(next);
        expectingNumber = true;
        break;
      case ')':
        while (operators.top() != '(') {
          // need two elements on the output stack to add a binary operator
          if (output.size() < 2) {
            motorController.RunForDuration(10);
            return;
          }
          auto node = std::make_shared<BinOp>();
          node->right = output.top();
          output.pop();
          node->left = output.top();
          output.pop();
          node->op = operators.top();
          operators.pop();
          output.push(node);
          if (operators.empty()) {
            motorController.RunForDuration(10);
            return;
          }
        }
        // discard the left parentheses
        operators.pop();
    }
  }
  while (!operators.empty()) {
    char op = operators.top();
    if (op == ')' || op == '(') {
      motorController.RunForDuration(10);
      return;
    }
    // need two elements on the output stack to add a binary operator
    if (output.size() < 2) {
      motorController.RunForDuration(10);
      return;
    }
    auto node = std::make_shared<BinOp>();
    node->right = output.top();
    output.pop();
    node->left = output.top();
    output.pop();
    node->op = op;
    operators.pop();
    output.push(node);
  }
  // perform the calculation
  errno = 0;
  double resultFloat = output.top()->calculate();
  if (errno != 0) {
    motorController.RunForDuration(10);
    return;
  }
  // make sure the result fits in a 32 bit int
  if (INT32_MAX < resultFloat || INT32_MIN > resultFloat) {
    motorController.RunForDuration(10);
    return;
  }
  // weird workaround because sprintf crashes when trying to use a float
  int32_t upper = resultFloat;
  int32_t lower = round(std::abs(resultFloat - upper) * 10000);
  // round up to the next int value
  if (lower >= 10000) {
    lower = 0;
    upper++;
  }
  // see if decimal places have to be printed
  if (lower != 0) {
    if (upper == 0 && resultFloat < 0) {
      position = sprintf(text, "-%ld.%ld", upper, lower);
    } else {
      position = sprintf(text, "%ld.%ld", upper, lower);
    }
    // remove extra zeros
    while (text[position - 1] == '0') {
      position--;
    }
  } else {
    position = sprintf(text, "%ld", upper);
  }
}

void Calculator::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == buttonMatrix) {
      const char* buttonstr = lv_btnmatrix_get_active_btn_text(obj);
      if (*buttonstr == '=') {
        eval();
      } else {
        if (position >= 30) {
          motorController.RunForDuration(10);
          return;
        }
        text[position] = *buttonstr;
        position++;
      }
    } else if (obj == returnButton) {
      if (position > 1) {

        position--;
      } else {
        position = 0;
        lv_label_set_text(result, "0");
        return;
      }
    }

    text[position] = '\0';
    lv_label_set_text(result, text);
  }
}

bool Calculator::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == Pinetime::Applications::TouchEvents::LongTap) {
    position = 0;
    lv_label_set_text(result, "0");
    return true;
  }
  if (event == Pinetime::Applications::TouchEvents::SwipeLeft) {
    lv_btnmatrix_set_map(buttonMatrix, buttonMap2);
    return true;
  }
  if (event == Pinetime::Applications::TouchEvents::SwipeRight) {
    lv_btnmatrix_set_map(buttonMatrix, buttonMap1);
    return true;
  }
  return false;
}
