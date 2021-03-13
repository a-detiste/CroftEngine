#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine::objects
{
class Mummy final : public ModelObject
{
public:
  Mummy(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Mummy(const gsl::not_null<world::World*>& world,
        const gsl::not_null<const loader::file::Room*>& room,
        const loader::file::Item& item,
        const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

private:
  core::Angle m_headRotation{0_deg};
};
} // namespace engine::objects
