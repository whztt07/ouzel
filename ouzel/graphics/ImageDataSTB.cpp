// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include <functional>
#include <memory>
#include "ImageDataSTB.hpp"
#include "utils/Errors.hpp"
#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace ouzel
{
    namespace graphics
    {
        ImageDataSTB::ImageDataSTB(const std::vector<uint8_t>& newData,
                                   PixelFormat newPixelFormat)
        {
            int width;
            int height;
            int comp;

            int reqComp;
            switch (newPixelFormat)
            {
                case PixelFormat::R8_UNORM: reqComp = STBI_grey; break;
                case PixelFormat::RG8_UNORM: reqComp = STBI_grey_alpha; break;
                case PixelFormat::RGBA8_UNORM: reqComp = STBI_rgb_alpha; break;
                default: reqComp = STBI_default;
            }

            stbi_uc* tempData = stbi_load_from_memory(newData.data(), static_cast<int>(newData.size()), &width, &height, &comp, reqComp);

            if (!tempData)
                throw ParseError("Failed to load texture, reason: " + std::string(stbi_failure_reason()));

            if (reqComp != STBI_default) comp = reqComp;

            size_t pixelSize;
            switch (comp)
            {
                case STBI_grey: pixelFormat = PixelFormat::R8_UNORM; pixelSize = 1; break;
                case STBI_grey_alpha: pixelFormat = PixelFormat::RG8_UNORM; pixelSize = 2; break;
                case STBI_rgb_alpha: pixelFormat = PixelFormat::RGBA8_UNORM; pixelSize = 4; break;
                default:
                    stbi_image_free(tempData);
                    throw ParseError("Unknown pixel size");
            }

            data.assign(tempData, tempData + (width * height * pixelSize));

            stbi_image_free(tempData);

            size.width = static_cast<float>(width);
            size.height = static_cast<float>(height);
        }

        std::vector<uint8_t> ImageDataSTB::encode() const
        {
            int len;
            int x, y;
            int comp;
            int strideBytes;
            std::unique_ptr<unsigned char, std::function<void(unsigned char*)>> png(stbi_write_png_to_mem(const_cast<unsigned char*>(data.data()),
                                                                                                          strideBytes, x, y, comp, &len),
                                                                                    [](unsigned char* p){ STBI_FREE(p); });
            if (!png) throw SystemError("Failed to encode image");

            return std::vector<uint8_t>(png.get(), png.get() + len);
        }
    } // namespace graphics
} // namespace ouzel
