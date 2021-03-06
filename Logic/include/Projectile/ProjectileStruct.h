#ifndef PROJECTILESTRUCT_H
#define PROJECTILESTRUCT_H

#include <Entity\Entity.h>

#include <Graphics\include\RenderInfo.h>
#include <Graphics\include\Particles\ParticleSystem.h>

namespace Logic
{
	enum ProjectileType
	{
		ProjectileTypeNormal,
		ProjectileTypeGrappling,
		ProjectileTypeMelee,
        ProjectileTypeMeleeParry,
		ProjectileTypeShield,
		ProjectileTypeBulletTime,
		ProjectileTypeBulletTimeSensor,
		ProjectileTypeIce,
        ProjectileTypeFreezeGrenade,
        ProjectileTypeFreezeExplosion,
        ProjectileTypeIceShard,
        ProjectileTypeFireArrow,
        ProjectileTypeDefenderShield,
        ProjectileTypeShieldCharge
	};

    

	struct ProjectileData
	{
		float damage;				// Projectile Damage
		float scale;				// Scale of the projectile
		float mass;					// Mass of projectile
		float speed;				// Bullet speed
		float gravityModifier;		// How fast the bullet falls to the ground
		float ttl;					// Time to live in milisec

        LightRenderInfo lightInfo;
        FancyRenderInfo fancy;

		ProjectileType type;
		Resources::Models::Files meshID;
		int materialID;

        bool hasEffect;
        bool effectVelocity;
        bool effectActivated;
        Graphics::ParticleEffect effect;

		bool isSensor;				// If bullet is sensor or not (collision with other bullet)
		bool enemyBullet;			// if enemies shot it or a player
        bool shouldRender;          // If projectile should render
        bool dmgFallOff;

        btVector3 modelOffset;

        ProjectileData() : hasEffect(false), effectActivated(false), effectVelocity(false), effect({}), damage(1.f), scale(1.f),
            mass(1.f), fancy({}), speed(1.f), gravityModifier(0.f), ttl(1000), meshID(Resources::Models::UnitCube), materialID(1),
            type(ProjectileTypeNormal), isSensor(false), enemyBullet(false), dmgFallOff(false), modelOffset({ 0.f, 0.f, 0.f }) {}
        ProjectileData(const char *fx, bool effectVelocity, bool effectActivated, float inDamage, float inScale,
            float inMass, float inSpeed, float inGravityModifier, float inTTL, LightRenderInfo inLightInfo,
            Resources::Models::Files inMeshID, int inMaterialID, ProjectileType inType = ProjectileTypeNormal,
            bool inIsSensor = false, bool inEnemyBullet = false, bool inShouldRender = true, bool inDmgFallOff = false, btVector3 inModelOffset = { 0.f, 0.f, 0.f })
            : damage(inDamage), scale(inScale), mass(inMass), speed(inSpeed), lightInfo(inLightInfo),
            effectVelocity(effectVelocity), effectActivated(effectActivated), gravityModifier(inGravityModifier),
            ttl(inTTL), meshID(inMeshID), materialID(inMaterialID), type(inType), isSensor(inIsSensor),
            enemyBullet(inEnemyBullet), shouldRender(inShouldRender), dmgFallOff(inDmgFallOff), modelOffset(inModelOffset)
        {
            if (fx) {
                hasEffect = true;
                effect = Graphics::FXSystem->getEffect(fx);
            }
            else {
                hasEffect = false;
            }
        }
	};
}

#endif // !PROJECTILESTRUCT_H
