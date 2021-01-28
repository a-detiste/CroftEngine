#pragma once

#include "serialization/serialization_fwd.h"

#include <gsl-lite.hpp>
#include <memory>

namespace loader::file
{
struct Item;
} // namespace loader::file

namespace engine
{
class World;
}

namespace engine::objects
{
class Object;
extern std::shared_ptr<Object> createObject(World& world, loader::file::Item& item);
extern gsl::not_null<std::shared_ptr<Object>>
  create(const serialization::TypeId<gsl::not_null<std::shared_ptr<Object>>>&,
         const serialization::Serializer<World>& ser);
} // namespace engine::objects

namespace serialization
{
using engine::objects::create;
extern void serialize(std::shared_ptr<engine::objects::Object>& ptr, const Serializer<engine::World>& ser);
} // namespace serialization
