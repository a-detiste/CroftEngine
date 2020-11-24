#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_54 final : public AbstractStateHandler
{
public:
  explicit StateHandler_54(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::Handstand}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);
  }
};
} // namespace engine::lara
