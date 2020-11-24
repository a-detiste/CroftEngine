#pragma once

#include "abstractstatehandler.h"
#include "engine/audioengine.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"

namespace engine::lara
{
class StateHandler_9 final : public AbstractStateHandler
{
public:
  explicit StateHandler_9(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::FreeFall}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.fallspeed >= core::DeadlyFallSpeedThreshold)
    {
      getLara().playSoundEffect(TR1SoundEffect::LaraScream);
    }
    dampenHorizontalSpeed(0.05f);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 192_len;
    collisionInfo.facingAngle = getMovementAngle();
    getLara().m_state.falling = true;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);
    jumpAgainstWall(collisionInfo);
    if(collisionInfo.mid.floorSpace.y > 0_len)
    {
      return;
    }

    if(applyLandingDamage())
    {
      setGoalAnimState(LaraStateId::Death);
    }
    else
    {
      setGoalAnimState(LaraStateId::Stop);
      setAnimation(AnimationId::LANDING_HARD, 358_frame);
    }
    getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraScream, &getLara().m_state);
    getLara().m_state.fallspeed = 0_spd;
    placeOnFloor(collisionInfo);
    getLara().m_state.falling = false;
  }
};
} // namespace engine::lara
