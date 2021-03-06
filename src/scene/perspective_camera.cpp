#include "scene/perspective_camera.hpp"
#include "trace/logger.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

PerspectiveCamera::PerspectiveCamera(const glm::mat4& projection, const glm::vec3& position)
   : Camera(projection, position)
{
}

PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRatio, float nearClip,
                                     float farClip, const glm::vec3& position)
   : Camera(glm::perspective(fieldOfView, aspectRatio, nearClip, farClip), position)
{
}

void
PerspectiveCamera::MouseMovement(const glm::vec2& mouseMovement)
{
   m_yaw += mouseMovement.x;
   m_pitch += mouseMovement.y;

   if (m_constrainPitch)
   {
      m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
   }

   m_lookAtDirection.x = -glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
   m_lookAtDirection.y = glm::sin(glm::radians(m_pitch));
   m_lookAtDirection.z = -glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
   m_lookAtDirection = glm::normalize(m_lookAtDirection);

   m_rightVector = glm::normalize(glm::cross(m_lookAtDirection, m_worldUp));
   m_upVector = glm::normalize(glm::cross(m_rightVector, m_lookAtDirection));

   UpdateViewMatrix();
}

void
PerspectiveCamera::MoveCamera(const glm::vec2& leftRightVec)
{
   constexpr auto cameraSpeed = 0.5f;

   trace::Logger::Trace("Camera Pos:{} LookAtDir:{} RightVec:{}", m_position, m_lookAtDirection,
                        m_rightVector);
   m_position += cameraSpeed * (leftRightVec.y * m_lookAtDirection);
   m_position += cameraSpeed * (leftRightVec.x * m_rightVector);

   UpdateViewMatrix();
}

void
PerspectiveCamera::RotateCamera(float /*angle*/, const glm::vec3& /*axis*/)
{
   m_lookAtDirection = glm::normalize(m_lookAtDirection);

   m_rightVector = glm::normalize(glm::cross(m_lookAtDirection, m_worldUp));
   m_upVector = glm::normalize(glm::cross(m_rightVector, m_lookAtDirection));
   UpdateViewMatrix();
}

} // namespace shady::scene
