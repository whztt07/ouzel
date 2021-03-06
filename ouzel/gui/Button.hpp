// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include <functional>
#include "gui/Widget.hpp"
#include "scene/Sprite.hpp"
#include "scene/TextRenderer.hpp"
#include "events/Event.hpp"
#include "events/EventHandler.hpp"
#include "math/Color.hpp"

namespace ouzel
{
    namespace gui
    {
        class Button: public Widget
        {
        public:
            Button();
            Button(const std::string& normalImage,
                   const std::string& selectedImage,
                   const std::string& pressedImage,
                   const std::string& disabledImage,
                   const std::string& label = "",
                   const std::string& font = "",
                   float fontSize = 1.0F,
                   const Color& initLabelColor = Color::WHITE,
                   const Color& initLabelSelectedColor = Color::WHITE,
                   const Color& initLabelPressedColor = Color::WHITE,
                   const Color& initLabelDisabledColor = Color::WHITE);

            virtual void setEnabled(bool newEnabled) override;

            inline scene::Sprite* getNormalSprite() const { return normalSprite.get(); }
            inline scene::Sprite* getSelectedSprite() const { return selectedSprite.get(); }
            inline scene::Sprite* getPressedSprite() const { return pressedSprite.get(); }
            inline scene::Sprite* getDisabledSprite() const { return disabledSprite.get(); }
            inline scene::TextRenderer* getLabelDrawable() const { return labelDrawable.get(); }

        protected:
            virtual void setSelected(bool newSelected) override;

            bool handleUI(Event::Type type, const UIEvent& event);

            void updateSprite();

            std::unique_ptr<scene::Sprite> normalSprite;
            std::unique_ptr<scene::Sprite> selectedSprite;
            std::unique_ptr<scene::Sprite> pressedSprite;
            std::unique_ptr<scene::Sprite> disabledSprite;
            std::unique_ptr<scene::TextRenderer> labelDrawable;

            EventHandler eventHandler;

            bool pointerOver = false;
            bool pressed = false;

            Color labelColor = Color::WHITE;
            Color labelSelectedColor = Color::WHITE;
            Color labelPressedColor = Color::WHITE;
            Color labelDisabledColor = Color::WHITE;
        };
    } // namespace gui
} // namespace ouzel
