#pragma once

#include "scene/camera.hpp"

namespace shady::scene {

class PerspectiveCamera : public Camera
{
 public:
   explicit PerspectiveCamera(const glm::mat4& projection);
   PerspectiveCamera(float fieldOfView, float aspectRatio, float nearClip, float farClip);

   ~PerspectiveCamera() override = default;

   void
   MouseMovement(const glm::vec2& mouseMovement) override;

   void
   MoveCamera(const glm::vec2& leftRightVec) override;

   void
   RotateCamera(float angle, const glm::vec3& axis) override;

 private:
   float m_yaw = -90.0f;
   float m_pitch = 0.0f;
   bool m_constrainPitch = true;
};

} // namespace shady::scene
