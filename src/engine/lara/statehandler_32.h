#pragma once

#include "abstractstatehandler.h"
#include "engine/audioengine.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_32 final : public AbstractStateHandler
{
public:
  explicit StateHandler_32(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::SlideBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraSliding, &getLara().m_state);
      setGoalAnimState(LaraStateId::JumpBack);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
    commonSlideHandling(collisionInfo);
  }
};
} // namespace engine::lara
