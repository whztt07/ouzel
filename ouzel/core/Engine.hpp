// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include "Setup.h"
#include "core/Timer.hpp"
#include "core/Window.hpp"
#include "graphics/Renderer.hpp"
#include "audio/Audio.hpp"
#include "files/FileSystem.hpp"
#include "events/EventDispatcher.hpp"
#include "scene/SceneManager.hpp"
#include "assets/Cache.hpp"
#include "localization/Localization.hpp"
#include "network/Network.hpp"
#include "thread/Condition.hpp"
#include "thread/Mutex.hpp"
#include "thread/Thread.hpp"
#include "utils/INI.hpp"

void ouzelMain(const std::vector<std::string>& args);

namespace ouzel
{
    class Engine
    {
    public:
        Engine();
        virtual ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        void init();

        inline const std::vector<std::string>& getArgs() const { return args; }

        inline FileSystem* getFileSystem() { return &fileSystem; }
        inline EventDispatcher* getEventDispatcher() { return &eventDispatcher; }
        inline assets::Cache* getCache() { return &cache; }
        inline Window* getWindow() { return window.get(); }
        inline graphics::Renderer* getRenderer() const { return renderer.get(); }
        inline audio::Audio* getAudio() const { return audio.get(); }
        inline scene::SceneManager* getSceneManager() { return &sceneManager; }
        inline input::InputManager* getInputManager() const { return inputManager.get(); }
        inline Localization* getLocalization() { return &localization; }
        inline network::Network* getNetwork() { return &network; }

        inline const ini::Data& getDefaultSettings() const { return defaultSettings; }
        inline const ini::Data& getUserSettings() const { return userSettings; }

        virtual void run() = 0;
        void start();
        void pause();
        void resume();
        void exit();

        inline bool isPaused() const { return paused; }
        inline bool isActive() const { return active; }

        void update();

        virtual void executeOnMainThread(const std::function<void(void)>& func) = 0;

        virtual void openURL(const std::string& url);

        virtual void setScreenSaverEnabled(bool newScreenSaverEnabled);
        bool isScreenSaverEnabled() const { return screenSaverEnabled; }

    protected:
        virtual void main();

        FileSystem fileSystem;
        EventDispatcher eventDispatcher;
        std::unique_ptr<Window> window;
        std::unique_ptr<graphics::Renderer> renderer;
        std::unique_ptr<audio::Audio> audio;
        std::unique_ptr<input::InputManager> inputManager;
        Localization localization;
        assets::Cache cache;
        scene::SceneManager sceneManager;
        network::Network network;

        ini::Data defaultSettings;
        ini::Data userSettings;

#if OUZEL_MULTITHREADED
        Thread updateThread;
        Mutex updateMutex;
        Condition updateCondition;
#endif

        std::atomic<bool> active;
        std::atomic<bool> paused;

        std::atomic<bool> screenSaverEnabled;
        std::vector<std::string> args;
    };

    extern Engine* engine;
}
