#include "csm.h"

#include "camera.h"
#include "mesh.h"
#include "node.h"
#include "rendercontext.h"
#include "shadermanager.h"

#include <gl/debuggroup.h>
#include <gl/texturedepth.h>
#include <numeric>

namespace render::scene
{
void CSM::Split::init(int32_t resolution, size_t idx, ShaderManager& shaderManager)
{
  depthTexture = std::make_shared<gl::TextureDepth<float>>(glm::ivec2{resolution, resolution},
                                                           "csm-texture/" + std::to_string(idx));
  depthTexture->set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear)
    .set(gl::api::TextureCompareMode::CompareRefToTexture)
    .set(gl::api::DepthFunction::Gequal)
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToBorder)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToBorder)
    .setBorderColor(glm::vec4{1.0f});

  depthFramebuffer = gl::FrameBufferBuilder()
                       .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, depthTexture)
                       .build("csm-split-fb/" + std::to_string(idx));

  squaredTexture = std::make_shared<gl::Texture2D<gl::RG16F>>(glm::ivec2{resolution, resolution},
                                                              "csm-texture/" + std::to_string(idx) + "/squared");
  squaredTexture->set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear)
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);

  squareFramebuffer = gl::FrameBufferBuilder()
                        .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, squaredTexture)
                        .build("csm-split-fb/" + std::to_string(idx) + "/square");

  squareShader = shaderManager.getVSMSquare();
  squareMaterial = std::make_shared<Material>(squareShader);
  squareMaterial->getUniform("u_shadow")->set(depthTexture);

  squareMesh
    = createQuadFullscreen(gsl::narrow<float>(resolution), gsl::narrow<float>(resolution), squareShader->getHandle());
  squareMesh->getRenderState().setCullFace(false);
  squareMesh->getRenderState().setDepthTest(false);
  squareMesh->getRenderState().setDepthWrite(false);
  squareMesh->getMaterial().set(RenderMode::Full, squareMaterial);

  squareBlur
    = std::make_shared<SeparableBlur<gl::RG16F>>("squareBlur-" + std::to_string(idx), shaderManager, 10, true, true);
  squareBlur->setInput(squaredTexture);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void CSM::Split::renderSquare()
{
  gl::DebugGroup dbg{"square-pass"};
  GL_ASSERT(gl::api::viewport(0, 0, depthTexture->size().x, depthTexture->size().y));
  squareFramebuffer->bindWithAttachments();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  RenderContext context{RenderMode::Full, std::nullopt};

  depthTexture->set(gl::api::TextureCompareMode::None);
  squareMesh->render(context);
  depthTexture->set(gl::api::TextureCompareMode::CompareRefToTexture);
  squareBlur->render();
}

CSM::CSM(int32_t resolution, ShaderManager& shaderManager)
    : m_resolution{resolution}
{
  static_assert(CSMBuffer::NSplits > 0);
  Expects(resolution > 0);

  for(size_t i = 0; i < CSMBuffer::NSplits; ++i)
  {
    m_splits[i].init(resolution, i, shaderManager);
  }
}

std::shared_ptr<gl::Texture2D<gl::RG16F>> CSM::getTexture(size_t n) const
{
  Expects(n < CSMBuffer::NSplits);
  return m_splits.at(n).squareBlur->getBlurredTexture();
}

std::array<glm::mat4, CSMBuffer::NSplits> CSM::getMatrices(const glm::mat4& modelMatrix) const
{
  std::array<glm::mat4, CSMBuffer::NSplits> result{};
  std::transform(m_splits.begin(), m_splits.end(), result.begin(), [modelMatrix](const Split& split) {
    return split.vpMatrix * modelMatrix;
  });
  return result;
}

std::array<float, CSMBuffer::NSplits> CSM::getSplitEnds() const
{
  std::array<float, CSMBuffer::NSplits> result{};
  std::transform(m_splits.begin(), m_splits.end(), result.begin(), [](const Split& split) { return split.end; });
  return result;
}

void CSM::updateCamera(const Camera& camera)
{
  //Start off by calculating the split distances
  const float nearClip = camera.getNearPlane();
  const float farClip = camera.getFarPlane();

  std::vector<float> cascadeSplits;
  cascadeSplits.emplace_back(nearClip);
#if 0
  for(size_t i = 0; i < m_splits.size(); ++i)
  {
    static constexpr float Lambda = 0.9f;
    const auto ir = static_cast<float>(i + 1) / static_cast<float>(m_splits.size());
    const float zi
      = Lambda * nearClip * std::pow(farClip / nearClip, ir) + (1.0f - Lambda) * (nearClip + ir * (farClip - nearClip));
    cascadeSplits.emplace_back(zi);
    m_splits[i].end = -zi;
  }
#else
  for(size_t i = 0; i < m_splits.size(); ++i)
  {
    const auto ir = static_cast<float>(i + 1) / static_cast<float>(m_splits.size());
    const auto zi = nearClip + ir * (farClip - nearClip);
    cascadeSplits.emplace_back(zi);
    m_splits[i].end = -zi;
  }
#endif

  const auto position = camera.getPosition();
  const auto up = camera.getUpVector();
  const auto right = camera.getRightVector();
  const auto forward = camera.getFrontVector();

  for(size_t cascadeIterator = 0; cascadeIterator < m_splits.size(); ++cascadeIterator)
  {
    const auto nc = position + forward * cascadeSplits[cascadeIterator];
    const auto fc = position + forward * cascadeSplits[cascadeIterator + 1];

    const auto Hnear = glm::tan(camera.getFieldOfViewY() / 2) * cascadeSplits[cascadeIterator];
    const auto Wnear = Hnear * camera.getAspectRatio();
    const auto Hfar = glm::tan(camera.getFieldOfViewY() / 2) * cascadeSplits[cascadeIterator + 1];
    const auto Wfar = Hfar * camera.getAspectRatio();

    // frustum corners are in world space
    const std::array<glm::vec3, 8> frustumCorners{
      fc + up * Hfar - right * Wfar,
      fc + up * Hfar + right * Wfar,
      fc - up * Hfar - right * Wfar,
      fc - up * Hfar + right * Wfar,
      nc + up * Hnear - right * Wnear,
      nc + up * Hnear + right * Wnear,
      nc - up * Hnear - right * Wnear,
      nc - up * Hnear + right * Wnear,
    };

    // calculate frustum bbox as seen from the light
    const auto lightViewWS = glm::lookAt(glm::vec3{0.0f}, m_lightDir, m_lightDirOrtho);

    glm::vec3 bboxMin{std::numeric_limits<float>::max()};
    glm::vec3 bboxMax{std::numeric_limits<float>::lowest()};
    for(const auto& corner : frustumCorners)
    {
      const auto cornerLS = glm::vec3{lightViewWS * glm::vec4{corner, 1.0f}};
      bboxMin = glm::min(bboxMin, cornerLS);
      bboxMax = glm::max(bboxMax, cornerLS);
    }

    // extend the bboxes and snap to a grid to avoid shadow jumping
    static constexpr float SnapSize = 1024.0f;
    bboxMin = glm::floor(bboxMin / SnapSize) * SnapSize;
    bboxMax = glm::ceil(bboxMax / SnapSize) * SnapSize;

    // recalculate matrices to avoid numerical problems when zNear+zFar ~= 0
    const auto zNear = -bboxMax.z;
    static constexpr float zOffset = 100.0f;
    const auto zShift = zOffset - zNear;
    bboxMin.z -= zShift;
    bboxMax.z -= zShift;

    const auto offset = glm::normalize(m_lightDir) * zShift;
    m_splits[cascadeIterator].vpMatrix = glm::ortho(bboxMin.x, bboxMax.x, bboxMin.y, bboxMax.y, -bboxMax.z, -bboxMin.z)
                                         * glm::lookAt(glm::vec3{0.0f} - offset, m_lightDir - offset, m_lightDirOrtho);
  }
}
} // namespace render::scene
