// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include "math/Vector2.hpp"
#include "events/EventHandler.hpp"

namespace ouzel
{
    namespace scene
    {
        class SceneManager;
        class Layer;

        class Scene
        {
            friend SceneManager;
        public:
            Scene();
            virtual ~Scene();

            Scene(const Scene&) = delete;
            Scene& operator=(const Scene&) = delete;

            Scene(Scene&&) = delete;
            Scene& operator=(Scene&&) = delete;

            virtual void draw();

            virtual void addLayer(Layer* layer);

            template<typename T> void addLayer(const std::unique_ptr<T>& layer)
            {
                addLayer(layer.get());
            }

            template<typename T> void addLayer(std::unique_ptr<T>&& layer)
            {
                addLayer(layer.get());
                ownedLayers.push_back(std::move(layer));
            }

            virtual bool removeLayer(Layer* layer);

            template<typename T> bool removeLayer(const std::unique_ptr<T>& layer)
            {
                return removeLayer(layer.get());
            }

            void removeAllLayers();
            bool hasLayer(Layer* layer) const;
            inline const std::vector<Layer*>& getLayers() const { return layers; }

            virtual void recalculateProjection();

            std::pair<Actor*, ouzel::Vector3> pickActor(const Vector2& position, bool renderTargets = false) const;
            std::vector<std::pair<Actor*, ouzel::Vector3>> pickActors(const Vector2& position, bool renderTargets = false) const;
            std::vector<Actor*> pickActors(const std::vector<Vector2>& edges, bool renderTargets = false) const;

        protected:
            virtual void enter();
            virtual void leave();

            bool handleWindow(Event::Type type, const WindowEvent& event);
            bool handleMouse(Event::Type type, const MouseEvent& event);
            bool handleTouch(Event::Type type, const TouchEvent& event);

            void pointerEnterActor(uint64_t pointerId, Actor* actor, const Vector2& position);
            void pointerLeaveActor(uint64_t pointerId, Actor* actor, const Vector2& position);
            void pointerDownOnActor(uint64_t pointerId, Actor* actor, const Vector2& position, const Vector3& localPosition);
            void pointerUpOnActor(uint64_t pointerId, Actor* actor, const Vector2& position);
            void pointerDragActor(uint64_t pointerId, Actor* actor, const Vector2& position,
                                 const Vector2& difference, const ouzel::Vector3& localPosition);

            SceneManager* sceneManger = nullptr;

            std::vector<Layer*> layers;
            std::vector<std::unique_ptr<Layer>> ownedLayers;
            ouzel::EventHandler eventHandler;

            std::unordered_map<uint64_t, std::pair<Actor*, ouzel::Vector3>> pointerDownOnActors;

            bool entered = false;
        };
    } // namespace scene
} // namespace ouzel
