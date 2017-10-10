#include <AI\Behavior\RangedBehavior.h>
#include <AI\Enemy.h>
#include <Misc\RandomGenerator.h>

using namespace Logic;

RangedBehavior::RangedBehavior()
{
	m_distance = 20; // distance before stopping

	setRoot(NodeType::PRIORITY, 0, NULL);

	/*
		IMP: WHEN USING NODETYPE:PROIRITY, ADDING CHILDREN SHOULD BE DONE IMMEDIATELY
		FOR PERF AND NOT CRASHING

		Should change behvaior trees to follow Data Oriented Model, but for now
		boids is more important. But the infractstructure is here.
	*/

	// STAY ?
	BehaviorNode *stay = addNode(getRoot(), NodeType::RANDOM, 0, [](RunIn& in) -> bool {
		return true;
	});

	
	// SHOOT !
	addNode(stay, NodeType::ACTION, 995, [](RunIn& in) -> bool {
		if (RandomGenerator::singleton().getRandomInt(0,
			RangedBehavior::ABILITY_CHANCHE) == 0)
			in.enemy->useAbility(*in.target);

		return true;
	});

	// jump
	addNode(stay, NodeType::ACTION, 5, [](RunIn& in) -> bool {
		in.enemy->getRigidBody()->applyCentralForce({ 0, 8888, 0 });
		return true;
	});

	// WALK TOWARDS ?
	BehaviorNode *walkTowards = addNode(getRoot(), NodeType::CONDITION, 1,
		[](RunIn& in) -> bool {
			RangedBehavior *behavior = dynamic_cast<RangedBehavior*>(in.behavior);
			return (in.enemy->getPosition() - in.target->getPosition()).Length()
					> behavior->getDistance();
		}
	);

	// WALK TOWARDS & SHOOT !
	addNode(walkTowards, NodeType::ACTION, 1,
		[](RunIn& in) -> bool {
			RangedBehavior *behavior = dynamic_cast<RangedBehavior*>(in.behavior);
			if (RandomGenerator::singleton().getRandomInt(0, RangedBehavior::ABILITY_CHANCHE) == 0)
				in.enemy->useAbility(*in.target);
			behavior->walkPath(behavior->getPath(), in);
			return true;
		}
	);
}

int RangedBehavior::getDistance() const
{
	return m_distance;
}

void RangedBehavior::walkTowardsPlayer(Enemy &enemy, Player const &player, float deltaTime)
{
		
}

void RangedBehavior::update(Enemy &enemy, std::vector<Enemy*> const &closeEnemies,
	Player const &player, float deltaTime)
{
	// this is frame bound, fix it
	RunIn in { &enemy, closeEnemies, &player, this, deltaTime };
	runTree(in);
}

void RangedBehavior::updatePath(Entity const &from, Entity const &to)
{
	m_path.loadPath(from, to);
	m_path.setCurrentNode(0);
}

void RangedBehavior::debugRendering(Graphics::Renderer &renderer)
{
}

SimplePathing RangedBehavior::getPath() const
{
	return m_path;
}