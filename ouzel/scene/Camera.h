// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <memory>
#include "scene/Node.h"
#include "math/MathUtils.h"
#include "math/Rectangle.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"

namespace ouzel
{
    namespace scene
    {
        class Layer;

        class Camera: public Node
        {
            friend Layer;
        public:
            enum class Type
            {
                CUSTOM,
                ORTHOGRAPHIC,
                PERSPECTIVE
            };

            enum class ScaleMode
            {
                NONE,
                EXACT_FIT,
                NO_BORDER,
                SHOW_ALL
            };

            Camera(Matrix4 aProjection);
            Camera(const Size2& aTargetContentSize = Size2(), ScaleMode aScaleMode = ScaleMode::NONE);
            Camera(float aFov, float aNearPlane = 1.0f, float aFarPlane = 100.0f);
            virtual ~Camera();

            void setType(Type newType) { type = newType; }
            Type getType() const { return type; }

            void setFOV(float newFOV) { fov = newFOV; }
            float getFOV() const { return fov; }

            void setNearPlane(float newNearPlane) { nearPlane = newNearPlane; }
            float getNearPlane() const { return nearPlane; }

            void setFarPlane(float newFarPlane) { farPlane = newFarPlane; }
            float getFarPlane() const { return farPlane; }

            Layer* getLayer() const { return layer; }

            const Matrix4& getProjection() const { return projection; }
            void recalculateProjection();

            const Matrix4& getViewProjection() const;
            const Matrix4& getRenderViewProjection() const;
            const Matrix4& getInverseViewProjection() const;

            Vector3 convertNormalizedToWorld(const Vector2& normalizedPosition) const;
            Vector2 convertWorldToNormalized(const Vector3& worldPosition) const;

            bool checkVisibility(const Matrix4& boxTransform, const Box3& boundingBox) const;

            void setViewport(const Rectangle& newViewport);
            const Rectangle& getViewport() const { return viewport; }
            const Rectangle& getRenderViewport() const { return renderViewport; }

            void setScaleMode(ScaleMode newScaleMode);
            ScaleMode getScaleMode() const { return scaleMode; }

            void setTargetContentSize(const Size2& newTargetContentSize);
            const Size2& getTargetContentSize() const { return targetContentSize; }

            const Size2& getContentSize() const { return contentSize; }
            const Vector2& getContentScale() const { return contentScale; }
            const Vector2& getContentPosition() const { return contentPosition; }

            void setRenderTarget(const std::shared_ptr<graphics::Texture>& newRenderTarget);
            const std::shared_ptr<graphics::Texture>& getRenderTarget() const { return renderTarget; }

            bool getDepthWrite() const { return depthWrite; }
            void setDepthWrite(bool newDepthWrite) { depthWrite = newDepthWrite; }
            bool getDepthTest() const { return depthTest; }
            void setDepthTest(bool newDepthTest) { depthTest = newDepthTest; }

            bool getWireframe() const { return wireframe; }
            void setWireframe(bool newWireframe) { wireframe = newWireframe; }

            graphics::Renderer::CullMode getCullMode() const { return cullMode; }
            void setCullMode(graphics::Renderer::CullMode newCullMode) { cullMode = newCullMode; }

        protected:
            virtual void setLayer(Layer* newLayer) override;

            virtual void calculateTransform() const override;
            void calculateViewProjection() const;

            Type type;
            float fov = TAU / 6.0f;
            float nearPlane = 1.0f;
            float farPlane = 100.0f;

            Matrix4 projection;

            Rectangle viewport = Rectangle(0.0f, 0.0f, 1.0f, 1.0f);
            Rectangle renderViewport;
            Size2 targetContentSize;

            ScaleMode scaleMode = ScaleMode::NONE;
            Size2 contentSize;
            Vector2 contentScale;
            Vector2 contentPosition;

            bool depthWrite = false;
            bool depthTest = false;
            bool wireframe = false;
            graphics::Renderer::CullMode cullMode = graphics::Renderer::CullMode::NONE;

            mutable bool viewProjectionDirty = false;
            mutable Matrix4 viewProjection;
            mutable Matrix4 renderViewProjection;

            mutable bool inverseViewProjectionDirty = false;
            mutable Matrix4 inverseViewProjection;

            std::shared_ptr<graphics::Texture> renderTarget;
            Layer* addedToLayer = nullptr;
        };
    } // namespace scene
} // namespace ouzel
