#include "physicsEngine.h"


bool Physics::initializePhysicsEngine()
{
        paused = false;
        twoPlayer = true;

        engine = createIrrKlangDevice();


        broadphase = new btDbvtBroadphase();

        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);

        solver = new btSequentialImpulseConstraintSolver;

        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

        dynamicsWorld->setGravity(btVector3(0, -9.8, 0));

        playerScore = new scores();
        playerScore->player1Score = 0;
        playerScore->player2Score = 0;

        oldPlayer1 = 0;
        oldPlayer2 = 0;

        follow = new int();
        *follow = 1;

        goalCollisionCallBack = new GoalContactResultCallback(playerScore);
        AICollisionBack =  new AIContactResultCallback(follow);

        return setupWorld();
}

bool Physics::setupWorld()
    {
        BodyObject paddleObject1, paddleObject2, tableObject, puckObject, barrierObject, goalObject1, goalObject2;
        bool status;

        // set object collision filters
        int paddleCollidesWith = COL_PADDLE | COL_TABLE | COL_PUCK | COL_BARRIER;
        int puckCollidesWith = COL_PADDLE | COL_TABLE | COL_GOAL;
        int tableCollidesWith = COL_PADDLE | COL_TABLE | COL_PUCK | COL_BARRIER;
        int barrierCollidesWith = COL_PADDLE | COL_TABLE | COL_BARRIER;
        int goalCollidesWith = COL_PUCK;

        status = paddleObject1.initializeMovingBody(paddleShape, btVector3(0,-1,-7), 8, btVector3(0,0,0), btVector3(2.0f, 2.0f, 2.0f));
        if(!status){return false;}
        dynamicsWorld->addRigidBody(paddleObject1.rigidBody, COL_PADDLE, paddleCollidesWith);
        objects.push_back(paddleObject1);

        status = tableObject.initializeStaticBody("table.obj", btVector3(0,-5,0), btVector3(5.0f, 5.0f, 5.0f));
        if(!status){return false; }
        dynamicsWorld->addRigidBody(tableObject.rigidBody, COL_TABLE, tableCollidesWith);
        objects.push_back(tableObject);

        status = puckObject.initializeMovingBody(puckShape, btVector3(0,-1,0), 2, btVector3(0,0,0), btVector3(1.f, 1.f, 1.f));
        if(!status){return false;}
        dynamicsWorld->addRigidBody(puckObject.rigidBody, COL_PUCK, puckCollidesWith);
        objects.push_back(puckObject);

        status = paddleObject2.initializeMovingBody(paddleShape, btVector3(5,-1,5), 4, btVector3(0,0,0), btVector3(2.f, 2.f, 2.f));
        if(!status){return false;}
        dynamicsWorld->addRigidBody(paddleObject2.rigidBody, COL_PADDLE, paddleCollidesWith);
        objects.push_back(paddleObject2);

        // create center barrier
        barrierObject.initializeBoxObject(btVector3(3,1,1), btVector3(0,0,0), btVector3(5,1,0.1));
        dynamicsWorld->addRigidBody(barrierObject.rigidBody, COL_BARRIER, barrierCollidesWith);
        objects.push_back(barrierObject);

        // create goals
        goalObject1.initializeBoxObject(btVector3(1.2,1,0.5), btVector3(0,0,-14.5), btVector3(2,1,4));
        dynamicsWorld->addRigidBody(goalObject1.rigidBody, COL_GOAL, goalCollidesWith);
        objects.push_back(goalObject1);

        goalObject2.initializeBoxObject(btVector3(1.2,1,0.5), btVector3(0,0,14.5), btVector3(2,1,4));
        dynamicsWorld->addRigidBody(goalObject2.rigidBody, COL_GOAL, goalCollidesWith);
        objects.push_back(goalObject2);

        return true;
    }

void Physics::updateWorld(float dt){

        dynamicsWorld->stepSimulation(dt, 10);

        static btTransform transTable, transPaddle1, transPaddle2, transPuck;
        static int timeTicks = 0;

        if (playerScore->player1Score == 7) {
            paused = true;

        }
        else if (playerScore->player2Score == 7) {
            engine->play2D("../src/audio_files/media/youWin.wav");
            paused = true;

        }
        if(oldPlayer2 == oldPlayer1 && playerScore->player2Score > playerScore->player1Score){

          engine->play2D("../src/audio_files/media/gainedthelead.ogg");

        }
        else if(oldPlayer2 == oldPlayer1 && playerScore->player2Score < playerScore->player1Score){

          engine->play2D("../src/audio_files/media/lost_the_lead.ogg");

          }
        if(oldPlayer1 < playerScore->player1Score)
        {
            objects[2].rigidBody->clearForces();
            btVector3 zeroVector(0,0,0);
            objects[2].rigidBody->setLinearVelocity(zeroVector);
            objects[2].rigidBody->setAngularVelocity(zeroVector);
            objects[2].rigidBody->translate(btVector3(0,-1,0) - transPuck.getOrigin());
            oldPlayer1 = playerScore->player1Score;
        }
        else if(oldPlayer2 < playerScore->player2Score)
        {
            objects[2].rigidBody->clearForces();
            btVector3 zeroVector(0,0,0);
            objects[2].rigidBody->setLinearVelocity(zeroVector);
            objects[2].rigidBody->setAngularVelocity(zeroVector);
            objects[2].rigidBody->translate(btVector3(0,-1,0) - transPuck.getOrigin());
            oldPlayer2 = playerScore->player2Score;
        }

        // paddle 1
        objects[0].rigidBody->getMotionState()->getWorldTransform(transPaddle1);
        btScalar mPaddle_1[16];
        transPaddle1.getOpenGLMatrix(mPaddle_1);
        objects[0].model.model = glm::make_mat4(mPaddle_1);
        objects[0].model.model = glm::scale(objects[0].model.model, glm::vec3(2, 2, 2));

        // table
        objects[1].rigidBody->getMotionState()->getWorldTransform(transTable);
        btScalar mTable[16];
        transTable.getOpenGLMatrix(mTable);
        objects[1].model.model = glm::make_mat4(mTable);
        objects[1].model.model = glm::scale(objects[1].model.model, glm::vec3(5, 5, 5));

        // puck
        objects[2].rigidBody->getMotionState()->getWorldTransform(transPuck);
        btScalar mPuck[16];
        transPuck.getOpenGLMatrix(mPuck);
        objects[2].model.model = glm::make_mat4(mPuck);

        if (!twoPlayer)
        {
            timeTicks++;

            // collision just occurred
            if (*follow == 0)
            {
                timeTicks = 0;
                if (transPaddle1.getOrigin().z() > -11.0f)
                    objects[0].rigidBody->translate((transPaddle1.getOrigin() + btVector3(0,0,-1)) - transPaddle1.getOrigin());
                *follow = 1;
            }
            if (timeTicks >= 50)
                *follow = 1;
            if (transPuck.getOrigin().z() > 0)
            {
                objects[0].rigidBody->translate(btVector3(0,-1,-7) - transPaddle1.getOrigin());
                *follow = 1;
            }
            else if (((objects[2].rigidBody->getLinearVelocity().x() != 0) || (objects[2].rigidBody->getLinearVelocity().z() != 0)) && *follow)
            {
                float time = getInterceptTime(transPuck);

                // compute position of puck at time of intercept
                btVector3 interceptPos = transPuck.getOrigin() + objects[2].rigidBody->getLinearVelocity() * time;

                // move paddle to intercept the puck
                btVector3 direction = interceptPos - transPaddle1.getOrigin();
                direction.normalize();
                float speed = 10.0f;
                objects[0].rigidBody->setLinearVelocity(btVector3( speed * direction.x(), 0.0, speed * direction.z()));
            }

            // check for collision between AI paddle and puck
            dynamicsWorld->contactPairTest(objects[0].rigidBody, objects[2].rigidBody, *AICollisionBack);
        }

        // paddle 2
        objects[3].rigidBody->getMotionState()->getWorldTransform(transPaddle2);
        btScalar mPaddle_2[16];
        transPaddle2.getOpenGLMatrix(mPaddle_2);
        objects[3].model.model = glm::make_mat4(mPaddle_2);
        objects[3].model.model = glm::scale(objects[3].model.model, glm::vec3(2, 2, 2));

        // check for collision with left goal
        dynamicsWorld->contactPairTest(objects[5].rigidBody, objects[2].rigidBody, *goalCollisionCallBack);

        // check for collision with right goal
        dynamicsWorld->contactPairTest(objects[2].rigidBody, objects[6].rigidBody, *goalCollisionCallBack);
}

float Physics::getInterceptTime(btTransform puckTransform)
{
    // get puck position and velocity
    btVector3 position = puckTransform.getOrigin();
    btVector3 velocity = objects[2].rigidBody->getLinearVelocity();

    float pxSquared = position.x() * position.x();
    float pzSquared = position.z() * position.z();
    float vxSquared = velocity.x() * velocity.x();
    float vzSquared = velocity.z() * velocity.z();

    // compute the speed
    float speed = sqrt(vxSquared + vzSquared);
    float speedSquared = speed * speed;

    // use the quadratic formula to compute the time
    float a = vxSquared + vzSquared - speedSquared;
    float b = (2.0f * position.x() * velocity.x()) + (2.0f * position.z() * velocity.z());
    float c = pxSquared + pzSquared;

    float time1 = (-b + sqrt(b*b - 4.0f*a*c))/2.0f*a;
    float time2 = (-b - sqrt(b*b - 4.0f*a*c))/2.0f*a;

    if ((time1 > 0) && (time2 > 0))
    {
        if (time1 < time2)
            return time1;
        else
            return time2;
    }
    if (time1 > 0)
    {
        return time1;
    }
    return time2;
}

void Physics::cleanup() {
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].cleanup(dynamicsWorld);
    }


    engine->drop();
    objects.clear();
    delete playerScore;
    delete follow;
    delete broadphase;
    delete collisionConfiguration;
    delete dispatcher;
    delete solver;
    delete goalCollisionCallBack;
    delete AICollisionBack;
    delete dynamicsWorld;
}
