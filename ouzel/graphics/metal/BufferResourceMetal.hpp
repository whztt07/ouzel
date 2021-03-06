// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include "core/Setup.h"

#if OUZEL_COMPILE_METAL

#if defined(__OBJC__)
#import <Metal/Metal.h>
typedef id<MTLBuffer> MTLBufferPtr;
#else
#include <objc/objc.h>
typedef id MTLBufferPtr;
#endif

#include "graphics/BufferResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceMetal;

        class BufferResourceMetal: public BufferResource
        {
        public:
            explicit BufferResourceMetal(RenderDeviceMetal& renderDeviceMetal);
            virtual ~BufferResourceMetal();

            virtual void init(Buffer::Usage newUsage, uint32_t newFlags,
                              const std::vector<uint8_t>& newData,
                              uint32_t newSize) override;

            virtual void setData(const std::vector<uint8_t>& newData) override;

            inline MTLBufferPtr getBuffer() const { return buffer; }

        private:
            void createBuffer(NSUInteger newSize);

            MTLBufferPtr buffer = nil;
            NSUInteger bufferSize = 0;
        };
    } // namespace graphics
} // namespace ouzel

#endif
