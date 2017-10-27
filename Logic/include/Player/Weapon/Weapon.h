#ifndef WEAPON_H
#define WEAPON_H

#pragma region ClassDesc
		
		/*
			CLASS: Weapon
			AUTHOR:

			DESCRIPTION: This class defines a weapon of the system
		*/

#pragma endregion

#include <Entity\Object.h>
//#include <Projectile\ProjectileManager.h>
#include <Projectile\ProjectileStruct.h>

namespace Graphics
{
    class Renderer;
    class Structs;
}

namespace Logic
{
    class ProjectileManager;
    //struct ProjetileData;

	class Weapon : public Object
	{
	private:
		DirectX::SimpleMath::Matrix rot, trans, scale;
		ProjectileData m_projectileData;
		int m_weaponID;
		int m_ammoCap;
		int m_ammo;
		int m_magSize;
		int m_magAmmo;
		int m_ammoConsumption;
		int m_projectileCount;
		int m_spreadH;							// Horizontal spread in degrees	
		int m_spreadV;							// Vertical spread in degrees
		float m_attackRate;						// Attacks per minute
		float m_freeze;
		float m_reloadTime;
	//	Animation m_animation;

		btVector3 calcSpread(float yaw, float pitch);

        std::function<Projectile*(ProjectileData& pData, btVector3 position,
            btVector3 forward, Entity& shooter)> SpawnProjectile;

	public:
		Weapon();
		Weapon(Graphics::ModelID modelID, ProjectileManager* projectileManager, ProjectileData &projectileData, int weaponID, int ammoCap, int ammo, int magSize, int magAmmo, int ammoConsumption, int projectileCount,
			int spreadH, int spreadV, float attackRate, float freeze, float reloadTime);
		void reset();

        void setSpawnFunctions(ProjectileManager &projManager);

		void use(btVector3 position, float yaw, float pitch, Entity& shooter);
		void setWeaponModelFrontOfPlayer(DirectX::SimpleMath::Matrix playerTranslation, DirectX::SimpleMath::Vector3 playerForward);

		ProjectileData* getProjectileData();
		int getAmmoCap();
		void setAmmoCap(int ammoCap);
		int getAmmo();
		void setAmmo(int ammo);
		int getMagSize();
		void setMagSize(int magSize);
		int getMagAmmo();
		void removeMagAmmo();
		void removeMagAmmo(int ammo);
		int getAmmoConsumption();
		float getAttackTimer();
		float getRealoadTime();

		void fillMag();
	};
}

#endif