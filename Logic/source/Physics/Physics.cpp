#include "Physics\Physics.h"
#include <Singletons\DebugWindow.h>

using namespace Logic;

#include <libs\Bullet2.86\include\BulletCollision\CollisionDispatch\btGhostObject.h>

Physics::Physics(btCollisionDispatcher* dispatcher, btBroadphaseInterface* overlappingPairCache, btSequentialImpulseConstraintSolver* constraintSolver, btDefaultCollisionConfiguration* collisionConfiguration)
	: btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, constraintSolver, collisionConfiguration)
{
	this->dispatcher = dispatcher;
	this->overlappingPairCache = overlappingPairCache;
	this->constraintSolver = constraintSolver;
	this->collisionConfiguration = collisionConfiguration;

	// Render Debug Construction
	debugRenderInfo.points = newd std::vector<DirectX::SimpleMath::Vector3>();
	debugRenderInfo.color = DirectX::SimpleMath::Color(1, 1, 1);
	debugRenderInfo.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	debugRenderInfo.useDepth = true;
}

Physics::~Physics()
{
	delete debugRenderInfo.points;
	clear();
	delete ghostPairCB;
}

bool Physics::init()
{
    registerDebugCommands();
	// World gravity
	this->setGravity(btVector3(0, -PHYSICS_GRAVITY, 0));
	this->setLatencyMotionStateInterpolation(true);
	ghostPairCB = newd btGhostPairCallback();
	m_broadphasePairCache->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCB);

	return true;
}

void Physics::clear()
{
	// Cleanup in the reverse order of creation / initialization!
	for (int i = this->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = this->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		btCollisionShape* shape = obj->getCollisionShape();
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		this->removeCollisionObject(obj);
        if (shape)
		    delete shape;
		delete obj;
	} 

	// Deleting members
	delete constraintSolver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}

void Physics::update(float delta)
{
	// Stepping the physics
	PROFILE_BEGIN("Stepping Physics");

	if (delta * 0.001f > (1.f / 60.f))
		this->stepSimulation(1.f / 60.f, 0, 0);
	else
		this->stepSimulation(delta * 0.001f, 0, 0);

	PROFILE_END();

	PROFILE_BEGIN("Collision Handling");

	// Collisions
	int numManifolds = dispatcher->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
	
		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		if (numContacts > 0) // Only returns the first contact as for now, fix this
		{
	//		if (obA->getCollisionShape()->getShapeType() != BroadphaseNativeTypes::SPHERE_SHAPE_PROXYTYPE)
		
			btManifoldPoint contactPoint = contactManifold->getContactPoint(0);

			btVector3 b = contactPoint.getPositionWorldOnA();
			btVector3 a = contactPoint.getPositionWorldOnB();

			PhysicsObject* entityA = reinterpret_cast<PhysicsObject*>(obA->getUserPointer());
			PhysicsObject* entityB = reinterpret_cast<PhysicsObject*>(obB->getUserPointer());

			if (entityA && entityB)
			{
				entityA->collision(*entityB, a, *this);
				entityB->collision(*entityA, b, *this);
			}
		}
	}
	PROFILE_END();
}

void Physics::registerDebugCommands()
{
    DebugWindow *win = DebugWindow::getInstance();
    win->registerCommand("PHYSICS_ADD_CUBE", [&](std::vector<std::string> &para) -> std::string {
        try {
            float x, y, z, width, height, length;
            x = stof(para.at(0));
            y = stof(para.at(1));
            z = stof(para.at(2));
            width = stof(para.at(3));
            height = stof(para.at(4));
            length = stof(para.at(5));
            Cube cube({ x, y, z }, { 0, 0, 0 }, {width, height, length});
            createBody(cube, 0);
            return "Cube created";
        }
        catch (std::exception ex)
        {
            return "Writing coordinates is tough\n\n\nFor you.";
        }
    });
}

// Returns nullptr if not intersecting, otherwise returns the rigidbody of the hit
const btRigidBody* Physics::RayTestOnRigidBodies(Ray& ray)
{
	const btVector3& start	= ray.getStart();
	const btVector3& end	= ray.getEnd();

	// Ray testing to see first callback
	btCollisionWorld::ClosestRayResultCallback rayCallBack(start, end);
	this->rayTest(start, end, rayCallBack);

	if (rayCallBack.hasHit())
	{
		const btCollisionObject* object = rayCallBack.m_collisionObject;
		const btRigidBody* body = btRigidBody::upcast(object);

		return body;
	}

	return nullptr;
}

// Returns the actual point of intersection, returns { 0, 0, 0 } if not hit (I KNOW, IT BECOMES FUCKING STUPID IF YOU ACTUALLY HIT THE 0,0,0, BUT I DIDN''T FIND A SIMPLE WAY)
const btVector3 Physics::RayTestGetPoint(Ray & ray)
{
	const btVector3& start = ray.getStart();
	const btVector3& end = ray.getEnd();

	// Ray testing to see first callback
	btCollisionWorld::ClosestRayResultCallback rayCallBack(start, end);
	this->rayTest(start, end, rayCallBack);

	if (rayCallBack.hasHit())
	{
		const btVector3 object = rayCallBack.m_hitPointWorld;
		return object;
	}

	return { 0, 0, 0 };
}

const btVector3 Physics::RayTestGetNormal(Ray & ray)
{
	const btVector3& start = ray.getStart();
	const btVector3& end = ray.getEnd();

	// Ray testing to see first callback
	btCollisionWorld::ClosestRayResultCallback rayCallBack(start, end);
	this->rayTest(start, end, rayCallBack);

	if (rayCallBack.hasHit())
	{
		const btVector3 object = rayCallBack.m_hitNormalWorld;
		return object;
	}

	return { 0, 0, 0 };
}

btRigidBody* Physics::createBody(Shape* shape, float mass, bool isSensor, int group, int mask)
{
    btRigidBody* body = nullptr;

    switch (shape->getType())
    {
    case ShapeType::ShapeTypeCube:      body = createBody(static_cast<Cube&>       (*shape), mass, isSensor, group, mask); break;
    case ShapeType::ShapeTypeCapsule:   body = createBody(static_cast<Capsule&>    (*shape), mass, isSensor, group, mask); break;
    case ShapeType::ShapeTypeCylinder:  body = createBody(static_cast<Cylinder&>   (*shape), mass, isSensor, group, mask); break;
    case ShapeType::ShapeTypePlane:     body = createBody(static_cast<Plane&>      (*shape), mass, isSensor, group, mask); break;
    case ShapeType::ShapeTypeSphere:    body = createBody(static_cast<Sphere&>     (*shape), mass, isSensor, group, mask); break;
    default: printf("Could not create rigidbody, what the fuck did you do?\n"); break;
    }

    return body;
}

btRigidBody* Physics::createBody(Cube& cube, float mass, bool isSensor, int group, int mask)
{
	// Setting Motions state with position & rotation
	btQuaternion rotation;
	rotation.setEulerZYX(cube.getRot().getZ(), cube.getRot().getY(), cube.getRot().getX());
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, cube.getPos()));

	// Creating the specific shape
	btCollisionShape* shape = new btBoxShape(cube.getDimensions());
	
	btVector3 localInertia(0, 0, 0);
	if (mass != 0.f)
		shape->calculateLocalInertia(mass, localInertia);

	// Creating the actual body
	btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, shape, localInertia);
	BodySpecifics specifics(DEFAULT_R, DEFAULT_F, DEFAULT_S, DEFAULT_D, isSensor);
	btRigidBody* body = initBody(constructionInfo, specifics);
	shape->setUserPointer(body);

	// Adding body to the world
	this->addRigidBody(body, group, mask);

	return body;
}

btRigidBody * Physics::createBody(Plane& plane, float mass, bool isSensor, int group, int mask)
{
	// Setting Motions state with position & rotation
	btQuaternion rotation;
	rotation.setEulerZYX(plane.getRot().getZ(), plane.getRot().getY(), plane.getRot().getX());
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, plane.getPos()));

	// Creating the specific shape
	btCollisionShape* shape = new btStaticPlaneShape(plane.getNormal(), 0.1f);

	// Creating the actual body
	btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, shape);
	BodySpecifics specifics(DEFAULT_R, DEFAULT_F, DEFAULT_S, DEFAULT_D, isSensor);
	btRigidBody* body = initBody(constructionInfo, specifics);
	shape->setUserPointer(body);

	// Adding body to the world
    this->addRigidBody(body, group, mask);

	return body;
}

btRigidBody * Physics::createBody(Sphere& sphere, float mass, bool isSensor, int group, int mask)
{
	// Setting Motions state with position & rotation
	btQuaternion rotation;
	rotation.setEulerZYX(sphere.getRot().getZ(), sphere.getRot().getY(), sphere.getRot().getX());
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, sphere.getPos()));

	// Creating the specific shape
	btCollisionShape* shape = new btSphereShape(sphere.getRadius());

	// Creating the actual body
	btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, shape);
	BodySpecifics specifics(DEFAULT_R, DEFAULT_F, DEFAULT_S, DEFAULT_D, isSensor);
	btRigidBody* body = initBody(constructionInfo, specifics);
	shape->setUserPointer(body);

	// Adding body to the world
    this->addRigidBody(body, group, mask);

	return body;
}

btRigidBody* Physics::createBody(Cylinder& cylinder, float mass, bool isSensor, int group, int mask)
{
	// Setting Motions state with position & rotation
	btQuaternion rotation;
	rotation.setEulerZYX(cylinder.getRot().getZ(), cylinder.getRot().getY(), cylinder.getRot().getX());
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, cylinder.getPos()));

	// Creating the specific shape
	btCollisionShape* shape = new btCylinderShape(cylinder.getHalfExtends());

	// Creating the actual body
	btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, shape);
	BodySpecifics specifics(DEFAULT_R, DEFAULT_F, DEFAULT_S, DEFAULT_D, isSensor);
	btRigidBody* body = initBody(constructionInfo, specifics);
	shape->setUserPointer(body);

	// Adding body to the world
    this->addRigidBody(body, group, mask);

	return body;
}

btRigidBody* Physics::createBody(Capsule& capsule, float mass, bool isSensor, int group, int mask)
{
	// Setting Motions state with position & rotation
	btQuaternion rotation;
	rotation.setEulerZYX(capsule.getRot().getZ(), capsule.getRot().getY(), capsule.getRot().getX());
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, capsule.getPos()));

	// Creating the specific shape
	btCollisionShape* shape = new btCapsuleShape(capsule.getRadius(), capsule.getHeight());

	// Creating the actual body
	btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, shape);
	BodySpecifics specifics(DEFAULT_R, DEFAULT_F, DEFAULT_S, DEFAULT_D, isSensor);
	btRigidBody* body = initBody(constructionInfo, specifics);
	shape->setUserPointer(body);

	// Adding body to the world
    this->addRigidBody(body, group, mask);

	return body;
}

btPairCachingGhostObject* Physics::createPlayer(btCapsuleShape* capsule, btVector3 pos)
{
	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();

	ghostObject->setCollisionShape(capsule);

	// Rotation
	btQuaternion rotation;
	rotation.setEulerZYX(0.f, 0.f, 0.f);
	ghostObject->getWorldTransform().setRotation(rotation);

	// Position
	btTransform transform = ghostObject->getWorldTransform();
	transform.setOrigin(pos);
	ghostObject->setWorldTransform(transform);

	// Adding to physics world
	this->addCollisionObject(ghostObject, COL_FLAG::COL_PLAYER, Physics::COL_EVERYTHING);

	return ghostObject;
}

btRigidBody * Logic::Physics::createHitbox(btVector3 position, btQuaternion rotation, btVector3 halfSize, bool isSensor, int group, int mask)
{
    // Setting Motions state with position & rotation
    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, position));

    // Creating the specific shape
    btCollisionShape* shape = new btBoxShape(halfSize);

    // Creating the actual body
    btRigidBody::btRigidBodyConstructionInfo constructionInfo(0, motionState, shape);
    BodySpecifics specifics(DEFAULT_R, DEFAULT_F, DEFAULT_S, DEFAULT_D, isSensor);
    btRigidBody* body = initBody(constructionInfo, specifics);
    shape->setUserPointer(body);

    // Adding body to the world
    this->addRigidBody(body, group, mask);

    return body;
}

// Only used for debugging, draws all collision shapes onto screen
void Physics::render()
{
	// Clearing last debug draw
	debugRenderInfo.points->clear();

	for (int i = this->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = this->getCollisionObjectArray()[i];
		if (btGhostObject* ghostObject = dynamic_cast<btGhostObject*>(obj))
		{
			renderGhostCapsule(dynamic_cast<btCapsuleShape*>(ghostObject->getCollisionShape()), ghostObject);
		}
		else
		{
			btRigidBody* body = btRigidBody::upcast(obj);
			btCollisionShape* shape = obj->getCollisionShape();

            // Static Bodies have already been built as debug mesh and gets drawn from map.cpp
            if (!shape->isNonMoving())
            {
                // Render Boxes
                if (btBoxShape* bs = dynamic_cast<btBoxShape*>(shape))
                    renderCube(bs, body);

                // Render Spheres
                else if (btSphereShape* ss = dynamic_cast<btSphereShape*>(shape))
                    renderSphere(ss, body);

                // Render Cylinders
                else if (btCylinderShape* cs = dynamic_cast<btCylinderShape*>(shape))
                    renderCylinder(cs, body);

                // Render Capsules
                else if (btCapsuleShape* cs = dynamic_cast<btCapsuleShape*>(shape))
                    renderCapsule(cs, body);
            }
		}
	}

    QueueRender(debugRenderInfo);
}

btRigidBody* Physics::initBody(btRigidBody::btRigidBodyConstructionInfo constructionInfo, BodySpecifics specifics)
{
	btRigidBody* body = new btRigidBody(constructionInfo);

	// If the body is a trigger
	if (specifics.isSensor)
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	// Specifics
	body->setRestitution(specifics.restitution);
	body->setFriction(specifics.friction);
	body->setSleepingThresholds(specifics.sleepingThresholds.x, specifics.sleepingThresholds.y);
	body->setDamping(specifics.damping.x, specifics.damping.y);

	return body;
}

// Render a debug with help of it's vertices
void Physics::renderCube(btBoxShape* bs, btRigidBody* body)
{
	btVector3 vp = { 0, 0, 0 };
	btVector3 center = body->getWorldTransform().getOrigin();
	btQuaternion q = body->getWorldTransform().getRotation();
	DirectX::SimpleMath::Matrix quaternion = DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::SimpleMath::Quaternion(q));

	// Front side
	for (int i = 0; i < bs->getNumVertices() - 1; i++)
	{
		bs->getVertex(0, vp);
        debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
		bs->getVertex(i + 1, vp);
        debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));

		bs->getVertex(bs->getNumVertices() - 1, vp);
        debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
		bs->getVertex(i + 1, vp);
        debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	}


	// Diagonal right
	bs->getVertex(5, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(4, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(5, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(1, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));

	// Diagonal left
	bs->getVertex(6, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(4, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(6, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(2, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));

	// Diagonal top
	bs->getVertex(3, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(1, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(3, vp);
    debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
	bs->getVertex(2, vp);
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(center) + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vp), quaternion));
}

// Draws a cube around the sphere
void Physics::renderSphere(btSphereShape* ss, btRigidBody* body)
{
	btVector3 origin = body->getWorldTransform().getOrigin();
	btVector3 c;
	float r = 0.f;
	ss->getBoundingSphere(c, r);
	r /= 2;

	renderRectangleAround(origin, {r, r, r});
}

// Draws a cube around the cylinder
void Physics::renderCylinder(btCylinderShape * cs, btRigidBody * body)
{
	btVector3 origin = body->getWorldTransform().getOrigin();
	btVector3 half = cs->getHalfExtentsWithMargin();

	renderRectangleAround(origin, half);
}

// Draws a cube around the capsule
void Physics::renderCapsule(btCapsuleShape* cs, btRigidBody* body)
{
	btVector3 origin = body->getWorldTransform().getOrigin();
	btVector3 half = cs->getImplicitShapeDimensions();

	renderRectangleAround(origin, half);
}

// Draws a cube around the capsule
void Physics::renderGhostCapsule(btCapsuleShape* cs, btGhostObject* ghostObject)
{
	btVector3 origin = ghostObject->getWorldTransform().getOrigin();
	btVector3 half = cs->getImplicitShapeDimensions();

	renderRectangleAround(origin, half);
}

void Physics::renderRectangleAround(btVector3 origin, btVector3 half)
{
	// Side Front
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() + half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() + half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() - half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() - half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() + half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() - half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() + half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() - half.y(), origin.z() - half.z()));

	// Side Back
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() + half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() + half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() - half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() - half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() + half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() - half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() + half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() - half.y(), origin.z() - half.z()));

	// Left
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() + half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() + half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() - half.y(), origin.z() + half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() - half.y(), origin.z() + half.z()));

	// Right
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() + half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() + half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() + half.x(), origin.y() - half.y(), origin.z() - half.z()));
	debugRenderInfo.points->push_back(DirectX::SimpleMath::Vector3(origin.x() - half.x(), origin.y() - half.y(), origin.z() - half.z()));
}
