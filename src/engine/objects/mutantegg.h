#pragma once

#include "modelobject.h"

namespace engine::objects
{
class AIAgent;

class MutantEgg final : public ModelObject
{
public:
  MutantEgg(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  MutantEgg(const std::string& name,
            const gsl::not_null<world::World*>& world,
            const gsl::not_null<const world::Room*>& room,
            loader::file::Item item,
            const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  std::shared_ptr<AIAgent> m_childObject{nullptr};
};
} // namespace engine::objects
