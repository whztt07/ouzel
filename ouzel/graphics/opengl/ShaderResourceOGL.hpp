// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include "core/Setup.h"

#if OUZEL_COMPILE_OPENGL

#if OUZEL_SUPPORTS_OPENGLES
#  define GL_GLEXT_PROTOTYPES 1
#  include "GLES/gl.h"
#  include "GLES2/gl2.h"
#  include "GLES2/gl2ext.h"
#  include "GLES3/gl3.h"
#else
#  define GL_GLEXT_PROTOTYPES 1
#  include "GL/glcorearb.h"
#  include "GL/glext.h"
#endif

#include "graphics/ShaderResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceOGL;

        class ShaderResourceOGL: public ShaderResource
        {
        public:
            explicit ShaderResourceOGL(RenderDeviceOGL& renderDeviceOGL);
            virtual ~ShaderResourceOGL();

            virtual void init(const std::vector<uint8_t>& newFragmentShader,
                              const std::vector<uint8_t>& newVertexShader,
                              const std::set<Vertex::Attribute::Usage>& newVertexAttributes,
                              const std::vector<Shader::ConstantInfo>& newFragmentShaderConstantInfo,
                              const std::vector<Shader::ConstantInfo>& newVertexShaderConstantInfo,
                              uint32_t newFragmentShaderDataAlignment = 0,
                              uint32_t newVertexShaderDataAlignment = 0,
                              const std::string& newFragmentShaderFunction = "",
                              const std::string& newVertexShaderFunction = "") override;

            virtual void reload() override;

            struct Location
            {
                GLint location;
                DataType dataType;
            };

            inline const std::vector<Location>& getFragmentShaderConstantLocations() const { return fragmentShaderConstantLocations; }
            inline const std::vector<Location>& getVertexShaderConstantLocations() const { return vertexShaderConstantLocations; }

            inline GLuint getProgramId() const { return programId; }

        private:
            void compileShader();
            std::string getShaderMessage(GLuint shaderId);
            std::string getProgramMessage();

            GLuint fragmentShaderId = 0;
            GLuint vertexShaderId = 0;
            GLuint programId = 0;

            std::vector<Location> fragmentShaderConstantLocations;
            std::vector<Location> vertexShaderConstantLocations;
        };
    } // namespace graphics
} // namespace ouzel

#endif
