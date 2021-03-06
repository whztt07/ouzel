// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "graphics/PixelFormat.hpp"
#include "math/Color.hpp"
#include "math/Size2.hpp"

namespace ouzel
{
    namespace graphics
    {
        class Renderer;
        class TextureResource;

        class Texture final
        {
        public:
            enum Flags
            {
                DYNAMIC = 0x01,
                RENDER_TARGET = 0x02,
                DEPTH_BUFFER = 0x04,
                BINDABLE_COLOR_BUFFER = 0x08,
                BINDABLE_DEPTH_BUFFER = 0x10,
            };

            enum class Filter
            {
                DEFAULT,
                POINT,
                LINEAR,
                BILINEAR,
                TRILINEAR
            };

            enum class Address
            {
                CLAMP,
                REPEAT,
                MIRROR_REPEAT
            };

            struct Level
            {
                Size2 size;
                uint32_t pitch;
                std::vector<uint8_t> data;
            };

            static const uint32_t LAYERS = 4;

            Texture(Renderer& initRenderer);
            ~Texture();

            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;

            Texture(Texture&&) = delete;
            Texture& operator=(Texture&&) = delete;

            void init(const Size2& newSize,
                      uint32_t newFlags = 0,
                      uint32_t newMipmaps = 0,
                      uint32_t newSampleCount = 1,
                      PixelFormat newPixelFormat = PixelFormat::RGBA8_UNORM);
            void init(const std::vector<uint8_t>& newData,
                      const Size2& newSize,
                      uint32_t newFlags = 0,
                      uint32_t newMipmaps = 0,
                      PixelFormat newPixelFormat = PixelFormat::RGBA8_UNORM);
            void init(const std::vector<Level>& newLevels,
                      const Size2& newSize,
                      uint32_t newFlags = 0,
                      PixelFormat newPixelFormat = PixelFormat::RGBA8_UNORM);

            inline TextureResource* getResource() const { return resource; }

            inline const Size2& getSize() const { return size; }
            void setSize(const Size2& newSize);

            void setData(const std::vector<uint8_t>& newData, const Size2& newSize);

            inline uint32_t getFlags() const { return flags; }
            inline uint32_t getMipmaps() const { return mipmaps; }

            inline Filter getFilter() const { return filter; }
            void setFilter(Filter newFilter);

            inline Address getAddressX() const { return addressX; }
            void setAddressX(Address newAddressX);

            inline Address getAddressY() const { return addressY; }
            void setAddressY(Address newAddressY);

            inline uint32_t getMaxAnisotropy() const { return maxAnisotropy; }
            void setMaxAnisotropy(uint32_t newMaxAnisotropy);

            inline uint32_t getSampleCount() const { return sampleCount; }

            inline PixelFormat getPixelFormat() const { return pixelFormat; }

            inline bool getClearColorBuffer() const { return clearColorBuffer; }
            void setClearColorBuffer(bool clear);

            inline bool getClearDepthBuffer() const { return clearDepthBuffer; }
            void setClearDepthBuffer(bool clear);

            inline Color getClearColor() const { return clearColor; }
            void setClearColor(Color color);

            inline float getClearDepth() const { return clearDepth; }
            void setClearDepth(float depth);

        private:
            Renderer& renderer;
            TextureResource* resource = nullptr;

            Size2 size;
            uint32_t flags = 0;
            uint32_t mipmaps = 0;
            bool clearColorBuffer = true;
            bool clearDepthBuffer = false;
            float clearDepth = 1.0F;
            uint32_t sampleCount = 1;
            PixelFormat pixelFormat = PixelFormat::RGBA8_UNORM;
            Color clearColor;
            Filter filter = Texture::Filter::DEFAULT;
            Address addressX = Texture::Address::CLAMP;
            Address addressY = Texture::Address::CLAMP;
            uint32_t maxAnisotropy = 0;
        };
    } // namespace graphics
} // namespace ouzel
