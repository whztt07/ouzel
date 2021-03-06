// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include <algorithm>
#include "Engine.hpp"
#include "Setup.h"
#include "utils/INI.hpp"
#include "utils/Errors.hpp"
#include "utils/Log.hpp"
#include "utils/Utils.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RenderDevice.hpp"
#include "audio/Audio.hpp"
#include "thread/Lock.hpp"

#if OUZEL_PLATFORM_MACOS
#include "input/macos/InputManagerMacOS.hpp"
#elif OUZEL_PLATFORM_IOS
#include "input/ios/InputManagerIOS.hpp"
#elif OUZEL_PLATFORM_TVOS
#include "input/tvos/InputManagerTVOS.hpp"
#elif OUZEL_PLATFORM_ANDROID
#include <jni.h>
#include "input/android/InputManagerAndroid.hpp"
#elif OUZEL_PLATFORM_LINUX
#include "linux/NativeWindowLinux.hpp"
#include "input/linux/InputManagerLinux.hpp"
#elif OUZEL_PLATFORM_WINDOWS
#include "input/windows/InputManagerWin.hpp"
#elif OUZEL_PLATFORM_EMSCRIPTEN
#include "input/emscripten/InputManagerEm.hpp"
#endif

extern std::string APPLICATION_NAME;

namespace ouzel
{
    ouzel::Engine* engine = nullptr;

    Engine::Engine():
        cache(fileSystem), active(false), paused(false), screenSaverEnabled(true)
    {
        engine = this;
    }

    Engine::~Engine()
    {
        if (active)
        {
            Event event;
            event.type = Event::Type::ENGINE_STOP;
            eventDispatcher.postEvent(event);
        }

        paused = true;
        active = false;

#if OUZEL_MULTITHREADED
        if (updateThread.isJoinable())
        {
            {
                Lock lock(updateMutex);
                updateCondition.signal();
            }

            updateThread.join();
        }
#endif

        engine = nullptr;
    }

    void Engine::init()
    {
        Thread::setCurrentThreadName("Main");

        graphics::Renderer::Driver graphicsDriver = graphics::Renderer::Driver::DEFAULT;
        Size2 size;
        uint32_t sampleCount = 1; // MSAA sample count
        graphics::Texture::Filter textureFilter = graphics::Texture::Filter::POINT;
        uint32_t maxAnisotropy = 1;
        bool resizable = false;
        bool fullscreen = false;
        bool verticalSync = true;
        bool depth = false;
        bool debugRenderer = false;
        bool exclusiveFullscreen = false;
        bool highDpi = true; // should high DPI resolution be used
        audio::Audio::Driver audioDriver = audio::Audio::Driver::DEFAULT;
        bool debugAudio = false;

        defaultSettings = ini::Data(fileSystem.readFile("settings.ini"));

        try
        {
            userSettings = ini::Data(fileSystem.readFile(fileSystem.getStorageDirectory() + FileSystem::DIRECTORY_SEPARATOR + "settings.ini"));
        }
        catch (const FileError&)
        {
            Log(Log::Level::INFO) << "User settings not provided";
        }

        const ini::Section& userEngineSection = userSettings.getSection("engine");
        const ini::Section& defaultEngineSection = defaultSettings.getSection("engine");

        std::string graphicsDriverValue = userEngineSection.getValue("graphicsDriver", defaultEngineSection.getValue("graphicsDriver"));

        if (!graphicsDriverValue.empty())
        {
            if (graphicsDriverValue == "default")
                graphicsDriver = ouzel::graphics::Renderer::Driver::DEFAULT;
            else if (graphicsDriverValue == "empty")
                graphicsDriver = ouzel::graphics::Renderer::Driver::EMPTY;
            else if (graphicsDriverValue == "opengl")
                graphicsDriver = ouzel::graphics::Renderer::Driver::OPENGL;
            else if (graphicsDriverValue == "direct3d11")
                graphicsDriver = ouzel::graphics::Renderer::Driver::DIRECT3D11;
            else if (graphicsDriverValue == "metal")
                graphicsDriver = ouzel::graphics::Renderer::Driver::METAL;
            else
                throw ConfigError("Invalid graphics driver specified");
        }

        std::string widthValue = userEngineSection.getValue("width", defaultEngineSection.getValue("width"));
        if (!widthValue.empty()) size.width = std::stof(widthValue);

        std::string heightValue = userEngineSection.getValue("height", defaultEngineSection.getValue("height"));
        if (!heightValue.empty()) size.height = std::stof(heightValue);

        std::string sampleCountValue = userEngineSection.getValue("sampleCount", defaultEngineSection.getValue("sampleCount"));
        if (!sampleCountValue.empty()) sampleCount = static_cast<uint32_t>(std::stoul(sampleCountValue));

        std::string textureFilterValue = userEngineSection.getValue("textureFilter", defaultEngineSection.getValue("textureFilter"));
        if (!textureFilterValue.empty())
        {
            if (textureFilterValue == "point")
                textureFilter = ouzel::graphics::Texture::Filter::POINT;
            else if (textureFilterValue == "linear")
                textureFilter = ouzel::graphics::Texture::Filter::LINEAR;
            else if (textureFilterValue == "bilinear")
                textureFilter = ouzel::graphics::Texture::Filter::BILINEAR;
            else if (textureFilterValue == "trilinear")
                textureFilter = ouzel::graphics::Texture::Filter::TRILINEAR;
            else
                throw ConfigError("Invalid texture filter specified");
        }

        std::string maxAnisotropyValue = userEngineSection.getValue("maxAnisotropy", defaultEngineSection.getValue("maxAnisotropy"));
        if (!maxAnisotropyValue.empty()) maxAnisotropy = static_cast<uint32_t>(std::stoul(maxAnisotropyValue));

        std::string resizableValue = userEngineSection.getValue("resizable", defaultEngineSection.getValue("resizable"));
        if (!resizableValue.empty()) resizable = (resizableValue == "true" || resizableValue == "1" || resizableValue == "yes");

        std::string fullscreenValue = userEngineSection.getValue("fullscreen", defaultEngineSection.getValue("fullscreen"));
        if (!fullscreenValue.empty()) fullscreen = (fullscreenValue == "true" || fullscreenValue == "1" || fullscreenValue == "yes");

        std::string verticalSyncValue = userEngineSection.getValue("verticalSync", defaultEngineSection.getValue("verticalSync"));
        if (!verticalSyncValue.empty()) verticalSync = (verticalSyncValue == "true" || verticalSyncValue == "1" || verticalSyncValue == "yes");

        std::string exclusiveFullscreenValue = userEngineSection.getValue("exclusiveFullscreen", defaultEngineSection.getValue("exclusiveFullscreen"));
        if (!exclusiveFullscreenValue.empty()) exclusiveFullscreen = (exclusiveFullscreenValue == "true" || exclusiveFullscreenValue == "1" || exclusiveFullscreenValue == "yes");

        std::string depthValue = userEngineSection.getValue("depth", defaultEngineSection.getValue("depth"));
        if (!depthValue.empty()) depth = (depthValue == "true" || depthValue == "1" || depthValue == "yes");

        std::string debugRendererValue = userEngineSection.getValue("debugRenderer", defaultEngineSection.getValue("debugRenderer"));
        if (!debugRendererValue.empty()) debugRenderer = (debugRendererValue == "true" || debugRendererValue == "1" || debugRendererValue == "yes");

        std::string highDpiValue = userEngineSection.getValue("highDpi", defaultEngineSection.getValue("highDpi"));
        if (!highDpiValue.empty()) highDpi = (highDpiValue == "true" || highDpiValue == "1" || highDpiValue == "yes");

        std::string audioDriverValue = userEngineSection.getValue("audioDriver", defaultEngineSection.getValue("audioDriver"));

        if (!audioDriverValue.empty())
        {
            if (audioDriverValue == "default")
                audioDriver = ouzel::audio::Audio::Driver::DEFAULT;
            else if (audioDriverValue == "empty")
                audioDriver = ouzel::audio::Audio::Driver::EMPTY;
            else if (audioDriverValue == "openal")
                audioDriver = ouzel::audio::Audio::Driver::OPENAL;
            else if (audioDriverValue == "directsound")
                audioDriver = ouzel::audio::Audio::Driver::DIRECTSOUND;
            else if (audioDriverValue == "xaudio2")
                audioDriver = ouzel::audio::Audio::Driver::XAUDIO2;
            else if (audioDriverValue == "opensl")
                audioDriver = ouzel::audio::Audio::Driver::OPENSL;
            else if (audioDriverValue == "coreaudio")
                audioDriver = ouzel::audio::Audio::Driver::COREAUDIO;
            else if (audioDriverValue == "alsa")
                audioDriver = ouzel::audio::Audio::Driver::ALSA;
            else
                throw ConfigError("Invalid audio driver specified");
        }

        std::string debugAudioValue = userEngineSection.getValue("debugAudio", defaultEngineSection.getValue("debugAudio"));
        if (!debugAudioValue.empty()) debugAudio = (debugAudioValue == "true" || debugAudioValue == "1" || debugAudioValue == "yes");

        if (graphicsDriver == graphics::Renderer::Driver::DEFAULT)
        {
            auto availableDrivers = graphics::Renderer::getAvailableRenderDrivers();

            if (availableDrivers.find(graphics::Renderer::Driver::METAL) != availableDrivers.end())
                graphicsDriver = graphics::Renderer::Driver::METAL;
            else if (availableDrivers.find(graphics::Renderer::Driver::DIRECT3D11) != availableDrivers.end())
                graphicsDriver = graphics::Renderer::Driver::DIRECT3D11;
            else if (availableDrivers.find(graphics::Renderer::Driver::OPENGL) != availableDrivers.end())
                graphicsDriver = graphics::Renderer::Driver::OPENGL;
            else
                graphicsDriver = graphics::Renderer::Driver::EMPTY;
        }

        window.reset(new Window(size,
                                resizable,
                                fullscreen,
                                exclusiveFullscreen,
                                APPLICATION_NAME,
                                graphicsDriver,
                                highDpi,
                                depth));

        renderer.reset(new graphics::Renderer(graphicsDriver,
                                              window.get(),
                                              window->getResolution(),
                                              sampleCount,
                                              textureFilter,
                                              maxAnisotropy,
                                              verticalSync,
                                              depth,
                                              debugRenderer));

        if (audioDriver == audio::Audio::Driver::DEFAULT)
        {
            auto availableDrivers = audio::Audio::getAvailableAudioDrivers();

            if (availableDrivers.find(audio::Audio::Driver::COREAUDIO) != availableDrivers.end())
                audioDriver = audio::Audio::Driver::COREAUDIO;
            else if (availableDrivers.find(audio::Audio::Driver::ALSA) != availableDrivers.end())
                audioDriver = audio::Audio::Driver::ALSA;
            else if (availableDrivers.find(audio::Audio::Driver::OPENAL) != availableDrivers.end())
                audioDriver = audio::Audio::Driver::OPENAL;
            else if (availableDrivers.find(audio::Audio::Driver::XAUDIO2) != availableDrivers.end())
                audioDriver = audio::Audio::Driver::XAUDIO2;
            else if (availableDrivers.find(audio::Audio::Driver::DIRECTSOUND) != availableDrivers.end())
                audioDriver = audio::Audio::Driver::DIRECTSOUND;
            else if (availableDrivers.find(audio::Audio::Driver::OPENSL) != availableDrivers.end())
                audioDriver = audio::Audio::Driver::OPENSL;
            else
                audioDriver = audio::Audio::Driver::EMPTY;
        }

        audio.reset(new audio::Audio(audioDriver, debugAudio, window.get()));

#if OUZEL_PLATFORM_MACOS
        inputManager.reset(new input::InputManagerMacOS());
#elif OUZEL_PLATFORM_IOS
        inputManager.reset(new input::InputManagerIOS());
#elif OUZEL_PLATFORM_TVOS
        inputManager.reset(new input::InputManagerTVOS());
#elif OUZEL_PLATFORM_ANDROID
        inputManager.reset(new input::InputManagerAndroid());
#elif OUZEL_PLATFORM_LINUX
        inputManager.reset(new input::InputManagerLinux());
#elif OUZEL_PLATFORM_WINDOWS
        inputManager.reset(new input::InputManagerWin());
#elif OUZEL_PLATFORM_EMSCRIPTEN
        inputManager.reset(new input::InputManagerEm());
#else
        inputManager.reset(new input::InputManager());
#endif
    }

    void Engine::start()
    {
        if (!active)
        {
            Event event;
            event.type = Event::Type::ENGINE_START;
            eventDispatcher.postEvent(event);

            active = true;
            paused = false;

#if OUZEL_MULTITHREADED
            updateThread = Thread(std::bind(&Engine::main, this), "Game");
#else
            main();
#endif
        }
    }

    void Engine::pause()
    {
        if (active && !paused)
        {
            Event event;
            event.type = Event::Type::ENGINE_PAUSE;
            eventDispatcher.postEvent(event);

            paused = true;
        }
    }

    void Engine::resume()
    {
        if (active && paused)
        {
            Event event;
            event.type = Event::Type::ENGINE_RESUME;
            eventDispatcher.postEvent(event);

            paused = false;

#if OUZEL_MULTITHREADED
            Lock lock(updateMutex);
            updateCondition.signal();
#endif
        }
    }

    void Engine::exit()
    {
        paused = true;

        if (active)
        {
            Event event;
            event.type = Event::Type::ENGINE_STOP;
            eventDispatcher.postEvent(event);

            active = false;
        }

#if OUZEL_MULTITHREADED
        if (updateThread.isJoinable() &&
            updateThread.getId() != Thread::getCurrentThreadId())
        {
            {
                Lock lock(updateMutex);
                updateCondition.signal();
            }

            updateThread.join();
        }
#endif
    }

    void Engine::update()
    {
        eventDispatcher.dispatchEvents();

        sceneManager.update();

        if (renderer->getDevice()->getRefillQueue())
        {
            sceneManager.draw();
            renderer->getDevice()->flushCommands();
        }

        audio->update();
    }

    void Engine::main()
    {
        try
        {
            ouzelMain(args);

#if OUZEL_MULTITHREADED
            while (active)
            {
                if (!paused)
                {
                    update();

                    // TODO: implement sleep to reduce the power consumption
                }
                else
                {
                    Lock lock(updateMutex);
                    while (active && paused)
                        updateCondition.wait(lock);
                }
            }

            eventDispatcher.dispatchEvents();
#endif
        }
        catch (const std::exception& e)
        {
            Log(Log::Level::ERR) << e.what();
            exit();
        }
        catch (...)
        {
            Log(Log::Level::ERR) << "Unknown error occurred";
            exit();
        }
    }

    void Engine::openURL(const std::string&)
    {
    }

    void Engine::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        screenSaverEnabled = newScreenSaverEnabled;
    }
}
