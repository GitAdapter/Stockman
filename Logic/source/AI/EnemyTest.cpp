#include "AI/EnemyTest.h"
using namespace Logic;

EnemyTest::EnemyTest()
: Enemy(10, 5, 3, 1) { //just test values
}


EnemyTest::~EnemyTest()
{
}

void EnemyTest::clear()
{
}

void EnemyTest::onCollision(const Entity &other)
{
}

void EnemyTest::onCollision(const Player& other) 
{
	
}

void EnemyTest::updateSpec(float deltaTime) 
{
	
}

void EnemyTest::updateDead(float deltaTime)
{
	// x _ x
}
