#pragma once

#include "vulkan/vertex.hpp"
#include "vulkan/types.hpp"

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace shady::scene {

class Mesh
{
 public:
   Mesh(const std::string& name, std::vector< render::vulkan::Vertex >&& vertices,
        std::vector< uint32_t >&& indices, render::vulkan::TextureMaps&& textures);

   /*void
   AddTexture(const render::TexturePtr& texture);*/

   void
   Draw(const std::string& modelName, const glm::mat4& modelMat, const glm::vec4& tintColor);

 private:
   std::vector< render::vulkan::Vertex > m_vertices = {};
   std::vector< uint32_t > m_indices = {};
   // render::TexturePtrVec m_textures = {};
   render::vulkan::TextureMaps m_textures;
   std::string m_name = "dummyMeshName";
};

} // namespace shady::scene
