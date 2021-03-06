#include <AI\Behavior\BigBadBehavior.h>
#include <AI\Enemy.h>
using namespace Logic;

const float BigBadBehavior::MELEE_RANGE = 8.5f;
const int BigBadBehavior::PHASE_ONE = 0, BigBadBehavior::PHASE_TWO = 1, BigBadBehavior::PHASE_THREE = 2;

BigBadBehavior::BigBadBehavior() :
    Behavior(PathingType::CHASING)
{
    setRoot(NodeType::PRIORITY, 0, [](RunIn &in) -> bool {
        in.behavior->walkPath(in);
        return true;
    });
    BehaviorNode *parent = getRoot();

    // melee
    BehaviorNode *node = addNode(parent, NodeType::CONDITION, 4, [](RunIn &in) -> bool {
        return (in.enemy->getPositionBT() - in.target->getPositionBT()).length() < MELEE_RANGE;
    });
    addNode(node, NodeType::ACTION, 0, [](RunIn &in) -> bool {
        in.enemy->useAbility(*in.target);
        return true;
    });

    node = addNode(parent, NodeType::CONDITION, 3, [](RunIn &in) -> bool {
        return (static_cast<float> (in.enemy->getHealth()) / in.enemy->getMaxHealth()) > 0.65f;
    });
    addNode(node, NodeType::ACTION, 0, [](RunIn &in) -> bool {
        in.enemy->useAbility(*in.target, PHASE_ONE);
        return true;
    });

    node = addNode(parent, NodeType::CONDITION, 2, [](RunIn &in) -> bool {
        return (static_cast<float> (in.enemy->getHealth()) / in.enemy->getMaxHealth()) > 0.30f;
    });
    addNode(node, NodeType::ACTION, 0, [](RunIn &in) -> bool {
        in.enemy->useAbility(*in.target, PHASE_TWO);
        return true;
    });

    addNode(parent, NodeType::ACTION, 0, [](RunIn &in) -> bool {
        in.enemy->useAbility(*in.target, PHASE_THREE);
        return true;
    });
}

BigBadBehavior::~BigBadBehavior()
{

}