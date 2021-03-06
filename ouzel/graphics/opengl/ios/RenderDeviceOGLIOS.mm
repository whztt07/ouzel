// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "core/Setup.h"

#if OUZEL_PLATFORM_IOS && OUZEL_COMPILE_OPENGL

#include "RenderDeviceOGLIOS.hpp"
#include "core/Window.hpp"
#include "core/ios/NativeWindowIOS.hpp"
#include "utils/Errors.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderDeviceOGLIOS::RenderDeviceOGLIOS():
            displayLink(*this)
        {
        }

        RenderDeviceOGLIOS::~RenderDeviceOGLIOS()
        {
            displayLink.stop();
            flushCommands();

            if (msaaColorRenderBufferId) glDeleteRenderbuffersProc(1, &msaaColorRenderBufferId);
            if (msaaFrameBufferId) glDeleteFramebuffersProc(1, &msaaFrameBufferId);
            if (resolveColorRenderBufferId) glDeleteRenderbuffersProc(1, &resolveColorRenderBufferId);
            if (depthRenderBufferId) glDeleteRenderbuffersProc(1, &depthRenderBufferId);
            if (resolveFrameBufferId) glDeleteFramebuffersProc(1, &resolveFrameBufferId);

            if (context)
            {
                [EAGLContext setCurrentContext:nil];
                [context release];
            }
        }

        void RenderDeviceOGLIOS::init(Window* newWindow,
                                      const Size2& newSize,
                                      uint32_t newSampleCount,
                                      Texture::Filter newTextureFilter,
                                      uint32_t newMaxAnisotropy,
                                      bool newVerticalSync,
                                      bool newDepth,
                                      bool newDebugRenderer)
        {
            UIView* view = static_cast<NativeWindowIOS*>(newWindow->getNativeWindow())->getNativeView();

            eaglLayer = (CAEAGLLayer*)view.layer;
            eaglLayer.opaque = YES;
            eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                            [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
            eaglLayer.contentsScale = newWindow->getNativeWindow()->getContentScale();

            context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];

            if (context)
            {
                apiMajorVersion = 3;
                apiMinorVersion = 0;
                Log(Log::Level::INFO) << "EAGL OpenGL ES 3 context created";
            }
            else
            {
                context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

                if (context)
                {
                    apiMajorVersion = 2;
                    apiMinorVersion = 0;
                    Log(Log::Level::INFO) << "EAGL OpenGL ES 2 context created";
                }
                else
                    throw SystemError("Failed to create EAGL context");
            }

            if (![EAGLContext setCurrentContext:context])
                throw SystemError("Failed to set current EAGL context");

            RenderDeviceOGL::init(newWindow,
                                  newSize,
                                  newSampleCount,
                                  newTextureFilter,
                                  newMaxAnisotropy,
                                  newVerticalSync,
                                  newDepth,
                                  newDebugRenderer);

            createFrameBuffer();

            displayLink.start(verticalSync);
        }

        void RenderDeviceOGLIOS::setSize(const Size2& newSize)
        {
            RenderDeviceOGL::setSize(newSize);

            createFrameBuffer();
        }

        void RenderDeviceOGLIOS::lockContext()
        {
            if (![EAGLContext setCurrentContext:context])
                throw SystemError("Failed to set current OpenGL context");
        }

        void RenderDeviceOGLIOS::swapBuffers()
        {
            if (sampleCount > 1)
            {
                glBindFramebufferProc(GL_DRAW_FRAMEBUFFER_APPLE, resolveFrameBufferId); // draw to resolve frame buffer
                glBindFramebufferProc(GL_READ_FRAMEBUFFER_APPLE, msaaFrameBufferId); // read from MSAA frame buffer

                GLenum error;

                if ((error = glGetError()) != GL_NO_ERROR)
                    throw SystemError("Failed to bind MSAA frame buffer, error: " + std::to_string(error));

                if (apiMajorVersion >= 3)
                    glBlitFramebufferProc(0, 0, frameBufferWidth, frameBufferHeight,
                                          0, 0, frameBufferWidth, frameBufferHeight,
                                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
                else
                    glResolveMultisampleFramebufferAPPLE();

                if ((error = glGetError()) != GL_NO_ERROR)
                    throw SystemError("Failed to blit MSAA texture, error: " + std::to_string(error));

                // reset framebuffer
                const GLenum discard[] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
                glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 1, discard);

                if ((error = glGetError()) != GL_NO_ERROR)
                    throw SystemError("Failed to discard render buffers, error: " + std::to_string(error));

                stateCache.frameBufferId = resolveFrameBufferId;
            }

            glBindRenderbufferProc(GL_RENDERBUFFER, resolveColorRenderBufferId);

            [context presentRenderbuffer:GL_RENDERBUFFER];
        }

        void RenderDeviceOGLIOS::createFrameBuffer()
        {
            if (sampleCount > 1)
            {
                // create resolve buffer with no depth buffer
                if (!resolveFrameBufferId) glGenFramebuffersProc(1, &resolveFrameBufferId);

                if (!resolveColorRenderBufferId) glGenRenderbuffersProc(1, &resolveColorRenderBufferId);

                glBindRenderbufferProc(GL_RENDERBUFFER, resolveColorRenderBufferId);
                [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

                graphics::RenderDeviceOGL::bindFrameBuffer(resolveFrameBufferId);
                glFramebufferRenderbufferProc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_RENDERBUFFER, resolveColorRenderBufferId);

                GLenum status = glCheckFramebufferStatusProc(GL_FRAMEBUFFER);

                if (status != GL_FRAMEBUFFER_COMPLETE)
                    throw SystemError("Failed to create framebuffer object, status: " + std::to_string(status));

                // create MSAA frame buffer
                if (!msaaFrameBufferId) glGenFramebuffers(1, &msaaFrameBufferId);

                if (!msaaColorRenderBufferId) glGenRenderbuffersProc(1, &msaaColorRenderBufferId);

                glBindRenderbufferProc(GL_RENDERBUFFER, msaaColorRenderBufferId);
                glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, static_cast<GLsizei>(sampleCount), GL_RGBA8_OES, frameBufferWidth, frameBufferHeight);

                if (depth)
                {
                    if (!depthRenderBufferId) glGenRenderbuffersProc(1, &depthRenderBufferId);
                    glBindRenderbufferProc(GL_RENDERBUFFER, depthRenderBufferId);
                    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, static_cast<GLsizei>(sampleCount), GL_DEPTH_COMPONENT24_OES, frameBufferWidth, frameBufferHeight);
                }

                graphics::RenderDeviceOGL::bindFrameBuffer(msaaFrameBufferId);
                glFramebufferRenderbufferProc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRenderBufferId);

                if (depth)
                    glFramebufferRenderbufferProc(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferId);

                status = glCheckFramebufferStatusProc(GL_FRAMEBUFFER);

                if (status != GL_FRAMEBUFFER_COMPLETE)
                    throw SystemError("Failed to create framebuffer object, status: " + std::to_string(status));

                frameBufferId = msaaFrameBufferId;
            }
            else
            {
                // create resolve buffer with depth buffer
                if (!resolveFrameBufferId) glGenFramebuffersProc(1, &resolveFrameBufferId);

                if (!resolveColorRenderBufferId) glGenRenderbuffersProc(1, &resolveColorRenderBufferId);

                glBindRenderbufferProc(GL_RENDERBUFFER, resolveColorRenderBufferId);
                [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

                if (depth)
                {
                    if (!depthRenderBufferId) glGenRenderbuffersProc(1, &depthRenderBufferId);
                    glBindRenderbufferProc(GL_RENDERBUFFER, depthRenderBufferId);
                    glRenderbufferStorageProc(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, frameBufferWidth, frameBufferHeight);
                }

                graphics::RenderDeviceOGL::bindFrameBuffer(resolveFrameBufferId);
                glFramebufferRenderbufferProc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_RENDERBUFFER, resolveColorRenderBufferId);

                if (depth)
                    glFramebufferRenderbufferProc(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferId);

                GLenum status = glCheckFramebufferStatusProc(GL_FRAMEBUFFER);

                if (status != GL_FRAMEBUFFER_COMPLETE)
                    throw SystemError("Failed to create framebuffer object, status: " + std::to_string(status));

                frameBufferId = resolveFrameBufferId;
            }
        }
    } // namespace graphics
} // namespace ouzel

#endif
