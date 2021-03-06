// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "RenderDeviceEmpty.hpp"
#include "BlendStateResourceEmpty.hpp"
#include "BufferResourceEmpty.hpp"
#include "RenderTargetResourceEmpty.hpp"
#include "ShaderResourceEmpty.hpp"
#include "TextureResourceEmpty.hpp"
#include "thread/Lock.hpp"
#include "assets/Cache.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderDeviceEmpty::RenderDeviceEmpty():
            RenderDevice(Renderer::Driver::EMPTY)
        {
        }

        void RenderDeviceEmpty::init(Window* newWindow,
                                     const Size2& newSize,
                                     uint32_t newSampleCount,
                                     Texture::Filter newTextureFilter,
                                     uint32_t newMaxAnisotropy,
                                     bool newVerticalSync,
                                     bool newDepth,
                                     bool newDebugRenderer)
        {
            RenderDevice::init(newWindow,
                               newSize,
                               newSampleCount,
                               newTextureFilter,
                               newMaxAnisotropy,
                               newVerticalSync,
                               newDepth,
                               newDebugRenderer);
        }

        void RenderDeviceEmpty::processCommands(CommandBuffer&)
        {
        }

        BlendStateResource* RenderDeviceEmpty::createBlendState()
        {
            Lock lock(resourceMutex);

            BlendStateResource* blendState = new BlendStateResourceEmpty(*this);
            resources.push_back(std::unique_ptr<RenderResource>(blendState));
            return blendState;
        }

        BufferResource* RenderDeviceEmpty::createBuffer()
        {
            Lock lock(resourceMutex);

            BufferResource* buffer = new BufferResourceEmpty(*this);
            resources.push_back(std::unique_ptr<RenderResource>(buffer));
            return buffer;
        }

        RenderTargetResource* RenderDeviceEmpty::createRenderTarget()
        {
            Lock lock(resourceMutex);

            RenderTargetResource* renderTarget = new RenderTargetResourceEmpty(*this);
            resources.push_back(std::unique_ptr<RenderResource>(renderTarget));
            return renderTarget;
        }

        ShaderResource* RenderDeviceEmpty::createShader()
        {
            Lock lock(resourceMutex);

            ShaderResource* shader = new ShaderResourceEmpty(*this);
            resources.push_back(std::unique_ptr<RenderResource>(shader));
            return shader;
        }

        TextureResource* RenderDeviceEmpty::createTexture()
        {
            Lock lock(resourceMutex);

            TextureResource* texture(new TextureResourceEmpty(*this));
            resources.push_back(std::unique_ptr<RenderResource>(texture));
            return texture;
        }
    } // namespace graphics
} // namespace ouzel
