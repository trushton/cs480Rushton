#ifndef PHYSICS_H
#define PHYSICS_H

#include <iostream>
#include <btBulletDynamicsCommon.h>
#include "bodyObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

#include <irrKlang.h>
#include "audio_files/conio.h"
#include "timer.h"

using namespace std;
using namespace irrklang;

#define BIT(x) (1<<(x))

class Physics
{

    public:

        bool initializePhysicsEngine();
        bool setupWorld();
        void cleanup();
        ISoundEngine* engine;


        void updateWorld(float dt);
        float getInterceptTime(btTransform puckTransform);

        vector<BodyObject> objects;
        int * holeDetected;
        int * goalDetected;
        bool paused;
        bool twoBalls;
        string levelFileName;
        glm::vec3 ballPos;
        glm::vec3 ball2Pos;
        glm::vec3 goalPos;
        Timer timer;

    private:

        btBroadphaseInterface* broadphase;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btSequentialImpulseConstraintSolver* solver;
        btDiscreteDynamicsWorld* dynamicsWorld;

        enum collisiontypes {
            COL_NOTHING = 0,
            COL_PADDLE = BIT(0),
            COL_TABLE = BIT(1),
            COL_BALL = BIT(2),
            COL_PLANE = BIT(3),
            COL_GOAL = BIT(4)
        };

    struct HoleContactResultCallback : public btCollisionWorld::ContactResultCallback
    {
            HoleContactResultCallback(int* holeDetected)
            {
                context = holeDetected;
            }

            btScalar addSingleResult(btManifoldPoint& cp,
                const btCollisionObjectWrapper* colObj0Wrap,
                int partId0,
                int index0,
                const btCollisionObjectWrapper* colObj1Wrap,
                int partId1,
                int index1)
            {
                *context = 1;
                cout << "OMG...ThE BALL HATH FALLEN INTO A HOLE.\n";

                return 0;
            }
        int * context;
    };

    struct GoalContactResultCallback : public btCollisionWorld::ContactResultCallback
    {
            GoalContactResultCallback(int* goalDetected)
            {
                context = goalDetected;
            }

            btScalar addSingleResult(btManifoldPoint& cp,
                const btCollisionObjectWrapper* colObj0Wrap,
                int partId0,
                int index0,
                const btCollisionObjectWrapper* colObj1Wrap,
                int partId1,
                int index1)
            {
                *context = 1;
                cout << "YOU WIN!!!!!!!.\n";

                return 0;
            }
        int * context;
    };

    HoleContactResultCallback* holeCollisionBack;
    GoalContactResultCallback* goalCollisionBack;
};

#endif
