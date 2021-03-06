#ifndef ENEMY_TEST
#define ENEMY_TEST

#include <AI\Enemy.h>
#include <Player\Player.h>

namespace Logic
{
	class EnemyTest : public Enemy
	{
		public:
			EnemyTest(btRigidBody* body, btVector3 halfExtent);
			virtual ~EnemyTest();

			virtual void clear();
			virtual void onCollision(PhysicsObject& other, btVector3 contactPoint, float dmgMultiplier);
			virtual void onCollision(Player& other);
			virtual void updateSpecific(Player const &player, float deltaTime);
			virtual void updateDead(float deltaTime);
	};
}
#endif