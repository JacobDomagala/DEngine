#include "renderer_2D.hpp"
#include "camera.hpp"
#include "render_command.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"
#include "vertex.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <array>

namespace shady::render {

struct RendererData
{
   static const uint32_t MaxQuads = 20000;
   static const uint32_t MaxVertices = MaxQuads * 4;
   static const uint32_t MaxIndices = MaxQuads * 6;
   static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

   std::shared_ptr< VertexArray > QuadVertexArray;
   std::shared_ptr< VertexBuffer > QuadVertexBuffer;
   std::shared_ptr< Shader > TextureShader;
   std::shared_ptr< Texture > WhiteTexture;

   uint32_t QuadIndexCount = 0;
   QuadVertex* QuadVertexBufferBase = nullptr;
   QuadVertex* QuadVertexBufferPtr = nullptr;

   std::array< std::shared_ptr< Texture >, MaxTextureSlots > TextureSlots;
   uint32_t TextureSlotIndex = 1; // 0 = white texture
};

struct LineRendererData
{
   static const uint32_t MaxLines = 200000;
   static const uint32_t MaxVertices = MaxLines * 2;
   uint32_t NumLines = 0;

   std::shared_ptr< VertexArray > LineVertexArray;
   std::shared_ptr< VertexBuffer > LineVertexBuffer;
   std::shared_ptr< Shader > LineShader;

   LineVertex* LineVertexBufferBase = nullptr;
   LineVertex* LineVertexBufferPtr = nullptr;
};

static RendererData s_Data;
static LineRendererData s_LineData;

void
Renderer2D::Init()
{
   s_Data.QuadVertexArray = VertexArray::Create();

   s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
   s_Data.QuadVertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                       {ShaderDataType::Float4, "a_Color"},
                                       {ShaderDataType::Float2, "a_TexCoord"},
                                       {ShaderDataType::Float, "a_TexIndex"},
                                       {ShaderDataType::Float, "a_TilingFactor"}});
   s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

   s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

   uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

   uint32_t offset = 0;
   for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
   {
      quadIndices[i + 0] = offset + 0;
      quadIndices[i + 1] = offset + 1;
      quadIndices[i + 2] = offset + 2;

      quadIndices[i + 3] = offset + 0;
      quadIndices[i + 4] = offset + 2;
      quadIndices[i + 5] = offset + 3;

      offset += 4;
   }

   auto quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
   s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
   delete[] quadIndices;

   s_Data.WhiteTexture = Texture::Create(glm::ivec2{1,1});
   //s_Data.WhiteTexture->CreateColorTexture({1, 1}, {1.0f, 1.0f, 1.0f});

   int32_t samplers[s_Data.MaxTextureSlots];
   for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
   {
      samplers[i] = i;
   }

   s_Data.TextureShader = ShaderLibrary::GetShader("DefaultShader");
   s_Data.TextureShader->Bind();
   s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

   // Set all texture slots to 0
   s_Data.TextureSlots[0] = s_Data.WhiteTexture;


   s_LineData.LineVertexArray = VertexArray::Create();

   s_LineData.LineVertexBuffer = VertexBuffer::Create(s_LineData.MaxVertices * sizeof(LineVertex));
   s_LineData.LineVertexBuffer->SetLayout(
      {{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float4, "a_Color"}});
   s_LineData.LineVertexArray->AddVertexBuffer(s_LineData.LineVertexBuffer);

   s_LineData.LineVertexBufferBase = new LineVertex[s_LineData.MaxVertices];

   s_LineData.LineShader = ShaderLibrary::GetShader("LineShader");
}

void
Renderer2D::Shutdown()
{
   delete[] s_Data.QuadVertexBufferBase;
   delete[] s_LineData.LineVertexBufferBase;

   s_Data.QuadVertexArray.reset();
   s_Data.QuadVertexBuffer.reset();
   s_LineData.LineVertexArray.reset();
   s_LineData.LineVertexBuffer.reset();
   s_Data.WhiteTexture.reset();

   for (auto& texture : s_Data.TextureSlots)
   {
      texture.reset();
   }

   TextureLibrary::Clear();
   ShaderLibrary::Clear();

   s_Data.TextureShader.reset();
   s_LineData.LineShader.reset();
}

void
Renderer2D::BeginScene(const Camera& camera)
{
   s_Data.QuadIndexCount = 0;
   s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

   s_Data.TextureShader->Bind();
   s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

   s_Data.TextureSlotIndex = 1;

   s_LineData.LineShader->Bind();
   s_LineData.LineShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

   s_LineData.LineVertexBufferPtr = s_LineData.LineVertexBufferBase;
   s_LineData.NumLines = 0;
}

void
Renderer2D::EndScene()
{
   SendData(PrimitiveType::QUAD);
   SendData(PrimitiveType::LINE);
}

void
Renderer2D::SendData(PrimitiveType type)
{
   if (type == PrimitiveType::QUAD)
   {
      uint32_t dataSize =
         (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
      s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
      Flush(PrimitiveType::QUAD);
   }
   else
   {
      uint32_t dataSize = (uint32_t)((uint8_t*)s_LineData.LineVertexBufferPtr
                                     - (uint8_t*)s_LineData.LineVertexBufferBase);
      s_LineData.LineVertexBuffer->SetData(s_LineData.LineVertexBufferBase, dataSize);
      Flush(PrimitiveType::LINE);
   }
}

void
Renderer2D::Flush(PrimitiveType type)
{
   if (type == PrimitiveType::QUAD)
   {
      if (s_Data.QuadIndexCount == 0)
      {
         return; // Nothing to draw
      }

      s_Data.QuadVertexArray->Bind();
      s_Data.QuadVertexBuffer->Bind();
      s_Data.TextureShader->Bind();

      // Bind textures
      for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
      {
         s_Data.TextureSlots[i]->Bind(i);
      }

      RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
   }
   else
   {
      s_LineData.LineVertexArray->Bind();
      s_LineData.LineVertexBuffer->Bind();
      s_LineData.LineShader->Bind();

      RenderCommand::DrawLines(s_LineData.NumLines);
   }
}

void
Renderer2D::FlushAndReset(PrimitiveType type)
{
   SendData(type);

   if (type == PrimitiveType::QUAD)
   {
      s_Data.QuadIndexCount = 0;
      s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

      s_Data.TextureSlotIndex = 1;
   }
   else
   {
      s_LineData.NumLines = 0;
      s_LineData.LineVertexBufferPtr = s_LineData.LineVertexBufferBase;
   }
}

void
Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, float radiansRotation,
                     const std::shared_ptr< Texture >& texture, float tilingFactor,
                     const glm::vec4& tintColor)
{
   constexpr size_t quadVertexCount = 4;
   constexpr glm::vec2 textureCoords[] = {{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};

   if (s_Data.QuadIndexCount >= RendererData::MaxIndices)
   {
      FlushAndReset(PrimitiveType::QUAD);
   }

   float textureIndex = 0.0f;
   for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
   {
      if (*s_Data.TextureSlots[i] == *texture)
      {
         textureIndex = (float)i;
         break;
      }
   }

   if (textureIndex == 0.0f)
   {
      if (s_Data.TextureSlotIndex >= RendererData::MaxTextureSlots - 1)
      {
         FlushAndReset(PrimitiveType::QUAD);
      }

      textureIndex = (float)s_Data.TextureSlotIndex;
      s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
      s_Data.TextureSlotIndex++;
   }

   constexpr glm::vec4 positions[] = {
      glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f),
      glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(0.5f, -0.5f, 0.0f, 1.0f)};

   glm::mat4 transformMat = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f))
                            * glm::rotate(glm::mat4(1.0f), radiansRotation, {0.0f, 0.0f, 1.0f})
                            * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

   for (size_t i = 0; i < quadVertexCount; i++)
   {
      s_Data.QuadVertexBufferPtr->Position = transformMat * positions[i];
      s_Data.QuadVertexBufferPtr->Color = tintColor;
      s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
      s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
      s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
      s_Data.QuadVertexBufferPtr++;
   }

   s_Data.QuadIndexCount += 6;
}

void
Renderer2D::DrawLine(const glm::vec2& startPosition, const glm::vec2& endPosition,
                     const glm::vec4& color)
{
   constexpr size_t LineVertexCount = 2;

   if (s_LineData.NumLines == s_LineData.MaxLines)
   {
      FlushAndReset(PrimitiveType::LINE);
   }

   for (size_t i = 0; i < LineVertexCount; i++)
   {
      s_LineData.LineVertexBufferPtr->Position =
         glm::vec3(i == 0 ? startPosition : endPosition, 0.0f);
      s_LineData.LineVertexBufferPtr->Color = color;
      s_LineData.LineVertexBufferPtr++;
   }

   ++s_LineData.NumLines;
}

} // namespace shady::render