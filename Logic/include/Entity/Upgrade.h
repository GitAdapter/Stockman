#ifndef UPGRADE_H
#define UPGRADE_H

#pragma region Comment

/*
	Author: Fredholm
	This class keeps track on upgrades
*/

#pragma endregion Description of class

#include <vector>
#include "Effect.h"

namespace Logic
{
	class Upgrade
	{
	public:
        enum UPGRADE_FLAG {
            UPGRADE_IS_WEAPON = 0x1,
            UPGRADE_INCREASE_DMG = 0x2,
            UPGRADE_DECREASE_CD = 0x4,
            UPGRADE_INCREASE_SIZE = 0x8,
            UPGRADE_INCREASE_AMMOCAP = 0x20,
            UPGRADE_INCREASE_MAGSIZE = 0x40,
            UPGRADE_IS_BOUNCING = 0x80,
            UPGRADE_TRANSFERABLE = 0x100,
            UPGRADE_CROSSBOW = 0x200,
            UPGRADE_ICESTAFF = 0x400,
            UPGRADE_SLEDGEHAMMER = 0x800,
            UPGRADE_BURNING = 0x1000,
            UPGRADE_FREEZING = 0x2000,
            UPGRADE_INCREASE_JUMPHEIGHT = 0x4000,
            UPGRADE_DECREASE_RELOADTIME = 0x8000,
            UPGRADE_INCREASE_FIRERATE = 0x10000,
            UPGRADE_IS_SKILL = 0x20000,
            UPGRADE_INCREASE_MOVEMENTSPEED = 0x40000,
		};

		struct FlatUpgrades {
			float increaseCooldown;
            float increaseDmg;
            int increaseSize;
			int increaseAmmoCap;
            float increaseMagSize;
            float movementSpeed;
		};

		Upgrade();
		Upgrade(long long flags, int actionID, FlatUpgrades const &flatUpgrades);
		~Upgrade();

		// should only be created by statusmanager :9
		void init(long long flags, int actionID, FlatUpgrades const &flatUpgrades);

		// Add get & sets here when you're not lazy
		FlatUpgrades getFlatUpgrades() const;
		long long getTranferEffects() const;
	private:
		long long m_flags;
		int	m_actionID;				//< The specific ID of the weapon or skill
		FlatUpgrades m_flatUpgrades;

		long long m_tranferEffects;		//< The list of effects that should be tranfered on hit
	};
}

#endif // !UPGRADE_H
