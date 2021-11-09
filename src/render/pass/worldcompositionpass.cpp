#include "worldcompositionpass.h"

#include "config.h"
#include "geometrypass.h"
#include "portalpass.h"
#include "render/rendersettings.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "render/scene/uniformparameter.h"

#include <algorithm>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>
#include <utility>

namespace render::scene
{
class Node;
}

namespace render::pass
{
WorldCompositionPass::WorldCompositionPass(scene::MaterialManager& materialManager,
                                           const RenderSettings& renderSettings,
                                           const glm::ivec2& viewport,
                                           const GeometryPass& geometryPass,
                                           const PortalPass& portalPass)
    : m_noWaterMaterial{materialManager.getWorldComposition(false, renderSettings.dof)}
    , m_inWaterMaterial{materialManager.getWorldComposition(true, renderSettings.dof)}
    , m_noWaterMesh{scene::createScreenQuad(m_noWaterMaterial, "composition-nowater")}
    , m_inWaterMesh{scene::createScreenQuad(m_inWaterMaterial, "composition-water")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGB8>>(viewport, "composition-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        m_colorBuffer,
        gslu::make_nn_unique<gl::Sampler>("composition-color-sampler")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::Repeat)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::Repeat)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .build("composition-fb")}
{
  m_noWaterMesh->bind("u_portalPosition",
                      [buffer = portalPass.getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });
  if(renderSettings.waterDenoise)
    m_noWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass.getBlurredTexture()](const render::scene::Node* /*node*/,
                                                                   const render::scene::Mesh& /*mesh*/,
                                                                   gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  else
    m_noWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass.getNoisyTexture()](const render::scene::Node* /*node*/,
                                                                 const render::scene::Mesh& /*mesh*/,
                                                                 gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  m_noWaterMesh->bind("u_geometryPosition",
                      [buffer = geometryPass.getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });
  m_noWaterMesh->bind("u_texture",
                      [buffer = geometryPass.getColorBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });

  m_noWaterMesh->getRenderState().merge(m_fb->getRenderState());

  m_inWaterMesh->bind("u_portalPosition",
                      [buffer = portalPass.getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });

  if(renderSettings.waterDenoise)
    m_inWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass.getBlurredTexture()](const render::scene::Node* /*node*/,
                                                                   const render::scene::Mesh& /*mesh*/,
                                                                   gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  else
    m_inWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass.getNoisyTexture()](const render::scene::Node* /*node*/,
                                                                 const render::scene::Mesh& /*mesh*/,
                                                                 gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  m_inWaterMesh->bind("u_geometryPosition",
                      [buffer = geometryPass.getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });
  m_inWaterMesh->bind("u_texture",
                      [buffer = geometryPass.getColorBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });

  m_inWaterMesh->getRenderState().merge(m_fb->getRenderState());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void WorldCompositionPass::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_noWaterMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
  m_inWaterMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void WorldCompositionPass::render(bool inWater)
{
  SOGLB_DEBUGGROUP("world-composition-pass");
  m_fb->bind();

  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  if(inWater)
    m_inWaterMesh->render(nullptr, context);
  else
    m_noWaterMesh->render(nullptr, context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
