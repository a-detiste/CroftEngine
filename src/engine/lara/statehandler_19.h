#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_19 final : public AbstractStateHandler
{
public:
  explicit StateHandler_19(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::Climbing}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);
  }
};
} // namespace engine::lara
