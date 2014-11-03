#include "bodyObject.h"

#include "physicsEngine.h"


bool BodyObject::initializeStaticBody (string fileName, btVector3 loc, btVector3 scale)
	{
		if(!model.loadModel(fileName))
			{	
				return false;
			}
		shape = new btBvhTriangleMeshShape(model.mTriMesh,true,true);
		shape->setLocalScaling(scale);

		motionState = new btDefaultMotionState( btTransform(btQuaternion(0,0,0,1),loc));
		btRigidBody::btRigidBodyConstructionInfo RigidBodyCI( 0, motionState, shape, btVector3(0,0,0));
		btScalar mass = 0;
		btVector3 inertia(0, 0, 0);
		shape->calculateLocalInertia(mass, inertia);

		//planeRigidBodyCI.m_friction = plane_friction;     //this is the friction of its surfaces
		RigidBodyCI.m_restitution = 0.5f;     //this is the "bouncy-ness"
		RigidBodyCI.m_friction = 0.5f;
		rigidBody = new btRigidBody(RigidBodyCI);
		rigidBody->setLinearFactor(btVector3(0,0,0));
		rigidBody->setAngularFactor(btVector3(0,0,0));

		return true;
	}

bool BodyObject::initializeMovingBody (string fileName, btVector3 loc, btScalar mass, btVector3 inertia, btVector3 scale)
	{
		if(!model.loadModel(fileName))
			{	
				return false;
			}
		shape = new btConvexTriangleMeshShape(model.mTriMesh);
		shape->setLocalScaling(scale);
		motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), loc));
		shape->calculateLocalInertia(mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(mass, motionState, shape, inertia);
		sphereRigidBodyCI.m_restitution = 0.7f;
		//sphereRigidBodyCI.m_friction = 0.6f;

		rigidBody = new btRigidBody(sphereRigidBodyCI);
		rigidBody->setLinearFactor(btVector3(1,0,1));
		rigidBody->setAngularFactor(btVector3(0,1,0));
		rigidBody->setDamping(btScalar(0.0f), btScalar(0.6));
		rigidBody->forceActivationState(DISABLE_DEACTIVATION);
		//rigidBody->setLinearLowerLimit(btVector3(0,0,1));

		return true;
	}

void BodyObject::initializeBoxObject (btVector3 boxHalfExtents, btVector3 loc, btVector3 scale)
	{
		shape = new btBoxShape(boxHalfExtents);
		shape->setLocalScaling(scale);
		motionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), loc));
		btRigidBody::btRigidBodyConstructionInfo RigidBodyCI( 0, motionState, shape, btVector3(0,0,0));
		btScalar mass = 0;
		btVector3 inertia(0, 0, 0);
		shape->calculateLocalInertia(mass, inertia);
		RigidBodyCI.m_restitution = 0.5f;
		RigidBodyCI.m_friction = 0.5f;
		rigidBody = new btRigidBody(RigidBodyCI);
		rigidBody->setLinearFactor(btVector3(0,0,0));
		rigidBody->setAngularFactor(btVector3(0,0,0));
	}


void BodyObject::cleanup(btDiscreteDynamicsWorld* dynamicsWorld)
	{
		dynamicsWorld->removeRigidBody(rigidBody);
		delete rigidBody->getMotionState();
		delete rigidBody;
		delete shape;
	}
