// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include <vector>
#include "scene/Component.hpp"
#include "graphics/Renderer.hpp"
#include "math/Color.hpp"
#include "graphics/BlendState.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/Shader.hpp"

namespace ouzel
{
    namespace scene
    {
        class ShapeRenderer: public Component
        {
        public:
            static const uint32_t CLASS = Component::SHAPE_RENDERER;

            ShapeRenderer();

            virtual void draw(const Matrix4& transformMatrix,
                              float opacity,
                              const Matrix4& renderViewProjection,
                              bool wireframe) override;

            void clear();

            void line(const Vector2& start,
                      const Vector2& finish,
                      const Color& color,
                      float thickness = 0.0F);

            void circle(const Vector2& position,
                        float radius,
                        const Color& color,
                        bool fill = false,
                        uint32_t segments = 16,
                        float thickness = 0.0F);

            void rectangle(const Rect& rectangle,
                           const Color& color,
                           bool fill = false,
                           float thickness = 0.0F);

            void polygon(const std::vector<Vector2>& edges,
                         const Color& color,
                         bool fill = false,
                         float thickness = 0.0F);

            void curve(const std::vector<Vector2>& controlPoints,
                       const Color& color,
                       uint32_t segments = 16,
                       float thickness = 0.0F);

            inline const std::shared_ptr<graphics::Shader>& getShader() const { return shader; }
            inline void setShader(const std::shared_ptr<graphics::Shader>& newShader) { shader = newShader; }

            inline const std::shared_ptr<graphics::BlendState>& getBlendState() const { return blendState; }
            inline void setBlendState(const std::shared_ptr<graphics::BlendState>& newBlendState)  { blendState = newBlendState; }

        private:
            struct DrawCommand
            {
                graphics::Renderer::DrawMode mode;
                uint32_t indexCount;
                uint32_t startIndex;
            };

            std::shared_ptr<graphics::Shader> shader;
            std::shared_ptr<graphics::BlendState> blendState;
            std::shared_ptr<graphics::Buffer> indexBuffer;
            std::shared_ptr<graphics::Buffer> vertexBuffer;

            std::vector<DrawCommand> drawCommands;

            std::vector<uint16_t> indices;
            std::vector<ouzel::graphics::Vertex> vertices;
            bool dirty = false;
        };
    } // namespace scene
} // namespace ouzel
