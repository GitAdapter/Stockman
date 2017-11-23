#include "Map.h"
#include <Physics/Physics.h>        
#include <Keyboard.h>
#include <Graphics\include\Structs.h>
#include <Graphics\include\Utility\DebugDraw.h>
#include <Misc\RandomGenerator.h>
#include <toml\toml.h>
#include <fstream>
#include <Graphics/include/Utility/ModelLoader.h>

using namespace Logic;

Map::Map() { }

Map::~Map() 
{
	clear();
}

void Map::add(FrameProp frameProp)
{
  //  m_objects.push_back(new Object(frameProp.modelID, frameProp.position, frameProp.rotation));
}

void Map::add(FrameHitbox frameHitbox)
{
    //if (frameHitbox.modelID == Resources::Models::UnitCube)
    //    m_hitboxes.push_back(new StaticObject(frameHitbox.modelID, m_physicsPtr->createBody(
    //        Cube(frameHitbox.position, frameHitbox.rotation, frameHitbox.dimensions), NULL, false,
    //        Physics::COL_HITBOX,
    //        Physics::COL_EVERYTHING),
    //        {1, 1.f, 1},
    //        StaticObject::NavigationMeshFlags::CULL
    //    ));
    //else
        m_hitboxes.push_back(newd StaticObject(frameHitbox.modelID, m_physicsPtr->createBody(
            Cube(frameHitbox.position, frameHitbox.rotation, frameHitbox.dimensions), NULL, false,
            Physics::COL_HITBOX,
            Physics::COL_EVERYTHING),
            frameHitbox.dimensions,
            StaticObject::NavigationMeshFlags::NO_CULL
        ));
}

void Map::init(Physics* physics)
{
    m_physicsPtr = physics;

    // Disables debug draw as default
    m_drawDebug = false;
}

void Map::clear()
{
	for (size_t i = 0; i < m_props.size(); i++)     delete m_props[i];
	for (size_t i = 0; i < m_hitboxes.size(); i++)  delete m_hitboxes[i];
    for (size_t i = 0; i < m_lights.size(); i++)    delete m_lights[i];

	m_props.clear();
	m_hitboxes.clear();
    m_lights.clear();
}

// If user holds tab, draw debug info
void Map::update(float deltaTime)
{
#ifdef _DEBUG
    m_drawDebug = DirectX::Keyboard::Get().GetState().IsKeyDown(DirectX::Keyboard::LeftShift) ? true : false;
#endif // _DEBUG
}

void Map::render() const
{
    for (StaticObject* o : m_props)     o->render();
    for (LightObject* l : m_lights)     l->render();
    for (StaticObject* e : m_hitboxes)  e->render(); // Hitboxes should not be visiable at all at release

    for (auto & d : decorations) d.render();
}
	
std::vector<StaticObject*>*			Map::getProps()				{ return &m_props;				}
std::vector<StaticObject*>*			Map::getHitboxes()			{ return &m_hitboxes;			}
std::vector<LightObject*>*			Map::getLights()            { return &m_lights;             }

void Map::loadStartMenuScene()
{
    // Temp campfire map, remove this when an actual campfire is done
    std::vector<FrameHitbox> hitboxes;
    std::vector<FrameLight> lights;

    hitboxes.push_back({ { 0, 0.0f, 0 },{ 0, 0, 0 },{ 1.f, 1.f, 1.f },    Resources::Models::MenuScene });

    add(FrameLight({ 0.f, 0.f, 0.f }, {1.f, 0.5f, 0.3f}, 1.f, 10.f));

    for (size_t i = hitboxes.size(); i--;) add(hitboxes[i]);
    for (size_t i = lights.size(); i--;) add(lights[i]);
}

void Logic::Map::loadMap(Resources::Maps::Files map)
{
    toml::ParseResult mapFile = toml::parseFile(Resources::Maps::Paths.at(map));
    if (!mapFile.valid()) throw std::runtime_error(mapFile.errorReason);

    toml::Value mapValue = mapFile.value;

    struct Instance
    {
        std::string name;
        std::string model;
        btVector3 translation = { 0, 0, 0 };
        btQuaternion rotation = { 0, 0, 0, 1 };
        btVector3 scale = { 1, 1, 1 };
    };

    std::vector<Instance> staticInstances;
    std::vector<Instance> foliageInstances;
    std::vector<Instance> triggerInstances;

    auto mapStatic = mapValue.find("Static");
    auto mapFoliage = mapValue.find("Foliage");
    auto mapTrigger = mapValue.find("Trigger");

    static auto pushInstances = [](toml::Value * src, std::vector<Instance> & dest)
    {
        auto vec3 = [](toml::Value const* v) -> btVector3
        {
            return
            {
                (btScalar)v->find(0)->asNumber(),
                (btScalar)v->find(1)->asNumber(),
                (btScalar)v->find(2)->asNumber(),
            };
        };

        auto quat = [](toml::Value const* v) -> btQuaternion
        {
            return
            {
                (btScalar)v->find(0)->asNumber(),
                (btScalar)v->find(1)->asNumber(),
                (btScalar)v->find(2)->asNumber(),
                (btScalar)v->find(3)->asNumber(),
            };
        };

        for (auto & tInstance : src->as<toml::Array>())
        {
            Instance instance;
            instance.name = tInstance.get<std::string>("name");
            instance.model = tInstance.get<std::string>("model");

            toml::Value const* translationValue = tInstance.findChild("translation");
            toml::Value const* rotationValue = tInstance.findChild("rotation");
            toml::Value const* scaleValue = tInstance.findChild("scale");

            if (translationValue) { instance.translation = vec3(translationValue); }
            if (rotationValue) { instance.rotation = quat(rotationValue); }
            if (scaleValue) { instance.scale = vec3(scaleValue); }

            dest.push_back(instance);
        }
    };

    if (mapStatic) pushInstances(mapStatic, staticInstances);
    if (mapFoliage) pushInstances(mapFoliage, foliageInstances);
    if (mapTrigger) pushInstances(mapTrigger, triggerInstances);
    // TODO USE THIS //    

    decorations.clear();

    auto toVec3 = [](DirectX::SimpleMath::Vector3 & vec) -> btVector3
    {
        return {vec.x, vec.y, vec.z};
    };

    auto toQuat = [](DirectX::SimpleMath::Quaternion & vec) -> btQuaternion
    {
        return {vec.x, vec.y, vec.z, vec.w};
    };

    for (auto & instance : staticInstances)
    {
        try
        {
            Resources::Models::Files model = Resources::Models::toEnum(instance.model.c_str());
            std::cout << "> " <<  instance.model.c_str() << std::endl;

            if (model != Resources::Models::House1
             && model != Resources::Models::House2
             && model != Resources::Models::House3) continue;

            DirectX::SimpleMath::Quaternion rotation(instance.rotation[0], instance.rotation[1], instance.rotation[2], instance.rotation[3]);
            DirectX::SimpleMath::Vector3 scale(instance.scale[0], instance.scale[1], instance.scale[2]);
            DirectX::SimpleMath::Vector3 translation(instance.translation[0], instance.translation[1], instance.translation[2]);

            DirectX::SimpleMath::Matrix transform = DirectX::XMMatrixAffineTransformation({1,1,1}, {}, rotation, translation);
            Decoration decor(model, transform);
            decorations.push_back(decor);

            btTransform instance_transform(instance.rotation, instance.translation);
            auto hitboxes = ModelLoader::get().getModel(model)->getHitboxes();
            for (auto & hitbox : *hitboxes)
            {
                btRigidBody * body = m_physicsPtr->createHitbox(
                    toVec3(hitbox.position),
                    toQuat(hitbox.rotation),
                    toVec3(hitbox.halfSize),
                    false,
                    Physics::COL_HITBOX,
                    Physics::COL_EVERYTHING ^ Physics::COL_HITBOX
                );

                body->setWorldTransform(instance_transform * body->getWorldTransform());
            }
        }
        catch (const char * e)
        {
            std::cerr << "Could not find model " << instance.model << " during map load. Ignoring model." << std::endl;
        }
    }
}

// Adds a pointlight to the map
void Map::add(FrameLight frameLight)
{
    m_lights.push_back(newd LightObject(frameLight));
}

//
//// Adds a static hitbox to the map
//void Map::add(FrameHitbox frameHitbox)
//{
//    m_hitboxes.push_back(newd StaticObject(
//        frameHitbox.modelID, 
//        m_physicsPtr->createBody(
//            Cube(frameHitbox.position, frameHitbox.rotation, frameHitbox.dimensions), /* Shape */             
//            NULL,                   /* Mass */              
//            false,                  /* Sensor */            
//            Physics::COL_HITBOX,    /* Collision Type */    
//            Physics::COL_EVERYTHING /* Collides With */     
//        ),
//        frameHitbox.dimensions, /* Graphical Scaling */ 
//        StaticObject::NavigationMeshFlags::NO_CULL
//    ));
//}