#include "physicsEngine.h"


bool Physics::initializePhysicsEngine()
{
        paused = false;

        engine = createIrrKlangDevice();

        broadphase = new btDbvtBroadphase();

        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);

        solver = new btSequentialImpulseConstraintSolver;

        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

        dynamicsWorld->setGravity(btVector3(0, -35, 0));

        goalDetected = new int();
        *goalDetected = 0;

        holeDetected = new int();
        *holeDetected = 0;

        holeCollisionBack = new HoleContactResultCallback(holeDetected);
        goalCollisionBack = new GoalContactResultCallback(goalDetected);

        return setupWorld();
}

bool Physics::setupWorld()
    {
        BodyObject tableObject, ballObject, ballObject2, planeObject, goalObject;
        bool status;

        // set object collision filters
        int ballCollidesWith =  COL_TABLE | COL_PLANE | COL_GOAL | COL_BALL;
        int tableCollidesWith = COL_BALL;
        int planeCollidesWith = COL_TABLE | COL_BALL;
        int goalCollidesWith = COL_PLANE | COL_TABLE | COL_BALL;

        tableObject.xAxisRotation = 0.0f;
        tableObject.zAxisRotation = 0.0f;

        status = tableObject.initializeStaticBody(levelFileName, btVector3(0,5,0), btVector3(5.0f, 5.0f, 5.0f));
        if(!status){return false; }

        dynamicsWorld->addRigidBody(tableObject.rigidBody, COL_TABLE, tableCollidesWith);
        objects.push_back(tableObject);

        status = ballObject.initializeSphereObject("ball3.obj", btVector3(ballPos.x, ballPos.y, ballPos.z), 0.1, btVector3(0,0.5,0), btVector3(1.f, 1.f, 1.f), btVector3(1,1,1));
        if(!status){return false; }

        dynamicsWorld->addRigidBody(ballObject.rigidBody, COL_BALL, ballCollidesWith);
        objects.push_back(ballObject);

        // plane to detect ball falling into holes
        planeObject.initializeBoxObject(btVector3(8.0f, 0.5f, 6.0f), btVector3(0,-2,0), btVector3(5.0f, 5.0f, 5.0f));
        dynamicsWorld->addRigidBody(planeObject.rigidBody, COL_PLANE, planeCollidesWith);
        objects.push_back(planeObject);

        // plane to detect a win
        goalObject.initializeBoxObject(btVector3(1.5f, 1.5f, 1.5f), btVector3(goalPos.x, goalPos.y, goalPos.z), btVector3(1.0f, 1.0f, 1.0f));
        dynamicsWorld->addRigidBody(goalObject.rigidBody, COL_GOAL, goalCollidesWith);
        objects.push_back(goalObject);

        if (twoBalls) 
        {
           status = ballObject2.initializeSphereObject("ball3.obj", btVector3(ball2Pos.x, ball2Pos.y, ball2Pos.z), 0.1, btVector3(0,0.5,0), btVector3(1.f, 1.f, 1.f), btVector3(1,1,1));
           if(!status){return false; }

           dynamicsWorld->addRigidBody(ballObject2.rigidBody, COL_BALL, ballCollidesWith);
           objects.push_back(ballObject2); 
       }

        return true;
    }

void Physics::updateWorld(float dt){

        dynamicsWorld->stepSimulation(dt, 10);

        static btTransform transTable, transBall;
        static int timeTicks = 0;

        // table
        objects[0].rigidBody->getMotionState()->getWorldTransform(transTable);
        btScalar mTable[16];
        transTable.getOpenGLMatrix(mTable);
        objects[0].model.model = glm::make_mat4(mTable);
        objects[0].model.model = glm::scale(objects[0].model.model, glm::vec3(5, 5, 5));

        // balls
        objects[1].rigidBody->getMotionState()->getWorldTransform(transBall);
        btScalar mBall[16];
        transBall.getOpenGLMatrix(mBall);
        objects[1].model.model = glm::make_mat4(mBall);

        btVector3 position = transBall.getOrigin();
        ballPos = glm::vec3(mBall[12], 5.0, mBall[14]);
        //cout << ballPos.x << " | "<< ballPos.y << " | "<< ballPos.z << endl;

        if (twoBalls)
        {
            objects[4].rigidBody->getMotionState()->getWorldTransform(transBall);
            transBall.getOpenGLMatrix(mBall);
            objects[4].model.model = glm::make_mat4(mBall);

            btVector3 position = transBall.getOrigin();
            ballPos = glm::vec3(mBall[12], 5.0, mBall[14]);

            // check if the ball 2 fell into a hole or goal
            dynamicsWorld->contactPairTest(objects[4].rigidBody, objects[3].rigidBody, *goalCollisionBack);
            dynamicsWorld->contactPairTest(objects[4].rigidBody, objects[2].rigidBody, *holeCollisionBack);
        }

        // check if the ball fell into a hole or goal
        dynamicsWorld->contactPairTest(objects[1].rigidBody, objects[3].rigidBody, *goalCollisionBack);
        dynamicsWorld->contactPairTest(objects[1].rigidBody, objects[2].rigidBody, *holeCollisionBack);
}

void Physics::cleanup() {
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].cleanup(dynamicsWorld);
    }


    engine->drop();
    objects.clear();
    delete holeDetected;
    delete broadphase;
    delete collisionConfiguration;
    delete dispatcher;
    delete solver;
    delete dynamicsWorld;
    delete holeCollisionBack;
    delete goalCollisionBack;
}
