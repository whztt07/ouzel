// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "NativeWindowTVOS.hpp"
#include "ViewTVOS.h"
#include "graphics/RenderDevice.hpp"
#include "graphics/opengl/tvos/OpenGLView.h"
#include "graphics/metal/tvos/MetalView.h"
#include "core/Engine.hpp"
#include "thread/Lock.hpp"
#include "utils/Errors.hpp"

@interface ViewController: UIViewController
{
    ouzel::NativeWindowTVOS* window;
}

@end

@implementation ViewController

-(id)initWithWindow:(ouzel::NativeWindowTVOS*)initWindow
{
    if (self = [super init])
        window = initWindow;

    return self;
}

-(void)textFieldDidChange:(__unused id)sender
{
}

-(void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];

    window->handleResize(ouzel::Size2(static_cast<float>(size.width),
                                      static_cast<float>(size.height)));
}

@end

namespace ouzel
{
    NativeWindowTVOS::NativeWindowTVOS(const std::string& newTitle,
                                       graphics::Renderer::Driver graphicsDriver,
                                       bool newHighDpi):
        ouzel::NativeWindow(Size2(),
                            true,
                            true,
                            true,
                            newTitle,
                            newHighDpi)
    {
        screen = [UIScreen mainScreen];

        window = [[UIWindow alloc] initWithFrame:[screen bounds]];

        viewController = [[[ViewController alloc] initWithWindow:this] autorelease];
        window.rootViewController = viewController;

        CGRect windowFrame = [window bounds];

        size = Size2(static_cast<float>(windowFrame.size.width),
                     static_cast<float>(windowFrame.size.height));

        switch (graphicsDriver)
        {
            case graphics::Renderer::Driver::EMPTY:
                view = [[ViewTVOS alloc] initWithFrame:windowFrame];
                break;
#if OUZEL_COMPILE_OPENGL
            case graphics::Renderer::Driver::OPENGL:
                view = [[OpenGLView alloc] initWithFrame:windowFrame];
                break;
#endif
#if OUZEL_COMPILE_METAL
            case graphics::Renderer::Driver::METAL:
                view = [[MetalView alloc] initWithFrame:windowFrame];
                break;
#endif
            default:
                throw SystemError("Unsupported render driver");
        }

        textField = [[UITextField alloc] init];
        textField.hidden = YES;
        textField.keyboardType = UIKeyboardTypeDefault;
        [textField addTarget:viewController action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];
        [view addSubview:textField];

        viewController.view = view;

        [window makeKeyAndVisible];

        if (highDpi)
        {
            contentScale = static_cast<float>(screen.scale);
            resolution = size * contentScale;
        }
        else
            resolution = size;
    }

    NativeWindowTVOS::~NativeWindowTVOS()
    {
        if (textField) [textField release];
        if (viewController) [viewController release];
        if (view) [view release];
        if (window) [window release];
    }

    void NativeWindowTVOS::handleResize(const Size2& newSize)
    {
        size = newSize;
        resolution = size * contentScale;

        Lock lock(listenerMutex);
        if (listener)
        {
            listener->onSizeChange(size);
            listener->onResolutionChange(resolution);
        }
    }
}
