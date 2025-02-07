#include "underwaterswitch.h"

#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/units.h"
#include "engine/objectmanager.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "object.h"
#include "objectstate.h"
#include "qs/quantity.h"

namespace engine::objects
{
void UnderwaterSwitch::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
    return;

  if(m_state.triggerState != TriggerState::Inactive)
    return;

  auto& lara = getWorld().getObjectManager().getLara();
  if(!lara.isDiving())
    return;

  if(lara.getCurrentAnimState() != loader::file::LaraStateId::UnderwaterStop)
    return;

  static const InteractionLimits limits{
    core::BoundingBox{{-1024_len, -1024_len, -1024_len}, {1024_len, 1024_len, 512_len}},
    {-80_deg, -80_deg, -80_deg},
    {+80_deg, +80_deg, +80_deg}};

  if(!limits.canInteract(m_state, lara.m_state))
    return;

  if(m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as)
    return;

  static const core::GenericVec<core::Speed> alignSpeed{0_spd, 0_spd, 108_spd};
  if(!lara.alignTransform(alignSpeed, *this))
    return;

  lara.m_state.fallspeed = 0_spd;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
  do
  {
    lara.setGoalAnimState(loader::file::LaraStateId::SwitchDown);
    lara.advanceFrame();
  } while(lara.getCurrentAnimState() != loader::file::LaraStateId::SwitchDown);
  lara.setGoalAnimState(loader::file::LaraStateId::UnderwaterStop);
  lara.setHandStatus(HandStatus::Grabbing);
  m_state.triggerState = TriggerState::Active;

  if(m_state.current_anim_state == 1_as)
  {
    m_state.goal_anim_state = 0_as;
  }
  else
  {
    m_state.goal_anim_state = 1_as;
  }

  activate();
  ModelObject::update(); // NOLINT(bugprone-parent-virtual-call)
}
} // namespace engine::objects
