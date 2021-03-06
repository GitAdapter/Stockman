#ifndef IMENU_H
#define IMENU_H

#include <Misc\NonCopyable.h>
#include <Graphics\include\RenderQueue.h>
#include <Misc\GUI\Button.h>
#include <Misc\Fader.h>
#include <Misc\GUI\Slider.h>

namespace Logic
{
    class iMenuFX;
    class iMenu : public NonCopyable
    {
    public:
        enum MenuGroup
        {
            Empty,
            FirstTime,
            Intro,
            Start,
            SettingsStart,
            SettingsPause,
            Skill,
            CardSelect,
            HighscoreStartMenu,
            HighscoreGameOver,
            GameOver, 
            GameWon,
            Pause,
            Controls,
            LoadingPre,
            LoadingPost, 
            Cinematic,
            Credits, 
            CreditsGameWon
        };

        struct ButtonData
        {
            FloatRect                   screenRect;     // Where the button should be drawn
            FloatRect                   texRectNormal;  // The texture-coordinates on the button-map
            FloatRect                   texRectHover;   // The texture-coordinates on the button-map
            FloatRect                   texRectActive;  // The texture-coordinates on the button-map
            Resources::Textures::Files  texture;        // What buttonmap-texture this button is present inside
            std::function<void(void)>   callback;       // What function this button calls

            void move(DirectX::SimpleMath::Vector2 add)
            {
                screenRect.bottomRight   += add;
                screenRect.topLeft       += add;
            }
        };

        struct SliderData
        {
            std::string                 name;
            FloatRect                   screenRect;     // Where the button should be drawn
            FloatRect                   texRectNormal;  // The texture-coordinates on the button-map
            FloatRect                   texRectHover;   // The texture-coordinates on the button-map
            FloatRect                   texRectActive;  // The texture-coordinates on the button-map
            Resources::Textures::Files  texture;        // What buttonmap-texture this button is present inside
            float                       min;            // Minimum x position
            float                       max;            // Maximum x position
            float *                     value;
            float                       minValue;
            float                       maxValue;
            float                       delimeter;

            void move(DirectX::SimpleMath::Vector2 add)
            {
                screenRect.bottomRight += add;
                screenRect.topLeft += add;
            }
        };

        iMenu(MenuGroup group);
        virtual ~iMenu();

        void removeButtons();
        void removeSliders();

        virtual void fadeIn();
        virtual void fadeOut();

        void addEffect(iMenuFX* effect);
        void addBackground(Resources::Textures::Files texture, float alpha);
        void addButton(ButtonData btn);
        void addSlider(SliderData sld);
        virtual void update(int x, int y, float deltaTime);
        virtual void render() const;

        void setGroup(iMenu::MenuGroup group)   { m_group       = group;        }
        void setDrawMenu(bool shouldDraw)       { m_drawMenu    = shouldDraw;   }
        void setDrawButtons(bool shouldDraw)    { m_drawButtons = shouldDraw;   }
        void setAlpha(float alpha);

        bool getIsFading()                      { return m_isFading;        }
        bool getIsSafeToRemove()                { return m_safeToRemove;    }
        MenuGroup getMenuType()                 { return m_group;           }

    protected:
        void updateClick(int x, int y);
        void updateHover(int x, int y);

        // Fading out/in
        Fader                   m_fader;
        bool                    m_safeToRemove;
        bool                    m_isFading;
        float                   m_fadingTimer;

        // GUI effects
        iMenuFX*                m_effect;

        // Menu
        SpriteRenderInfo        m_background;
        std::vector<Button>     m_buttons;
        std::vector<Slider>     m_sliders;
        bool                    m_pressed;
        MenuGroup               m_group;
        DirectX::Mouse::Mode    m_mouseMode;

        // Hide menu
        bool                    m_drawButtons;
        bool                    m_drawSliders;
        bool                    m_drawMenu;

        Slider*                  m_sld;
    };
}

#endif // !MENU_H
