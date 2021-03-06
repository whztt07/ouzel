// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include "graphics/RenderTargetResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceEmpty;

        class RenderTargetResourceEmpty: public RenderTargetResource
        {
        public:
            RenderTargetResourceEmpty(RenderDeviceEmpty& renderDeviceEmpty);
        };
    } // namespace graphics
} // namespace ouzel
