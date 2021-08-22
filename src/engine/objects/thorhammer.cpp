#include "thorhammer.h"

#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "serialization/serialization.h"

namespace engine::objects
{
ThorHammerHandle::ThorHammerHandle(const std::string& name,
                                   const gsl::not_null<world::World*>& world,
                                   const gsl::not_null<const world::Room*>& room,
                                   const loader::file::Item& item,
                                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel}
    , m_block{world->createObject<ThorHammerBlock>(TR1ItemId::ThorHammerBlock, room, item.rotation, item.position, 0)}
{
  m_block->activate();
  m_block->m_state.triggerState = TriggerState::Active;
}

ThorHammerHandle::ThorHammerHandle(const gsl::not_null<world::World*>& world, const Location& location)
    : ModelObject{world, location}
    , m_block{world->createObject<ThorHammerBlock>(location)}
{
}

void ThorHammerHandle::update()
{
  switch(m_state.current_anim_state.get())
  {
  case 0:
    if(m_state.updateActivationTimeout())
    {
      m_state.goal_anim_state = 1_as;
    }
    else
    {
      deactivate();
      m_state.triggerState = TriggerState::Inactive;
    }
    break;
  case 1:
    if(m_state.updateActivationTimeout())
    {
      m_state.goal_anim_state = 2_as;
    }
    else
    {
      m_state.goal_anim_state = 0_as;
    }
    break;
  case 2:
    if(getSkeleton()->getLocalFrame() > 30_frame)
    {
      auto posX = m_state.location.position.X;
      auto posZ = m_state.location.position.Z;
      if(m_state.rotation.Y == 0_deg)
      {
        posZ += 3 * core::SectorSize;
      }
      else if(m_state.rotation.Y == 90_deg)
      {
        posX += 3 * core::SectorSize;
      }
      else if(m_state.rotation.Y == 180_deg)
      {
        posZ -= 3 * core::SectorSize;
      }
      else if(m_state.rotation.Y == -90_deg)
      {
        posX -= 3 * core::SectorSize;
      }
      if(auto& lara = getWorld().getObjectManager().getLara(); !lara.isDead())
      {
        if(posX - 520_len < lara.m_state.location.position.X && posX + 520_len > lara.m_state.location.position.X
           && posZ - 520_len < lara.m_state.location.position.Z && posZ + 520_len > lara.m_state.location.position.Z)
        {
          lara.m_state.health = core::DeadHealth;
          lara.getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::Lara)->animations[139],
                                      3561_frame);
          lara.setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
          lara.setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
          lara.m_state.location.position.Y = m_state.location.position.Y;
          lara.m_state.falling = false;
        }
      }
    }
    break;
  case 3:
  {
    const auto sector = m_state.location.moved({}).updateRoom();
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);

    const auto oldPosX = m_state.location.position.X;
    const auto oldPosZ = m_state.location.position.Z;
    if(m_state.rotation.Y == 0_deg)
    {
      m_state.location.position.Z += 3 * core::SectorSize;
    }
    else if(m_state.rotation.Y == 90_deg)
    {
      m_state.location.position.X += 3 * core::SectorSize;
    }
    else if(m_state.rotation.Y == 180_deg)
    {
      m_state.location.position.Z -= 3 * core::SectorSize;
    }
    else if(m_state.rotation.Y == -90_deg)
    {
      m_state.location.position.X -= 3 * core::SectorSize;
    }
    if(!getWorld().getObjectManager().getLara().isDead())
    {
      world::patchHeightsForBlock(*this, -2 * core::SectorSize);
    }
    m_state.location.position.X = oldPosX;
    m_state.location.position.Z = oldPosZ;
    deactivate();
    m_state.triggerState = TriggerState::Deactivated;
    break;
  }
  default: break;
  }
  ModelObject::update();

  // sync anim
  const auto animIdx = std::distance(&getWorld().findAnimatedModelForType(TR1ItemId::ThorHammerHandle)->animations[0],
                                     getSkeleton()->getAnim());
  m_block->getSkeleton()->replaceAnim(
    &getWorld().findAnimatedModelForType(TR1ItemId::ThorHammerBlock)->animations[animIdx],
    getSkeleton()->getLocalFrame());
  m_block->m_state.current_anim_state = m_state.current_anim_state;
}

void ThorHammerHandle::collide(CollisionInfo& info)
{
  if(!info.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  enemyPush(info, false, true);
}

void ThorHammerHandle::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser.lazy([this](const serialization::Serializer<world::World>& ser) { ser(S_NV("block", *m_block)); });
}

void ThorHammerBlock::collide(CollisionInfo& info)
{
  if(m_state.current_anim_state == 2_as)
    return;

  if(!info.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
