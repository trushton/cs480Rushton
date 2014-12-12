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

using namespace std;
using namespace irrklang;

#define BIT(x) (1<<(x))


struct scores
    {
        int player1Score;
        int player2Score;
    };

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
        scores * playerScore;
        int * follow;
        bool paused;
        bool twoPlayer;
        int oldPlayer1;
        int oldPlayer2;
        string paddleShape;
        string puckShape;

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
            COL_PUCK = BIT(2),
            COL_BARRIER = BIT(3),
            COL_GOAL = BIT(4)
        };



    struct GoalContactResultCallback : public btCollisionWorld::ContactResultCallback
    {
            GoalContactResultCallback(scores* ptr)
            {
                context = ptr;
                context->player1Score = 0;
                context->player2Score = 0;
            }

            btScalar addSingleResult(btManifoldPoint& cp,
                const btCollisionObjectWrapper* colObj0Wrap,
                int partId0,
                int index0,
                const btCollisionObjectWrapper* colObj1Wrap,
                int partId1,
                int index1)
                {
                    // left goal
                    if(colObj0Wrap->getCollisionObject()->isStaticObject())
                    {
                        context->player2Score ++;
                    }
                    else
                    {
                        context->player1Score ++;
                    }

                return 0;
            }
        scores * context;
    };

        struct AIContactResultCallback : public btCollisionWorld::ContactResultCallback
        {
            AIContactResultCallback(int* follow)
                {
                    context = follow;
                    *context = *follow;
                }

            btScalar addSingleResult(btManifoldPoint& cp,
                const btCollisionObjectWrapper* colObj0Wrap,
                int partId0,
                int index0,
                const btCollisionObjectWrapper* colObj1Wrap,
                int partId1,
                int index1)
                {
                    *context = 0;
                    return 0;
                }
            int * context;
        };


        GoalContactResultCallback* goalCollisionCallBack;
        AIContactResultCallback* AICollisionBack;
};

#endif
