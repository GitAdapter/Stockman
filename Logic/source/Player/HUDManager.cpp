#include <Player\HUDManager.h>
#include <Player\Player.h> 
#include <Player\Skill\Skill.h>
#include <Player\Skill\SkillManager.h>
#include <Player\Weapon\Weapon.h>

#include <AI\WaveTimeManager.h>
#include <AI\EntityManager.h>

#include <Misc\ComboMachine.h>

#include <Graphics\include\Renderer.h>

using namespace Logic;

const int HUDManager::CURRENT_AMMO = 0;
const int HUDManager::TOTAL_AMMO = 1;


void Logic::HUDManager::constructHUD(Graphics::HUDInfo * info)
{

}

HUDManager::HUDManager()
{
    info = newd Graphics::HUDInfo;
    ZeroMemory(info, sizeof(info));
    info->cd0 = 1.0f;
    info->cd1 = 1.0f;
    info->currentSkills[0] = -1;
    info->currentSkills[1] = -1;
}

HUDManager::~HUDManager()
{
    delete info;
}

void HUDManager::update(Player const &player, WaveTimeManager const &timeManager,
    EntityManager const &entityManager)
{
    //updates hudInfo with the current info
    info->score = ComboMachine::Get().GetCurrentScore();
    info->hp = player.getHP();
    info->activeAmmo[HUDManager::CURRENT_AMMO] =     player.getMainHand()->getMagAmmo();
    info->activeAmmo[HUDManager::TOTAL_AMMO] =   player.getMainHand()->getAmmo();
    info->inactiveAmmo[HUDManager::CURRENT_AMMO] =   player.getOffHand()->getMagAmmo();
    info->inactiveAmmo[HUDManager::TOTAL_AMMO] = player.getOffHand()->getAmmo();
    info->sledge = player.isUsingMeleeWeapon();
    info->currentWeapon = player.getCurrentWeapon();

    // HUD info on the first skill
    const Skill* primary = player.getSkill(SkillManager::ID::PRIMARY);
    if (!primary->getCanUse())
        info->cd0 = primary->getCooldown() / primary->getCooldownMax();
    else
        info->cd0 = 1.0f;

    const Skill* secondary = player.getSkill(SkillManager::ID::SECONDARY);
    if (!secondary->getCanUse())
        info->cd1 = secondary->getCooldown() / secondary->getCooldownMax();
    else
        info->cd1 = 1.0f;

    info->wave = timeManager.getCurrentWave() + 1;
    info->timeRemaining = (timeManager.getTimeRequired() - timeManager.getTimeCurrent()) * 0.001f;
    info->enemiesRemaining = (int)entityManager.getNrOfAliveEnemies();


    info->currentSkills[0] = player.getCurrentSkill0();
    info->currentSkills[1] = player.getCurrentSkill1();

    
}

void HUDManager::render(Graphics::Renderer &renderer)
{
    renderer.fillHUDInfo(info);
}