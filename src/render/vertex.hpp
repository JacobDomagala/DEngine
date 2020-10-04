#pragma once

#include <glm/glm.hpp>

namespace shady::render{

struct Vertex
{
   glm::vec3 m_position;
   glm::vec3 m_normal;
   glm::vec2 m_texCoords;
   glm::vec3 m_tangent;
};

struct QuadVertex
{
   glm::vec3 Position;
   glm::vec4 Color;
   glm::vec2 TexCoord;
   float TexIndex;
   float TilingFactor;
};

struct LineVertex
{
   glm::vec3 Position;
   glm::vec4 Color;
};

}