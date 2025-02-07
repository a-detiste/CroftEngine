#pragma once

#include "hid/inputhandler.h"
#include "statehandler_standing.h"

namespace engine::lara
{
class StateHandler_20 final : public StateHandler_Standing
{
public:
  explicit StateHandler_20(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_Standing{lara, LaraStateId::TurnFast}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(getYRotationSpeed() >= 0_deg / 1_frame)
    {
      setYRotationSpeed(core::FastTurnSpeed);
      if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
      {
        return;
      }
    }
    else
    {
      setYRotationSpeed(-core::FastTurnSpeed);
      if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
      {
        return;
      }
    }

    setGoalAnimState(LaraStateId::Stop);
  }
};
} // namespace engine::lara
