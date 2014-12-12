/******************** Header Files ********************/
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting

#include <iostream>
#include <chrono>
#include <locale>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include "shader.h"
#include <string.h>
#include <vector>
#include "model.h"
#include "camera.h"
#include "physicsEngine.h"

#include <irrKlang.h>
#include "audio_files/conio.h"
#include <stdlib.h>

using namespace irrklang;

using namespace std;


/******************** Global Variables and Constants ********************/

// Window variables
int window;
int w = 640, h = 480; // Window size

//transform matrices
glm::mat4 view;  // world->eye
glm::mat4 projection; // eye->clip
glm::mat4 model; // eye->clip
glm::mat4 mvp; // eye->clip

// shader varaibles
string textureVertexShaderName;
string textureFragmentShaderName;
GLuint texture_program;

// index 0 is vertex, index 1 is fragment
LightingEngine lightingEngine[2];
int toggleShader = 1;

// Random time things
float getDT();
chrono::time_point<chrono::high_resolution_clock> t1,t2;

// User interaction variables
float speed = 2.0f;
int direction = 1;

vector<colorVertex> circleGeom;
GLuint VB;
glm::mat4 ring; // eye->clip
glm::mat4 ringMvp; // eye->clip

// camera variables
Camera camera;
bool freeCamera = true;
bool cameraZoom = true;
int zoomCount = 0;

// keyboard variables
bool keys[256];

// Mouse Position
float mouseX, mouseY;

// model variables
Model room;

Physics* physics;
bool isLevel1;
static string bestTime, currentTime = "0:00:00";
static long bestTimeMicroseconds;
glm::vec3 level1BallPos(18, 5, 11);
glm::vec3 level2BallPos(-2,5,-11);
glm::vec3 level1GoalPos(-16, 2, 12);
glm::vec3 level2GoalPos(6.5,2,-12.5);

int cameraCount = 5;
bool moveCamera = false;

/******************** Function Declarations ********************/
// GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboardListener(unsigned char key, int x_pos, int y_pos);
void keyboardUpListener(unsigned char key, int x_pos, int y_pos);
void specialKeyHandler(int key, int xx, int yy);
void specialKeyReleaseHandler(int key, int xx, int yy);
void getMousePos(int x, int y);
void checkKeyboard();
void renderBitmapString(float x, float y, float z, void *font, string text);

// Resource management
bool initialize();
void cleanUp();

// Shader Programs
bool createTextureShader();
bool createColorShader();

// Command Line Arguments
bool getCommandlineArgs(int argc, char **argv);

// Menu Functions
void createMenu();
void menuListener(int selection);

// File parse
bool loadData(string fileName);
bool loadRealisticData(string fileName);
void glCircle( unsigned int numPoints, unsigned int radius );

// Song picker
void chooseSong();

/******************** Main Program ********************/
int main(int argc, char **argv)
{
  // get commandline args
  bool validArgs = getCommandlineArgs(argc, argv);
  physics = new Physics();
  physics->engine = createIrrKlangDevice();
  bestTime = "01:00:00";

  physics->engine->play2D("../src/audio_files/media/labyrinth.ogg", true);

  // check to see if args are valid
  if(validArgs)
    {
      // Initialize glut
      glutInit(&argc, argv);
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
      glutInitWindowSize(w, h);

      // Name and create the Window
      window = glutCreateWindow("Assignment 11 - Labyrinth");

      // Now that the window is created the GL context is fully set up
      // Because of that we can now initialize GLEW to prepare work with shaders
      GLenum status = glewInit();
      if( status != GLEW_OK)
        {
          cerr << "[F] GLEW NOT INITIALIZED: ";
          cerr << glewGetErrorString(status) << endl;
          return -1;
        }

      // Set all of the callbacks to GLUT that we need
      glutDisplayFunc(render); // Called when its time to display
      glutReshapeFunc(reshape); // Called if the window is resized
      glutIdleFunc(update); // Called if there is nothing else to do
      glutKeyboardFunc(keyboardListener); // Called if there is keyboard input
      glutKeyboardUpFunc(keyboardUpListener);
      glutSpecialFunc(specialKeyHandler);// Called if a special keyboard key is pressed
      glutSpecialUpFunc(specialKeyReleaseHandler);// Called if a special key is releasedz
      glutPassiveMotionFunc(getMousePos);

      // initialize menu
      createMenu();

      // Initialize all of our resources(shaders, geometry)
      if(initialize())
        {
          t1 = chrono::high_resolution_clock::now();
          glutMainLoop();
        }

      // Clean up after ourselves
      cleanUp();
    }

    // exit program
    return 0;
  }


/******************** Function Implementations ********************/
bool initialize()
  {
    isLevel1 = true;
    physics->twoBalls = false;
    physics->levelFileName = "table1.obj";
    physics->ballPos = level1BallPos;
    physics->ball2Pos = level1BallPos + glm::vec3(0,0,-3);
    physics->goalPos = level1GoalPos;
    lightingEngine[0].initialize("vs_vertex_shading.glsl", "fs_vertex_shading.glsl");
    lightingEngine[1].initialize("vs_fragment_shading.glsl", "fs_fragment_shading.glsl");

    //set initial time
    bestTimeMicroseconds = 60000000;

    // initialize bullet
    if(!physics->initializePhysicsEngine())
      {
        return false;
      }

    // load the room model
    room.loadModel("room.obj");

    // initialize the keys array to false
    for(int i = 0; i < 256; i++)
      {
        keys[i] = false;
      }

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   300.0f); //Distance to the far plane,

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
  }

void renderBitmapString(float x, float y, float z, void *font, string text)
{
  // disable the shader program
  glUseProgram(0);
  glDisable(GL_TEXTURE_2D);

  // preserve and clear projection and model matrices
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, w, 0, h, 1, 0);
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);

  // set text color and position
  glColor3f(0,255,255);
  glRasterPos3f(x, y, z);

  // display each character in the string
  for (string::iterator i = text.begin(); i != text.end(); i++) {
    char c = *i;
    glutBitmapCharacter(font, c);
  }

  // restore projection and model matrices
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
}

string getTimeString(long timeInMicroseconds)
{

  int milliseconds = (int) (timeInMicroseconds / 1000);
  int seconds = (int) (timeInMicroseconds / 1000000);
  int minutes = seconds / 60;
  seconds = seconds % 60;
  milliseconds = milliseconds % 1000;

  string minuteStr = to_string(minutes);
  if (minutes < 10)
    minuteStr = '0' + minuteStr;

  string secondStr = to_string(seconds);
  if (seconds < 10)
    secondStr = '0' + secondStr;

  return(minuteStr + ":" + secondStr + ":" + to_string(milliseconds/10));
}

void render()
  {
    //clear the screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the shader program
    glUseProgram(lightingEngine[toggleShader].shaderProgram);

    mvp = projection * view * physics->objects[0].model.model;
    physics->objects[0].model.renderModel(mvp, physics->objects[0].model.model, lightingEngine[toggleShader], physics->ballPos, glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));

    mvp = projection * view * physics->objects[1].model.model;
    physics->objects[1].model.renderModel(mvp, physics->objects[1].model.model, lightingEngine[toggleShader],physics->ballPos, glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));

    if (physics->twoBalls)
    {
      mvp = projection * view * physics->objects[4].model.model;
      physics->objects[4].model.renderModel(mvp, physics->objects[4].model.model, lightingEngine[toggleShader],physics->ballPos, glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));
    }

    mvp = projection * view * room.model;
    room.renderModel(mvp, room.model, lightingEngine[toggleShader], physics->ballPos, glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));

    // display time
    string str2 = "Time Elapsed: ";
    if (physics->timer.isStarted())
    {
      currentTime = getTimeString(physics->timer.getElapsedTime());
    }
      str2.append(currentTime);
      renderBitmapString((w - 195), 10, 0, GLUT_BITMAP_HELVETICA_18, str2);
    


    // display scores
    string str = "Best Level Time: ";

    str.append(bestTime);

    renderBitmapString(15, 10, 0, GLUT_BITMAP_HELVETICA_18, str);

    glutSwapBuffers();
  }


void update()
  {
    //total time
    float dt = getDT(); // if you have anything moving, use dt.

    room.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -2.5, 0.0));
    room.model = glm::scale(room.model, glm::vec3(4.0f, 5.0f, 4.0f));

    if (!physics->paused)
      physics->updateWorld(dt);


    if(freeCamera)
      {
        checkKeyboard();
      }
      glm::vec3 ViewPoint = camera.Position + camera.ViewDir;

    if (cameraZoom)
    {
      view = glm::lookAt( glm::vec3(camera.Position.x++,camera.Position.y--,camera.Position.z),
        glm::vec3(ViewPoint.x,ViewPoint.y,ViewPoint.z),
        glm::vec3(camera.UpVector.x,camera.UpVector.y,camera.UpVector.z));

      zoomCount++;
      if (zoomCount > 19)
      {
        if(isLevel1)
        {
          camera.Position = glm::vec3(8,30,0);

          camera.ViewDir.x = 10.0f;
          camera.ViewDir.y = -30.0f;
          camera.ViewDir.z = 0.0f;

          camera.UpVector.x = 1.0f;
          camera.UpVector.y = 0.0f;
          camera.UpVector.z = 0.0f;

          camera.RotatedX = camera.RotatedZ = 0.0f;
          camera.RotatedY = M_PI/2;
          cameraZoom = false;
        }
        else
        {
          camera.Position = glm::vec3(-15,30,0);

          camera.ViewDir.x = 10.0f;
          camera.ViewDir.y = -30.0f;
          camera.ViewDir.z = 0.0f;

          camera.UpVector.x = 1.0f;
          camera.UpVector.y = 0.0f;
          camera.UpVector.z = 0.0f;

          camera.RotatedX = camera.RotatedZ = 0.0f;
          camera.RotatedY = M_PI/2;
          cameraZoom = false;
        }
      }
    }
    else
    {
      view = glm::lookAt( glm::vec3(camera.Position.x,camera.Position.y,camera.Position.z),
      glm::vec3(ViewPoint.x,ViewPoint.y,ViewPoint.z),
      glm::vec3(camera.UpVector.x,camera.UpVector.y,camera.UpVector.z));
    }

    if(isLevel1)
    {

        if(physics->ballPos.x > 9.5 && physics->ballPos.x < 10)
        {
          moveCamera = true;
        }
        if(cameraCount > 0 && moveCamera)
        {
              camera.Position = glm::vec3(camera.Position.x - 3,30,0);

              camera.ViewDir.x = 10.0f;
              camera.ViewDir.y = -30.0f;
              camera.ViewDir.z = 0.0f;

              camera.UpVector.x = 1.0f;
              camera.UpVector.y = 0.0f;
              camera.UpVector.z = 0.0f;

              camera.RotatedX = camera.RotatedZ = 0.0f;
              camera.RotatedY = M_PI/2;
              cameraCount --;
        }   

        if(physics->ballPos.x > 0 && physics->ballPos.x < .5 )
        {
              camera.Position = glm::vec3(-15,30,0);

              camera.ViewDir.x = 10.0f;
              camera.ViewDir.y = -30.0f;
              camera.ViewDir.z = 0.0f;

              camera.UpVector.x = 1.0f;
              camera.UpVector.y = 0.0f;
              camera.UpVector.z = 0.0f;

              camera.RotatedX = camera.RotatedZ = 0.0f;
              camera.RotatedY = M_PI/2;
        }  
    }
    else
    {
        if(physics->ballPos.x > -2 && physics->ballPos.x < -1 && physics->ballPos.z > -4.5 && physics->ballPos.z < 2)
        {
          moveCamera = true;
        }

        if(cameraCount > 0 && moveCamera)
        {
              camera.Position = glm::vec3(camera.Position.x + 3,30,0);

              camera.ViewDir.x = 10.0f;
              camera.ViewDir.y = -30.0f;
              camera.ViewDir.z = 0.0f;

              camera.UpVector.x = 1.0f;
              camera.UpVector.y = 0.0f;
              camera.UpVector.z = 0.0f;

              camera.RotatedX = camera.RotatedZ = 0.0f;
              camera.RotatedY = M_PI/2;
              cameraCount --;
        }      
    }

    // check if ball fell into a hole
    if (*(physics->holeDetected) == 1)
    {
      physics->engine->play2D("../src/audio_files/media/holeDrop.ogg");
      sleep(1);
      bool temp = physics->twoBalls;
      physics->cleanup();
      currentTime = "0:00:00";
      delete physics;
      physics = new Physics();

      cameraCount = 5;
      moveCamera = false;
      
      if(isLevel1)
      {
        physics->levelFileName = "table1.obj";
        physics->ballPos = level1BallPos;
        physics->goalPos = level1GoalPos;
        physics->ball2Pos = level1BallPos + glm::vec3(0,0,-3);

        camera.Position = glm::vec3(8,30,0);

        camera.ViewDir.x = 10.0f;
        camera.ViewDir.y = -30.0f;
        camera.ViewDir.z = 0.0f;

        camera.UpVector.x = 1.0f;
        camera.UpVector.y = 0.0f;
        camera.UpVector.z = 0.0f;

        camera.RotatedX = camera.RotatedZ = 0.0f;
        camera.RotatedY = M_PI/2;
      }
      else
      {
        physics->levelFileName = "table2.obj";
        physics->ballPos = level2BallPos;
        physics->goalPos = level2GoalPos;
        physics->ball2Pos = level2BallPos + glm::vec3(-3,0,0);

        camera.Position = glm::vec3(-15,30,0);

        camera.ViewDir.x = 10.0f;
        camera.ViewDir.y = -30.0f;
        camera.ViewDir.z = 0.0f;

        camera.UpVector.x = 1.0f;
        camera.UpVector.y = 0.0f;
        camera.UpVector.z = 0.0f;

        camera.RotatedX = camera.RotatedZ = 0.0f;
        camera.RotatedY = M_PI/2;          
        
      }
      if (temp)
        physics->twoBalls = true;
      else
        physics->twoBalls = false;

      physics->initializePhysicsEngine();

    }

    // check if goal is detected
    if(*(physics->goalDetected) == 1)
    {

      if(physics->timer.getElapsedTime() < bestTimeMicroseconds){
        bestTimeMicroseconds = physics->timer.getElapsedTime();
        bestTime = getTimeString(bestTimeMicroseconds);
      }

      cameraCount = 5;
      moveCamera = false;

      physics->engine->play2D("../src/audio_files/media/youWin.ogg");
      sleep(2);

      bool temp = physics->twoBalls;
      physics->cleanup();
      currentTime = "0:00:00";
      delete physics;
      physics = new Physics();

      if(isLevel1)
      {
        isLevel1 = false;
        bestTimeMicroseconds = 60000000;
      }
      physics->levelFileName = "table2.obj";
      physics->ballPos = level2BallPos;
      physics->goalPos = level2GoalPos;
      physics->ball2Pos = level2BallPos + glm::vec3(-3,0,0);

      camera.Position = glm::vec3(-15,30,0);

      camera.ViewDir.x = 10.0f;
      camera.ViewDir.y = -30.0f;
      camera.ViewDir.z = 0.0f;

      camera.UpVector.x = 1.0f;
      camera.UpVector.y = 0.0f;
      camera.UpVector.z = 0.0f;

      camera.RotatedX = camera.RotatedZ = 0.0f;
      camera.RotatedY = M_PI/2;

      if (temp)
        physics->twoBalls = true;
      else
        physics->twoBalls = false;

      physics->initializePhysicsEngine();
    }

    // Update the state of the scene
    glutPostRedisplay(); // call the display callback
  }


void reshape(int n_w, int n_h)
  {
    w = n_w;
    h = n_h;
    glViewport( 0, 0, w, h);
    projection = glm::infinitePerspective(45.0f, float(w)/float(h), 0.01f);
  }


void checkKeyboard()
  {
      // start time when board tilts the first time
      if (keys['w'] || keys['a'] || keys['s'] || keys['d'])
      {
        if (!physics->timer.isStarted())
        {
          physics->timer.start();
        }

        // board tilt
        btTransform transTable;
        physics->objects[0].rigidBody->getMotionState()->getWorldTransform(transTable);
        if(keys['w'] && transTable.getRotation().z() > -0.02) {
          physics->objects[0].rigidBody->setAngularVelocity(btVector3(physics->objects[0].rigidBody->getAngularVelocity().x(),0, physics->objects[0].rigidBody->getAngularVelocity().z()-0.01));
          physics->objects[2].rigidBody->setAngularVelocity(btVector3(physics->objects[2].rigidBody->getAngularVelocity().x(),0, physics->objects[2].rigidBody->getAngularVelocity().z()-0.01));
          physics->objects[3].rigidBody->setAngularVelocity(btVector3(physics->objects[3].rigidBody->getAngularVelocity().x(),0, physics->objects[3].rigidBody->getAngularVelocity().z()-0.01));

        }
        if(keys['a'] && transTable.getRotation().x() > -0.02) {
          physics->objects[0].rigidBody->setAngularVelocity(btVector3(physics->objects[0].rigidBody->getAngularVelocity().x()-0.01,0, physics->objects[0].rigidBody->getAngularVelocity().z()));
          physics->objects[2].rigidBody->setAngularVelocity(btVector3(physics->objects[2].rigidBody->getAngularVelocity().x()-0.01,0, physics->objects[2].rigidBody->getAngularVelocity().z()));
          physics->objects[3].rigidBody->setAngularVelocity(btVector3(physics->objects[3].rigidBody->getAngularVelocity().x()-0.01,0, physics->objects[3].rigidBody->getAngularVelocity().z()));

        }

        if(keys['s'] && transTable.getRotation().z() < 0.02) {
          physics->objects[0].rigidBody->setAngularVelocity(btVector3(physics->objects[0].rigidBody->getAngularVelocity().x(),0, physics->objects[0].rigidBody->getAngularVelocity().z()+0.01));
          physics->objects[2].rigidBody->setAngularVelocity(btVector3(physics->objects[2].rigidBody->getAngularVelocity().x(),0, physics->objects[2].rigidBody->getAngularVelocity().z()+0.01));
          physics->objects[3].rigidBody->setAngularVelocity(btVector3(physics->objects[3].rigidBody->getAngularVelocity().x(),0, physics->objects[3].rigidBody->getAngularVelocity().z()+0.01));

        }
        if(keys['d'] && transTable.getRotation().x() < 0.02) {
          physics->objects[0].rigidBody->setAngularVelocity(btVector3(physics->objects[0].rigidBody->getAngularVelocity().x()+0.01,0, physics->objects[0].rigidBody->getAngularVelocity().z()));
          physics->objects[2].rigidBody->setAngularVelocity(btVector3(physics->objects[2].rigidBody->getAngularVelocity().x()+0.01,0, physics->objects[2].rigidBody->getAngularVelocity().z()));
          physics->objects[3].rigidBody->setAngularVelocity(btVector3(physics->objects[3].rigidBody->getAngularVelocity().x()+0.01,0, physics->objects[3].rigidBody->getAngularVelocity().z()));

        }
      }
    }


void keyboardListener(unsigned char key, int x_pos, int y_pos)
  {
    // check if key is escape
    if(key == 27)
      {
        glutDestroyWindow(window);
        return;
      }
      
    else if (key == '1')
    {
        // toggle ambient
        lightingEngine[toggleShader].ambientLightOn = !lightingEngine[toggleShader].ambientLightOn;
    }
    else if (key == '2')
    {
        // toggle diffuse
        lightingEngine[toggleShader].diffuseLightOn = !lightingEngine[toggleShader].diffuseLightOn;

    }
    else if (key == '3')
    {
        // toggle specular
        lightingEngine[toggleShader].specularLightOn = !lightingEngine[toggleShader].specularLightOn;
    }
    else if (key == '4')
    {
        // toggle point light
        lightingEngine[toggleShader].pointLightOn = !lightingEngine[toggleShader].pointLightOn;
    }
    else if (key == '5')
    {
        // toggle spot light
        lightingEngine[toggleShader].spotLightOn = !lightingEngine[toggleShader].spotLightOn;
    }

    else if (key == 'z')
    {
        // toggle spot light
        toggleShader = (toggleShader+1)%2 ;
    }

    // assume key pressed is for movement
    else
      {
        keys[key] = true;
        freeCamera = true;
      }

    glutPostRedisplay();

  }

void keyboardUpListener(unsigned char key, int x_pos, int y_pos)
  {
    keys[key] = false;
    glutPostRedisplay();
  }

void specialKeyHandler(int key, int xx, int yy)
{
  switch(key) {
    case GLUT_KEY_LEFT:
      camera.StrafeRight(-2.1);
    break;
    case GLUT_KEY_RIGHT:
      camera.StrafeRight(2.1);
    break;
    case GLUT_KEY_UP:
      camera.MoveUpward(-2.1);
    break;
    case GLUT_KEY_DOWN:
      camera.MoveUpward(2.1);
    break;
  }
}

void specialKeyReleaseHandler(int key, int xx, int yy) {
  switch(key) {
    case GLUT_KEY_LEFT:
    case GLUT_KEY_RIGHT:
    break;
    case GLUT_KEY_UP:
    break;
    case GLUT_KEY_DOWN:
    break;
  }
}


void getMousePos(int x, int y)
{
  mouseX = (float)x;
  mouseY = (float)y;
}

void createMenu()
  {
    // create main menu entries
    glutCreateMenu(menuListener);
    glutAddMenuEntry("Toggle Between One/Two Balls", 1);
    glutAddMenuEntry("Pause Game", 2);
    glutAddMenuEntry("Resume Game", 3);
    glutAddMenuEntry("Restart Game", 4);
    glutAddMenuEntry("Change Level", 5);
    glutAddMenuEntry("Quit", 6);

    // set right mouse click to open menu
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void menuListener(int selection)
  {
    bool temp;

    // check which menu option was selected
    switch(selection)
      {
        // toggle between one and two balls
        case 1:
          temp = physics->twoBalls;
          physics->cleanup();
          currentTime = "0:00:00";
          delete physics;
          physics = new Physics();
          bestTime = "01:00:00";
          bestTimeMicroseconds = 6000000000;
          if (temp)
            physics->twoBalls = false;
          else
            physics->twoBalls = true;

      cameraCount = 5;
      moveCamera = false;

          if(isLevel1)
          {
            physics->levelFileName = "table1.obj";
            physics->ballPos = level1BallPos;
            physics->goalPos = level1GoalPos;
            physics->ball2Pos = level1BallPos + glm::vec3(0,0,-3);
            camera.Position = glm::vec3(8,30,0);

            camera.ViewDir.x = 10.0f;
            camera.ViewDir.y = -30.0f;
            camera.ViewDir.z = 0.0f;

            camera.UpVector.x = 1.0f;
            camera.UpVector.y = 0.0f;
            camera.UpVector.z = 0.0f;

            camera.RotatedX = camera.RotatedZ = 0.0f;
            camera.RotatedY = M_PI/2;
          }
          else
          {
            physics->levelFileName = "table2.obj";
            physics->ballPos = level2BallPos;
            physics->goalPos = level2GoalPos;
            physics->ball2Pos = level2BallPos + glm::vec3(-3,0,0);
            camera.Position = glm::vec3(-15,30,0);

            camera.ViewDir.x = 10.0f;
            camera.ViewDir.y = -30.0f;
            camera.ViewDir.z = 0.0f;

            camera.UpVector.x = 1.0f;
            camera.UpVector.y = 0.0f;
            camera.UpVector.z = 0.0f;

            camera.RotatedX = camera.RotatedZ = 0.0f;
            camera.RotatedY = M_PI/2;    
          }

          physics->initializePhysicsEngine();
        break;

        // pause
        case 2:
            physics->engine->setAllSoundsPaused(1);
            physics->timer.pause();
            physics->paused = true;

        break;

        // resume
        case 3:
            physics->engine->setAllSoundsPaused(0);
            physics->timer.resume();
            physics->paused = false;
        break;

        // restart
        case 4:
            temp = physics->twoBalls;
            currentTime = "0:00:00";
            delete physics;
            physics = new Physics();

            cameraCount = 5;
            moveCamera = false;

            if(isLevel1)
            {
              physics->levelFileName = "table1.obj";
              physics->ballPos = level1BallPos;
              physics->goalPos = level1GoalPos;
              physics->ball2Pos = level1BallPos + glm::vec3(0,0,-3);

              camera.Position = glm::vec3(8,30,0);

              camera.ViewDir.x = 10.0f;
              camera.ViewDir.y = -30.0f;
              camera.ViewDir.z = 0.0f;

              camera.UpVector.x = 1.0f;
              camera.UpVector.y = 0.0f;
              camera.UpVector.z = 0.0f;

              camera.RotatedX = camera.RotatedZ = 0.0f;
              camera.RotatedY = M_PI/2;
            }
            else
            {
              physics->levelFileName = "table2.obj";
              physics->ballPos = level2BallPos;
              physics->goalPos = level2GoalPos;
              physics->ball2Pos = level2BallPos + glm::vec3(-3,0,0);

              camera.Position = glm::vec3(-15,30,0);

              camera.ViewDir.x = 10.0f;
              camera.ViewDir.y = -30.0f;
              camera.ViewDir.z = 0.0f;

              camera.UpVector.x = 1.0f;
              camera.UpVector.y = 0.0f;
              camera.UpVector.z = 0.0f;

              camera.RotatedX = camera.RotatedZ = 0.0f;
              camera.RotatedY = M_PI/2;    
            }
            if (temp)
              physics->twoBalls = true;
            else
              physics->twoBalls = false;

            physics->initializePhysicsEngine();
        break;

        // change level
        case 5:
          temp = physics->twoBalls;
          currentTime = "0:00:00";
          physics->cleanup();
          delete physics;
          physics = new Physics();
          bestTime = "01:00:00";
          bestTimeMicroseconds = 6000000000;

          cameraCount = 5;
          moveCamera = false;

          if (isLevel1)
          {
            isLevel1 = false;
            physics->levelFileName = "table2.obj";
            physics->ballPos = level2BallPos;
            physics->goalPos = level2GoalPos;
            physics->ball2Pos = level2BallPos + glm::vec3(-3,0,0);

            camera.Position = glm::vec3(-15,30,0);

            camera.ViewDir.x = 10.0f;
            camera.ViewDir.y = -30.0f;
            camera.ViewDir.z = 0.0f;

            camera.UpVector.x = 1.0f;
            camera.UpVector.y = 0.0f;
            camera.UpVector.z = 0.0f;

            camera.RotatedX = camera.RotatedZ = 0.0f;
            camera.RotatedY = M_PI/2;    

          }
          else
          {
            isLevel1 = true;
            physics->levelFileName = "table1.obj";
            physics->ballPos = level1BallPos;
            physics->goalPos = level1GoalPos;
            physics->ball2Pos = level1BallPos + glm::vec3(0,0,-3);

            camera.Position = glm::vec3(8,30,0);

            camera.ViewDir.x = 10.0f;
            camera.ViewDir.y = -30.0f;
            camera.ViewDir.z = 0.0f;

            camera.UpVector.x = 1.0f;
            camera.UpVector.y = 0.0f;
            camera.UpVector.z = 0.0f;

            camera.RotatedX = camera.RotatedZ = 0.0f;
            camera.RotatedY = M_PI/2;
          }
          if (temp)
            physics->twoBalls = true;
          else
            physics->twoBalls = false;

          physics->initializePhysicsEngine();

        break;

        // exit the program
        case 6:
            glutDestroyWindow(window);
            return;
        break;

      }
    glutPostRedisplay();
  }

void cleanUp()
  {
    physics->engine->drop();
    physics->cleanup();
  }


//returns the time delta
float getDT()
  {
    float ret;
    t2 = chrono::high_resolution_clock::now();
    ret = chrono::duration_cast< chrono::duration<float> >(t2-t1).count();
    t1 = chrono::high_resolution_clock::now();
    return ret;
  }


bool getCommandlineArgs(int argc, char **argv)
{
  // if there are less then 5 args then set the shaders to default values
  if (argc < 6)
    {
      if (textureVertexShaderName.empty())
        textureVertexShaderName = "texture_vertex_shader.glsl";
      if (textureFragmentShaderName.empty())
        textureFragmentShaderName = "texture_fragment_shader.glsl";

      return true;
    }

  // get the args
  for(int i = 1; i < argc; i++)
  {
    // check for vertex shader filename
    if(string(argv[i]) == "-v")
    {
      // check if we are not at the end of the array
      if(i + 1 < argc && string(argv[i+1]) != "-f")
        {
          textureVertexShaderName = argv[i+1];
        }
      else
        {
          // print error
          cerr << "-v requires one argument." << endl;
          return false;
        }
    }

    // check for fragment shader filename
    else if(string(argv[i]) == "-f")
      {
        // check if we are not at the end of the array
        if(i + 1 < argc && string(argv[i+1]) != "-v")
          {
            textureFragmentShaderName = argv[i+1];
          }
        else
          {
            // print error
            cerr << "-f requires one argument." << endl;
            return false;
          }
      }
    else
      {
        cerr << "Usage: " << argv[0] << " -v VERTEX_SHADER_FILENAME -f FRAGMENT_SHADER_FILENAME" << endl;
        return false;
      }
    i++;
  }

  return true;
}
