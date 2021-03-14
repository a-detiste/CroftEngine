#pragma once

#include "modelobject.h"

namespace engine::objects
{
class TrapDoorDown final : public ModelObject
{
public:
  TrapDoorDown(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  TrapDoorDown(const gsl::not_null<world::World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override
  {
    if(m_state.updateActivationTimeout())
    {
      if(m_state.current_anim_state == 0_as)
        m_state.goal_anim_state = 1_as;
    }
    else if(m_state.current_anim_state == 1_as)
    {
      m_state.goal_anim_state = 0_as;
    }

    ModelObject::update();
  }

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(m_state.current_anim_state != 0_as || !possiblyOnTrapdoor(pos) || pos.Y > m_state.position.position.Y
       || y <= m_state.position.position.Y)
      return;

    y = m_state.position.position.Y;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(m_state.current_anim_state != 0_as || !possiblyOnTrapdoor(pos) || pos.Y <= m_state.position.position.Y
       || y > m_state.position.position.Y)
      return;

    y = m_state.position.position.Y + core::QuarterSectorSize;
  }

private:
  bool possiblyOnTrapdoor(const core::TRVec& pos) const
  {
    const auto trapdoorSectorX = m_state.position.position.X / core::SectorSize;
    const auto trapdoorSectorZ = m_state.position.position.Z / core::SectorSize;
    const auto posSectorX = pos.X / core::SectorSize;
    const auto posSectorZ = pos.Z / core::SectorSize;
    auto trapdoorAxis = axisFromAngle(m_state.rotation.Y, 1_au);
    BOOST_ASSERT(trapdoorAxis.has_value());

    if(*trapdoorAxis == core::Axis::PosZ && trapdoorSectorX == posSectorX
       && (trapdoorSectorZ + 1 == posSectorZ || trapdoorSectorZ == posSectorZ))
      return true;
    if(*trapdoorAxis == core::Axis::NegZ && trapdoorSectorX == posSectorX
       && (trapdoorSectorZ - 1 == posSectorZ || trapdoorSectorZ == posSectorZ))
      return true;
    if(*trapdoorAxis == core::Axis::PosX && trapdoorSectorZ == posSectorZ
       && (trapdoorSectorX + 1 == posSectorX || trapdoorSectorX == posSectorX))
      return true;
    if(*trapdoorAxis == core::Axis::NegX && trapdoorSectorZ == posSectorZ
       && (trapdoorSectorX - 1 == posSectorX || trapdoorSectorX == posSectorX))
      return true;

    return false;
  }
};
} // namespace engine::objects
