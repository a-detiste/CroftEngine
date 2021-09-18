#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <memory>

// IWYU pragma: no_forward_declare gl::Framebuffer
// IWYU pragma: no_forward_declare gl::Texture2D
// IWYU pragma: no_forward_declare gl::TextureDepth
// IWYU pragma: no_forward_declare gl::TextureHandle

namespace render::pass
{
class GeometryPass
{
public:
  explicit GeometryPass(const glm::ivec2& viewport);
  ~GeometryPass();
  void bind(const glm::ivec2& size);

  [[nodiscard]] const auto& getNormalBuffer() const
  {
    return m_normalBufferHandle;
  }

  [[nodiscard]] const auto& getPositionBuffer() const
  {
    return m_positionBufferHandle;
  }

  [[nodiscard]] const auto& getInterpolatedPositionBuffer() const
  {
    return m_interpolatedPositionBufferHandle;
  }

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

private:
  std::shared_ptr<gl::TextureDepth<float>> m_depthBuffer;
  std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_positionBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_positionBufferHandle;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_interpolatedPositionBufferHandle;
  std::shared_ptr<gl::Texture2D<gl::RGB16F>> m_normalBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>> m_normalBufferHandle;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
