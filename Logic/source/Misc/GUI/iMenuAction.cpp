#include <Misc\GUI\iMenuAction.h>
#include <StateMachine\StateBuffer.h>
#include <StateMachine\StatePrimary.h>
#include <StateMachine\StateSecondary.h>
#include <StatePlaying.h>
#include <Misc\GUI\iMenuMachine.h>
#include <Misc\GUI\Specific\iMenuSkillPick.h>
#include <Engine\Typing.h>
#include <Engine\Settings.h>
#include <State.h>
#include <Misc\Sound\NoiseMachine.h>
#include <Engine/Engine.h>

using namespace Logic;

// Switches both main-states to in-game, will unload & load everything
void ButtonFunction::startGame()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::LoadingPre);

}

void ButtonFunction::startGameHeroic()
{
    if (Action::Get().m_menuMachine)
    {
        Action::Get().m_heroic = true;
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::LoadingPre);
    }
}

// Switches the current menu-machine to settings screen
void ButtonFunction::startSettings()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::SettingsStart);
}

// Switches the current menu-machine to settings screen
void ButtonFunction::startSettingsPause()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::SettingsPause);
}

// Switches the current menu-machine to start screen
void ButtonFunction::startMainMenu()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Start);
}

// Switches the current menu-machine to highscore screen
void ButtonFunction::showHighscore()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::HighscoreStartMenu);
}

void ButtonFunction::showCredits()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Credits);
}

// Unloads all program data and quits the game
void ButtonFunction::quitGame()
{
    Sound::NoiseMachine::Get().playSFX(Sound::SFX::HELLO, nullptr, true);

    if (Action::Get().m_stateBuffer->currentPrimaryState)
        if (StatePrimary* primary = dynamic_cast<StatePrimary*>(Action::Get().m_stateBuffer->currentPrimaryState))
            primary->setQuit();
}

// Just simply removes the current pause menu
void ButtonFunction::unpause()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Empty);
}

// Just simply removes the current pause menu
void ButtonFunction::pause()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Pause);
}

// Goes to the gameoverhighscore screen
void ButtonFunction::goToGameOverHighscore()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::HighscoreGameOver);
}

// Goes back to gameover state
void ButtonFunction::goToGameOver()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::GameOver);
}

// Goes back to gamewon state
void ButtonFunction::gotoGameWon()
{
    if (Action::Get().m_menuMachine)
        Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::GameWon);
}

// Switches the program's state and switching to startmenu
void ButtonFunction::goBackToMainMenu()
{
    // Changes the main state
    if (Action::Get().m_stateBuffer->currentPrimaryState)
        if (StatePrimary* primary = dynamic_cast<StatePrimary*>(Action::Get().m_stateBuffer->currentPrimaryState))
            primary->queueState(StateType::State_Start);

    // Disables the secondary state (Not like it's being used, but whatever)
    if (Action::Get().m_stateBuffer->currentSecondaryState)
        if (StateSecondary* secondary = dynamic_cast<StateSecondary*>(Action::Get().m_stateBuffer->currentSecondaryState))
            secondary->queueState(StateType::Nothing);

    Action::Get().m_heroic = false;
}

void ButtonFunction::playAgain()
{
    if (StatePrimary* primary = dynamic_cast<StatePrimary*>(Action::Get().m_stateBuffer->currentPrimaryState))
    {
        if (StatePlaying* playing = dynamic_cast<StatePlaying*>(primary->getCurrentState()))
        {
            // Adding a "fake" static loading screen. Hides current non-pressable buttons.
            Sound::NoiseMachine::Get().playSFX(Sound::SFX::START_GAME, nullptr, true);
            Action::Get().m_menuMachine->getActiveMenu()->addBackground(Resources::Textures::Loadingscreen, 1.f);
            Action::Get().m_menuMachine->getActiveMenu()->setDrawButtons(false);

            playing->reset();

            Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Skill);
        }
    }
}

void ButtonFunction::writing()
{
    Typing* theChar = Typing::getInstance(); //might need to be deleted
    char trashThis = theChar->getSymbol();
}


void ButtonFunction::chooseUpgrade1()
{
    chooseUpgrade(0);
}

void ButtonFunction::chooseUpgrade2()
{
    chooseUpgrade(1);
}

void ButtonFunction::chooseUpgrade3()
{
    chooseUpgrade(2);
}

void Logic::chooseUpgrade(int index)
{
    if (StatePrimary* primary = dynamic_cast<StatePrimary*>(Action::Get().m_stateBuffer->currentPrimaryState))
    {
        if (StatePlaying* playing = dynamic_cast<StatePlaying*>(primary->getCurrentState()))
        {
            if (playing->getCardManager()->pickAndApplyCard(*playing->getPlayer(), index))
            {
                Sound::NoiseMachine::Get().playSFX(Sound::SFX::WAVE_CARD, nullptr, true);
                Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Empty);
                

                RECT rect;
                GetWindowRect(*Engine::g_window, &rect);

                DirectX::SimpleMath::Vector2 vec = DirectX::SimpleMath::Vector2((rect.left + rect.right* 0.5f), (rect.top + rect.bottom* 0.5f));
                SetCursorPos(vec.x, vec.y);
            }
        }
    }
}

// Just ignore how ugly this is
void ButtonFunction::confirmSkillPicks()
{
    if (StatePrimary* primary = dynamic_cast<StatePrimary*>(Action::Get().m_stateBuffer->currentPrimaryState))
    {
        if (StatePlaying* playing = dynamic_cast<StatePlaying*>(primary->getCurrentState()))
        {
            if (iMenuSkillPick* skillpick = dynamic_cast<iMenuSkillPick*>(Action::Get().m_menuMachine->getActiveMenu()))
            {
                // Check if skills are picked correcly
                int primary = skillpick->getPrimarySkill();
                int secondary = skillpick->getSecondarySkill();
                if (primary == -1 || secondary == -1)
                    return;

                // Set the skills on the player
                playing->getPlayer()->setCurrentSkills(primary, secondary);

                // Removing active menu
                Sound::NoiseMachine::Get().playSFX(Sound::SFX::WAVE_CARD, nullptr, true);
                Action::Get().m_menuMachine->queueMenu(iMenu::MenuGroup::Empty);
            }
        }
    }
}

void ButtonFunction::plusSense()
{
    Settings& settings = Settings::getInstance();
    settings.setFOV(settings.getFOV() + 0.01f);
}

void ButtonFunction::minusSense()
{
    Settings& settings = Settings::getInstance();
    settings.setFOV(settings.getFOV() - 0.01f);
}

void ButtonFunction::plusMaster()
{
    Settings& settings = Settings::getInstance();
    settings.setMasterSound(settings.getMasterSound() + 0.01f);
}

void ButtonFunction::minusMaster()
{
    Settings& settings = Settings::getInstance();
    settings.setMasterSound(settings.getMasterSound() - 0.01f);
}

void ButtonFunction::plusSFX()
{
    Settings& settings = Settings::getInstance();
    settings.setSFX(settings.getSFX() + 0.05f);
}

void ButtonFunction::minusSFX()
{
    Settings& settings = Settings::getInstance();
    settings.setSFX(settings.getSFX() - 0.05f);
}

void ButtonFunction::muteUnmute()
{

}

void ButtonFunction::plusFOV()
{
    Settings& settings = Settings::getInstance();
    settings.setFOV(settings.getFOV() + 1);
}

void ButtonFunction::minusFOV()
{
    Settings& settings = Settings::getInstance();
    settings.setFOV(settings.getFOV() - 1);
}

void ButtonFunction::windowed()
{
    Settings& settings = Settings::getInstance();
    settings.setWindowed(!settings.getWindowed());
}

void ButtonFunction::DOF()
{
    Settings& settings = Settings::getInstance();
    settings.setDOF(!settings.getDOF());
}

void ButtonFunction::SSAO()
{
    Settings& settings = Settings::getInstance();
    settings.setSSAO(!settings.getSSAO());
}

void ButtonFunction::fog()
{
    Settings& settings = Settings::getInstance();
    settings.setFog(!settings.getFog());
}