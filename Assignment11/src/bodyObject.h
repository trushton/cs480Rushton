#ifndef BODY_OBJECT_H
#define BODY_OBJECT_H

#include <iostream>
#include <btBulletDynamicsCommon.h>
#include "model.h"

#include <irrKlang.h>
#include "audio_files/conio.h"


using namespace irrklang;
using namespace std;

class BodyObject
{

    public:

    	bool initializeStaticBody(string fileName, btVector3 loc, btVector3 scale);
      	bool initializeSphereObject(string fileName, btVector3 loc, btScalar mass, btVector3 inertia, btVector3 scale, btVector3 linearFactor);
      	void initializeBoxObject (btVector3 boxHalfExtents, btVector3 loc, btVector3 scale);
    	void cleanup(btDiscreteDynamicsWorld* dynamicsWorld);

        btCollisionShape* shape;
        btDefaultMotionState* motionState;
        btRigidBody* rigidBody;

        Model model;
        float xAxisRotation;
        float zAxisRotation;

    private:

};

#endif
