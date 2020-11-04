#pragma once

#include "core/vec.h"
#include "loader/file/datatypes.h"
#include "util/helpers.h"

namespace engine
{
struct Lighting
{
  struct alignas(16) Light
  {
    glm::vec3 position = glm::vec3{std::numeric_limits<float>::quiet_NaN()};
    float brightness = 0;
    float fadeDistance = 0;

    bool operator==(const Light& rhs) const
    {
      return position == rhs.position && brightness == rhs.brightness && fadeDistance == rhs.fadeDistance;
    }

    bool operator!=(const Light& rhs) const
    {
      return !(*this == rhs);
    }
  };
  static_assert(sizeof(Light) == 32, "Invalid Light struct size");

  core::Brightness ambient{};
  std::vector<Light> lights;
  std::vector<Light> bufferLights;

  gl::ShaderStorageBuffer<Light> m_buffer{"lights-buffer"};

  void updateDynamic(const core::Shade& shade,
                     const core::RoomBoundPosition& pos,
                     const std::vector<loader::file::Room>& rooms)
  {
    if(shade.get() >= 0)
    {
      updateStatic(shade);
      return;
    }

    ambient = toBrightness(pos.room->ambientShade);

    lights.clear();
    if(pos.room->lights.empty())
    {
      m_buffer.setData(lights, gl::api::BufferUsageARB::StreamDraw);
      return;
    }

    std::vector<gsl::not_null<const loader::file::Room*>> testRooms;
    testRooms.emplace_back(pos.room);
    std::transform(pos.room->portals.begin(),
                   pos.room->portals.end(),
                   std::back_inserter(testRooms),
                   [&rooms](const auto& portal) { return &rooms.at(portal.adjoining_room.get()); });

    for(const auto& room : testRooms)
    {
      // http://www-f9.ijs.si/~matevz/docs/PovRay/pov274.htm
      // 1 / ( 1 + (d/fade_distance) ^ fade_power );
      // assuming fade_power = 1, multiply numerator and denominator with fade_distance (identity transform):
      // fade_distance / ( fade_distance + d )
      std::transform(
        room->lights.begin(), room->lights.end(), std::back_inserter(lights), [](const loader::file::Light& light) {
          return Light{light.position.toRenderSystem(), light.getBrightness(), light.fadeDistance.get<float>()};
        });
    }

    if(bufferLights != lights)
      m_buffer.setData(lights, gl::api::BufferUsageARB::StreamDraw);
    bufferLights = lights;
  }

  void updateStatic(const core::Shade& shade)
  {
    lights.clear();
    ambient = toBrightness(shade);
    m_buffer.setData(lights, gl::api::BufferUsageARB::StaticDraw);
  }

  void bind(render::scene::Node& node) const
  {
    node.addUniformSetter("u_lightAmbient", [this](const render::scene::Node& /*node*/, gl::Uniform& uniform) {
      uniform.set(ambient.get());
    });

    node.addBufferBinder("b_lights", [this](const render::scene::Node&, gl::ShaderStorageBlock& shaderStorageBlock) {
      shaderStorageBlock.bind(m_buffer);
    });
  }
};
} // namespace engine
