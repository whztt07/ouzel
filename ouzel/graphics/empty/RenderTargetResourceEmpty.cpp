// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "RenderTargetResourceEmpty.hpp"
#include "RenderDeviceEmpty.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderTargetResourceEmpty::RenderTargetResourceEmpty(RenderDeviceEmpty& renderDeviceEmpty):
            RenderTargetResource(renderDeviceEmpty)
        {
        }
    } // namespace graphics
} // namespace ouzel
