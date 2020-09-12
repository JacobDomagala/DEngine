
#include "openGL_renderer_api.hpp"

#include "trace/logger.hpp"

#include <glad/glad.h>

namespace shady::render::openGL {

void
OpenGLRendererAPI::Init()
{
   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

   glDebugMessageCallback(
      [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
         const GLchar* message, const void* logger) {
         switch (severity)
         {
            case GL_DEBUG_SEVERITY_HIGH:
               trace::Logger::Fatal(std::string(message));
               return;
            case GL_DEBUG_SEVERITY_MEDIUM:
               trace::Logger::Warn(std::string(message));
               return;
            case GL_DEBUG_SEVERITY_LOW:
               trace::Logger::Info(std::string(message));
               return;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
               trace::Logger::Debug(std::string(message));
               return;
         }
      },
      nullptr);

   glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL,
                         GL_FALSE);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
   glViewport(x, y, width, height);
}

void
OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
{
   glClearColor(color.r, color.g, color.b, color.a);
}

void
OpenGLRendererAPI::Clear()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
OpenGLRendererAPI::DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray,
                               uint32_t indexCount)
{
   uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
   glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
   glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace shady::render::openGL