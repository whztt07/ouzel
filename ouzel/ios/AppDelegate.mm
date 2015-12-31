// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#import "AppDelegate.h"
#import "ViewController.h"
#import "OpenGLView.h"
#include "Engine.h"

@interface AppDelegate ()
{
    ouzel::Engine* _engine;
}
@end

@implementation AppDelegate

-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    _engine = new ouzel::Engine();
    
    _window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    
    UIViewController* viewController = [[ViewController alloc] init];
    _window.rootViewController = viewController;
    
    OpenGLView* view = [[OpenGLView alloc] init];
    viewController.view = view;
    
    [_window makeKeyAndVisible];
    
    return YES;
}

-(void)applicationWillResignActive:(UIApplication *)application
{
    
}

-(void)applicationDidEnterBackground:(UIApplication *)application
{
    
}

-(void)applicationWillEnterForeground:(UIApplication *)application
{
    
}

-(void)applicationDidBecomeActive:(UIApplication *)application
{
    
}

-(void)applicationWillTerminate:(UIApplication *)application
{
    delete _engine;
}

-(void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    //TODO: free resources
}

@end
