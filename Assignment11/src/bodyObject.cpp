#include "bodyObject.h"

#include "physicsEngine.h"

//status = tableObject.initializeStaticBody("table2.obj", btVector3(0,5,0), btVector3(5.0f, 5.0f, 5.0f));
bool BodyObject::initializeStaticBody (string fileName, btVector3 loc, btVector3 scale)
{
	if(!model.loadModel(fileName))
	{
		return false;
	}
	shape = new btBvhTriangleMeshShape(model.mTriMesh,true,true);
	shape->setLocalScaling(scale);

	motionState = new btDefaultMotionState( btTransform(btQuaternion(0,0,0,1),loc));
	btScalar mass = 10;
	btVector3 inertia(0, 0.5, 0);
	shape->calculateLocalInertia(mass, inertia);
	btRigidBody::btRigidBodyConstructionInfo RigidBodyCI( mass, motionState, shape, inertia);


		//planeRigidBodyCI.m_friction = plane_friction;     //this is the friction of its surfaces
		RigidBodyCI.m_restitution = 0.5f;     //this is the "bouncy-ness"
		RigidBodyCI.m_friction = 0.1f;
		RigidBodyCI.m_rollingFriction = 0.05f;
		rigidBody = new btRigidBody(RigidBodyCI);
		rigidBody->setLinearFactor(btVector3(0,0,0));
		rigidBody->setAngularFactor(btVector3(1,0,1));
		rigidBody->setDamping(btScalar(0.0f), btScalar(0.6));
		rigidBody->setAnisotropicFriction(shape->getAnisotropicRollingFrictionDirection(),btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);

		return true;
	}

	bool BodyObject::initializeSphereObject (string fileName, btVector3 loc, btScalar mass, btVector3 inertia, btVector3 scale, btVector3 linearFactor)
	{
		if(!model.loadModel(fileName))
		{
			return false;
		}
		//shape = new btConvexTriangleMeshShape(model.mTriMesh);
		shape = new btSphereShape(1.0);
		shape->setLocalScaling(scale);
		motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), loc));
		shape->calculateLocalInertia(mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(mass, motionState, shape, inertia);
		sphereRigidBodyCI.m_restitution = 0.7f;
		sphereRigidBodyCI.m_friction = 0.6f;
		sphereRigidBodyCI.m_rollingFriction = 0.3f;
		rigidBody = new btRigidBody(sphereRigidBodyCI);
		rigidBody->setLinearFactor(linearFactor);
		rigidBody->setAngularFactor(btVector3(1,1,1));
		rigidBody->setDamping(btScalar(0.0f), btScalar(0.6));
		rigidBody->forceActivationState(DISABLE_DEACTIVATION);
		rigidBody->setAnisotropicFriction(shape->getAnisotropicRollingFrictionDirection(),btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);

		return true;
	}

	void BodyObject::initializeBoxObject (btVector3 boxHalfExtents, btVector3 loc, btVector3 scale)
	{
        shape = new btBoxShape(boxHalfExtents);
        shape->setLocalScaling(scale);
        motionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), loc));
        btScalar mass = 10;

        btRigidBody::btRigidBodyConstructionInfo RigidBodyCI( mass, motionState, shape, btVector3(0,0.5,0));
        btVector3 inertia(0, 0.5, 0);
        shape->calculateLocalInertia(mass, inertia);
        RigidBodyCI.m_restitution = 0.0f;
        RigidBodyCI.m_friction = 0.5f;
        rigidBody = new btRigidBody(RigidBodyCI);
        rigidBody->setLinearFactor(btVector3(0,0,0));
        rigidBody->setAngularFactor(btVector3(1,0,1));
        rigidBody->setDamping(btScalar(0.0f), btScalar(0.6));
        rigidBody->forceActivationState(DISABLE_DEACTIVATION);

    }


    void BodyObject::cleanup(btDiscreteDynamicsWorld* dynamicsWorld)
    {
    	dynamicsWorld->removeRigidBody(rigidBody);
    	delete rigidBody->getMotionState();
    	delete rigidBody;
    	delete shape;
    }
