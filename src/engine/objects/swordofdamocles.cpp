#include "swordofdamocles.h"

#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
void SwordOfDamocles::update()
{
  if(m_state.current_anim_state == 0_as)
  {
    m_state.goal_anim_state = 1_as;
    m_state.falling = true;
  }
  else if(m_state.current_anim_state == 1_as && m_state.touch_bits != 0)
  {
    getWorld().getObjectManager().getLara().m_state.is_hit = true;
    getWorld().getObjectManager().getLara().m_state.health -= 300_hp;
  }

  ModelObject::update();

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    deactivate();
  }
  else if(m_state.current_anim_state == 1_as && m_state.position.position.Y >= m_state.floor)
  {
    m_state.goal_anim_state = 2_as;
    m_state.position.position.Y = m_state.floor;
    m_state.fallspeed = 0_spd;
    m_state.falling = false;
  }
}

void SwordOfDamocles::collide(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
    {
      testBoneCollision(getWorld().getObjectManager().getLara());
    }
  }
  else if(m_state.triggerState != TriggerState::Invisible
          && isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius)
          && testBoneCollision(getWorld().getObjectManager().getLara()))
  {
    if(collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    {
      enemyPush(collisionInfo, false, true);
    }
  }
}
} // namespace engine::objects
