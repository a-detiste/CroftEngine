#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Wolf final : public AIAgent
{
public:
  Wolf(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : AIAgent{world, position}
  {
  }

  Wolf(const gsl::not_null<world::World*>& world,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
    getSkeleton()->setAnim(getSkeleton()->getAnim(), 96_frame);
    getSkeleton()->updatePose();
  }

  void update() override;
};
} // namespace engine::objects
