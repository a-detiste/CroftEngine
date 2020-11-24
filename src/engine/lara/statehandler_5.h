#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_5 final : public AbstractStateHandler
{
public:
  explicit StateHandler_5(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::RunBack}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    setGoalAnimState(LaraStateId::Stop);

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(2.25_deg, -6_deg);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(2.25_deg, 6_deg);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);
    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(collisionInfo.mid.floorSpace.y > 200_len)
    {
      setAnimation(AnimationId::FREE_FALL_BACK, 1473_frame);
      setGoalAnimState(LaraStateId::FallBackward);
      getLara().m_state.fallspeed = 0_spd;
      getLara().m_state.falling = true;
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      setAnimation(AnimationId::STAY_SOLID, 185_frame);
    }
    placeOnFloor(collisionInfo);
  }
};
} // namespace engine::lara
