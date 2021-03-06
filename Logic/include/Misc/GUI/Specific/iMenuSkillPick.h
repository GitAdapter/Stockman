#ifndef iMENUSKILLPICK_H
#define iMENUSKILLPICK_H

#include <Misc\GUI\iMenu.h>
#include <Graphics\include\RenderInfo.h>

namespace Logic
{
    class iMenuSkillPick : public iMenu
    {
    public:
        iMenuSkillPick(iMenu::MenuGroup group);
        void update(int x, int y, float deltaTime);

        void pickOne();
        void pickTwo();
        void pickThree();

        void resetSkillPicks();
        void replaceSkill(int id);

        int getPrimarySkill();
        int getSecondarySkill();

        void render() const;

    private:
        SpriteRenderInfo    m_grapplingHook;
        SpriteRenderInfo    m_spriteRenderInfo1;
        SpriteRenderInfo    m_spriteRenderInfo2;

        int                 m_skillPoints;      //< The current number of skill picks available
        std::wstring        m_skillpointsStr;   //< The current number of skill picks available as a string, apperantly needs to be kept in memory?
        TextRenderInfo      m_textRenderInfo;   //< The current number of skill picks as a text-renderinfo to get drawn on screen
        TextRenderInfo      m_textReadyInfo;    //< The "Ready" text
        TextRenderInfo      m_textSelected_1;
        TextRenderInfo      m_textSelected_2;
        TextRenderInfo      m_textSelected_3;
        std::pair<int, int> m_selectedSkills;   //< The currently selected skills
    };
}

#endif // !iMENUSKILLPICK_H