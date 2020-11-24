#pragma once

#include "statehandler_underwater.h"

namespace engine::lara
{
class StateHandler_13 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_13(objects::LaraObject& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterStop}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.health < 0_hp)
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    handleDiveRotationInput();

    if(getWorld().getPresenter().getInputHandler().getInputState().jump)
    {
      setGoalAnimState(LaraStateId::UnderwaterForward);
    }

    getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - core::Gravity * 1_frame);
  }
};
} // namespace engine::lara
