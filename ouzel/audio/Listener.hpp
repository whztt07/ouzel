// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <cfloat>
#include "audio/AudioDevice.hpp"
#include "audio/SoundOutput.hpp"
#include "scene/Component.hpp"
#include "math/Quaternion.hpp"
#include "math/Vector3.hpp"

namespace ouzel
{
    namespace audio
    {
        class Audio;

        class Listener: public SoundOutput, public scene::Component
        {
            friend Audio;
        public:
            Listener();
            virtual ~Listener();

            virtual void draw(const Matrix4& transformMatrix,
                              float opacity,
                              const Matrix4& renderViewProjection,
                              const std::shared_ptr<graphics::Texture>& renderTarget,
                              const Rectangle& renderViewport,
                              bool depthWrite,
                              bool depthTest,
                              bool wireframe,
                              bool scissorTest,
                              const Rectangle& scissorRectangle) override;

            const Vector3& getPosition() const { return position; }
            void setPosition(const Vector3& newPosition) { position = newPosition; }

            const Quaternion& getRotation() const { return rotation; }
            void setRotation(const Quaternion& newRotation) { rotation = newRotation; }

            AudioDevice::RenderCommand getRenderCommand();

        protected:
            static void setAttributes(Vector3& listenerPosition,
                                      Quaternion& listenerRotation,
                                      float& pitch,
                                      float& gain,
                                      float& rolloffScale,
                                      const Vector3& position,
                                      const Quaternion& rotation);

            Audio* audio = nullptr;

            Vector3 position;
            Quaternion rotation;
        };
    } // namespace audio
} // namespace ouzel