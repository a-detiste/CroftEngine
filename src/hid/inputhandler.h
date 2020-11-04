#pragma once

#include "inputstate.h"

#include <GLFW/glfw3.h>
#include <gsl-lite.hpp>

namespace hid
{
extern bool isKeyPressed(int key);
extern bool isMouseButtonPressed(int button);

class InputHandler final
{
public:
  explicit InputHandler(gsl::not_null<GLFWwindow*> window);

  void update();

  [[nodiscard]] const InputState& getInputState() const
  {
    return m_inputState;
  }

private:
  InputState m_inputState{};

  const gsl::not_null<GLFWwindow*> m_window;
  double m_lastCursorX = 0;
  double m_lastCursorY = 0;

  int m_controllerIndex = -1;
};
} // namespace hid
