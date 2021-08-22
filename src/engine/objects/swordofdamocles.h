#pragma once

#include "modelobject.h"

namespace engine::objects
{
class SwordOfDamocles final : public ModelObject
{
public:
  SwordOfDamocles(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  SwordOfDamocles(const std::string& name,
                  const gsl::not_null<world::World*>& world,
                  const gsl::not_null<const world::Room*>& room,
                  const loader::file::Item& item,
                  const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{name, world, room, item, true, animatedModel}
  {
    m_state.rotation.Y += util::rand15s(180_deg) + util::rand15s(180_deg);
    m_state.fallspeed = 50_spd;
    m_rotateSpeed = util::rand15s(2048_au / 1_frame);
  }

  void update() override;
  void collide(CollisionInfo& collisionInfo) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  QS_COMBINE_UNITS(core::Angle, /, core::Frame) m_rotateSpeed;
  core::Speed m_dropSpeedX{0_spd};
  core::Speed m_dropSpeedZ{0_spd};
};
} // namespace engine::objects
