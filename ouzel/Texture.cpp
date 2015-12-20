//
//  Texture.cpp
//  Ouzel
//
//  Created by Elviss Strazdins on 10/03/15.
//  Copyright (c) 2015 Elviss. All rights reserved.
//

#include "Texture.h"
#include "Utils.h"

namespace ouzel
{
    Texture::Texture(const std::string& filename, Renderer* renderer)
    {
        _renderer = renderer;
        _filename = filename;
        
        std::string path = std::string("file://") + getResourcePath(filename);
        
        CFStringRef pathStringRef = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingASCII);
        
        CFURLRef url = CFURLCreateWithString(kCFAllocatorDefault, pathStringRef, nullptr);
        
        CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL(url, NULL);
        CGImageRef imageRef = CGImageSourceCreateImageAtIndex (imageSourceRef, 0, NULL);
        
        _width = (GLsizei)CGImageGetWidth(imageRef);
        _height = (GLsizei)CGImageGetHeight(imageRef);
        CGRect rect = {{0, 0}, {static_cast<CGFloat>(_width), static_cast<CGFloat>(_height)}};
        void* data = calloc(_width * 4, _height);
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGContextRef bitmapContext = CGBitmapContextCreate (data,
                                                            _width, _height, 8,
                                                            _width * 4, space,
                                                            kCGBitmapByteOrder32Host |
                                                            kCGImageAlphaPremultipliedFirst);
        CGContextSetBlendMode(bitmapContext, kCGBlendModeCopy);
        CGContextDrawImage(bitmapContext, rect, imageRef);
        CGContextRelease(bitmapContext);
        CFRelease(imageSourceRef);
        CFRelease(imageRef);
        
        CFRelease(pathStringRef);
        CFRelease(url);
        
        glGenTextures(1, &_texture);
        
        glBindTexture(GL_TEXTURE_2D, _texture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height,
                     0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenerateMipmap(GL_TEXTURE_2D);
        
        checkOpenGLErrors();
        
        free(data);
        
        printf("texture: %s (%d), width: %d, height: %d\n", filename.c_str(), _texture, _width, _height);
    }

    Texture::~Texture()
    {
        if (_texture)
        {
            glDeleteTextures(1, &_texture);
        }
    }
}
