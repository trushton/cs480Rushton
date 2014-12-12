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
Camera camera;
Physics* physics;


// uniform locations
GLint texture_loc_mvpmat; // Location of the modelviewprojection matrix in the shader

// attribute locations
GLint texture_loc_position;
GLint loc_texture_coord;

GLint loc_color;
GLint color_loc_position;
GLuint color_program;
GLint color_loc_mvpmat; // Location of the modelviewprojection matrix in the shader

//transform matrices
glm::mat4 view;  // world->eye
glm::mat4 projection; // eye->clip
glm::mat4 model; // eye->clip
glm::mat4 mvp; // eye->clip

// shader varaibles
string textureVertexShaderName;
string textureFragmentShaderName;
GLuint texture_program;

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
bool freeCamera = true;
bool cameraZoom = true;
int zoomCount = 0;

// keyboard variables
bool keys[256];

// Mouse Position
float mouseX, mouseY;

// Room model variables
int theme = 1;
Model room;

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
  physics->engine->play2D("../src/audio_files/media/Good_Evening.ogg");
  sleep(4);

  // check to see if args are valid
  if(validArgs)
    {
      // Initialize glut
      glutInit(&argc, argv);
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
      glutInitWindowSize(w, h);

      // Name and create the Window
      window = glutCreateWindow("Assignment 8 - Air Hockey");

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
    physics->paddleShape = "round_paddle.obj";
    physics->puckShape = "red_round_puck.obj";

    // initialize bullet
    if(!physics->initializePhysicsEngine())
      {
        return false;
      }

    physics->engine->play2D("../src/audio_files/media/13_Kakumei.ogg", true);


    // load the room model
    room.loadModel("room.obj");

    // initialize the keys array to false
    for(int i = 0; i < 256; i++)
      {
        keys[i] = false;
      }

    // create the texture shader
    if(!createTextureShader())
      {
        return false;
      }

    // create the color shader
    if(!createColorShader())
      {
        return false;
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


bool createTextureShader()
  {
    // set up the texture vertex shader
    shader texture_vertex_shader(GL_VERTEX_SHADER);
    if(!texture_vertex_shader.initialize(textureVertexShaderName))
      {
        return false;
      }

    // set up the texture fragment shader
    shader texture_fragment_shader(GL_FRAGMENT_SHADER);
    if(!texture_fragment_shader.initialize(textureFragmentShaderName))
      {
        return false;
      }

    // link the texture shader program
    texture_program = glCreateProgram();
    glAttachShader(texture_program, texture_vertex_shader.getShader());
    glAttachShader(texture_program, texture_fragment_shader.getShader());
    glLinkProgram(texture_program);

    // check if everything linked ok
    GLint texture_shader_status;
    glGetProgramiv(texture_program, GL_LINK_STATUS, &texture_shader_status);
    if(!texture_shader_status)
      {
        cerr << "[F] THE TEXTURE SHADER PROGRAM FAILED TO LINK" << endl;
        return false;
      }

    // set up the vertex position attribute
    texture_loc_position = glGetAttribLocation(texture_program, const_cast<const char*>("v_position"));
    if(texture_loc_position == -1)
      {
        cerr << "[F] POSITION NOT FOUND" << endl;
        return false;
      }

    // set up the vertex uv coordinate attribute
    loc_texture_coord = glGetAttribLocation(texture_program, const_cast<const char*>("v_texture"));
      if(loc_texture_coord == -1)
      {
        cerr << "[F] V_COLOR NOT FOUND" << endl;
        return false;
      }

    // set up the MVP matrix attribute
    texture_loc_mvpmat = glGetUniformLocation(texture_program, const_cast<const char*>("mvpMatrix"));
    if(texture_loc_mvpmat == -1)
      {
        cerr << "[F] MVPMATRIX NOT FOUND" << endl;
        return false;
      }

    // return
    return true;
  }


bool createColorShader()
  {
    // set up the color vertex shader
    shader color_vertex_shader(GL_VERTEX_SHADER);
    if(!color_vertex_shader.initialize("color_vertex_shader.glsl"))
      {
        return false;
      }

    // set up the color fragment shader
    shader color_fragment_shader(GL_FRAGMENT_SHADER);
    if(!color_fragment_shader.initialize("color_fragment_shader.glsl"))
      {
        return false;
      }

    // link the color shader program
    color_program = glCreateProgram();
    glAttachShader(color_program, color_vertex_shader.getShader());
    glAttachShader(color_program, color_fragment_shader.getShader());
    glLinkProgram(color_program);

    // check if everything linked ok
    GLint color_shader_status;
    glGetProgramiv(color_program, GL_LINK_STATUS, &color_shader_status);
    if(!color_shader_status)
      {
        cerr << "[F] THE COLOR SHADER PROGRAM FAILED TO LINK" << endl;
        return false;
      }

    // set up the vertex position attribute
    color_loc_position = glGetAttribLocation(color_program, const_cast<const char*>("v_position"));
    if(color_loc_position == -1)
      {
        cerr << "[F] POSITION NOT FOUND" << endl;
        return false;
      }

    // set up the vertex color attribute
    loc_color = glGetAttribLocation(color_program, const_cast<const char*>("v_color"));
    if(loc_color == -1)
      {
        cerr << "[F] V_COLOR NOT FOUND" << endl;
        return false;
      }

    // set up the MVP matrix attribute
    color_loc_mvpmat = glGetUniformLocation(color_program, const_cast<const char*>("mvpMatrix"));
    if(color_loc_mvpmat == -1)
      {
        cerr << "[F] MVPMATRIX NOT FOUND" << endl;
        return false;
      }

    // return
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
  if (theme) {
    glColor3f(0,254,0);
  }
  else {
    glColor3f(0,206,209);
  }
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

void render()
  {
    //clear the screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the shader program
    glUseProgram(texture_program);

    mvp = projection * view * physics->objects[0].model.model;
    glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    physics->objects[0].model.renderModel(texture_loc_position, loc_texture_coord);

    mvp = projection * view * physics->objects[1].model.model;
    glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    physics->objects[1].model.renderModel(texture_loc_position, loc_texture_coord);

    mvp = projection * view * physics->objects[2].model.model;
    glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    physics->objects[2].model.renderModel(texture_loc_position, loc_texture_coord);

    mvp = projection * view * physics->objects[3].model.model;
    glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    physics->objects[3].model.renderModel(texture_loc_position, loc_texture_coord);

    mvp = projection * view * room.model;
    glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    room.renderModel(texture_loc_position, loc_texture_coord);

    // display scores
    string score = to_string(physics->playerScore->player1Score);
    string str = "Player 1 Score: " + score;
    renderBitmapString(15, 10, 0, GLUT_BITMAP_HELVETICA_18, str);

    score = to_string(physics->playerScore->player2Score);
    str = "Player 2 Score: " + score;
    renderBitmapString((w - 155), 10, 0, GLUT_BITMAP_HELVETICA_18, str);

    glutSwapBuffers();
  }


void update()
  {
    //total time
    float dt = getDT(); // if you have anything moving, use dt.

    if(theme){
      room.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -2.5, 0.0));
      room.model = glm::scale(room.model, glm::vec3(4.0f, 5.0f, 4.0f));
    }
    else{
      room.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -2.5, 0.0));
      room.model = glm::scale(room.model, glm::vec3(3.0f, 5.0f, 3.0f));
    }

    if (!physics->paused)
      physics->updateWorld(dt);


    if(freeCamera)
      {
        checkKeyboard();
      }

    if (cameraZoom)
    {
      glm::vec3 ViewPoint = camera.Position + camera.ViewDir;
      view = glm::lookAt( glm::vec3(camera.Position.x++,camera.Position.y--,camera.Position.z),
        glm::vec3(ViewPoint.x,ViewPoint.y,ViewPoint.z),
        glm::vec3(camera.UpVector.x,camera.UpVector.y,camera.UpVector.z));

      zoomCount++;
      if (zoomCount > 19)
        cameraZoom = false;
    }
    else
    {
      glm::vec3 ViewPoint = camera.Position + camera.ViewDir;
      view = glm::lookAt( glm::vec3(camera.Position.x,camera.Position.y,camera.Position.z),
        glm::vec3(ViewPoint.x,ViewPoint.y,ViewPoint.z),
        glm::vec3(camera.UpVector.x,camera.UpVector.y,camera.UpVector.z));
    }

    // Update the state of the scene
    glutPostRedisplay(); // call the display callback
  }


void reshape(int n_w, int n_h)
  {
    w = n_w;
    h = n_h;
      //Change the viewport to be correct
    glViewport( 0, 0, w, h);
      //Update the projection matrix as well
      //See the init function for an explaination
    //projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 1200.0f);
    projection = glm::infinitePerspective(45.0f, float(w)/float(h), 0.01f);
  }


void checkKeyboard()
  {
    // player 1 controls
    if (physics->twoPlayer)
      {
        if(keys['d']) {
          physics->objects[0].rigidBody->setLinearVelocity(btVector3( physics->objects[0].rigidBody->getLinearVelocity().x(), 0.0, physics->objects[0].rigidBody->getLinearVelocity().z()+1.0 ));
        }
        if(keys['a']) {
          physics->objects[0].rigidBody->setLinearVelocity(btVector3( physics->objects[0].rigidBody->getLinearVelocity().x(), 0.0, physics->objects[0].rigidBody->getLinearVelocity().z()-1.0 ));
        }
        if(keys['s']) {
          physics->objects[0].rigidBody->setLinearVelocity(btVector3( physics->objects[0].rigidBody->getLinearVelocity().x()-1.0, 0.0, physics->objects[0].rigidBody->getLinearVelocity().z() ));
        }
        if(keys['w']) {
          physics->objects[0].rigidBody->setLinearVelocity(btVector3( physics->objects[0].rigidBody->getLinearVelocity().x()+1.0, 0.0, physics->objects[0].rigidBody->getLinearVelocity().z() ));
        }
      }

    // player 2 controls  
    if(keys['l']) {
        physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x(), 0.0, physics->objects[3].rigidBody->getLinearVelocity().z()+1.0 ));
      }
    if(keys['j']) {
        physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x(), 0.0, physics->objects[3].rigidBody->getLinearVelocity().z()-1.0 ));
      }
    if(keys['k']) {
        physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x()-1.0, 0.0, physics->objects[3].rigidBody->getLinearVelocity().z() ));
      }
    if(keys['i']) {
        physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x()+1.0, 0.0, physics->objects[3].rigidBody->getLinearVelocity().z() ));
    }
  }


void keyboardListener(unsigned char key, int x_pos, int y_pos)
  {
    // check if key is escape
    if(key == 27)
      {
        physics->engine->setAllSoundsPaused(1);
        physics->engine->play2D("../src/audio_files/media/Game.ogg");
        sleep(2);
        glutDestroyWindow(window);
        return;
      }

    // check if key is the 1-9 keys
    if(key >= '1' && key <= '9')
      {
        if(key == '1')
        {
          camera.Position = glm::vec3(0,20,0);

          camera.ViewDir.x = 0.0f;
          camera.ViewDir.y = -1.0f;
          camera.ViewDir.z = 0.0f;

          camera.UpVector.x = 1.0f;
          camera.UpVector.y = 0.0f;
          camera.UpVector.z = 0.0f;

          camera.RotatedX = camera.RotatedZ = 0.0f;
          camera.RotatedY = M_PI/2;
        }
        else if(key == '2')
        {
          camera.Position = glm::vec3(-15,14,0);

          camera.ViewDir.x = 0.70f;
          camera.ViewDir.y = -0.70f;
          camera.ViewDir.z = 0.0f;

          camera.UpVector.x = 0.70f;
          camera.UpVector.y = 0.70f;
          camera.UpVector.z = 0.0f;


          camera.RotatedX = camera.RotatedZ = 0.0f;
          camera.RotatedY = M_PI/2;
        }
        else if(key == '3')
        {
          camera.Position = glm::vec3(0,7.5,-20);

          camera.ViewDir.x = 0.0f;
          camera.ViewDir.y = -0.60f;
          camera.ViewDir.z = 0.80f;

          camera.UpVector.x = 0.0f;
          camera.UpVector.y = 0.8f;
          camera.UpVector.z = -0.60f;


          camera.RotatedX = camera.RotatedZ = 0.0f;
          camera.RotatedY = M_PI/2;
        }
        else if(key == '4')
        {
          camera.Position = glm::vec3(0,7.5,20);

          camera.ViewDir.x = 0.0f;
          camera.ViewDir.y = -0.60f;
          camera.ViewDir.z = -0.80f;

          camera.UpVector.x = 0.0f;
          camera.UpVector.y = 0.8f;
          camera.UpVector.z = -0.60f;


          camera.RotatedX = camera.RotatedZ = 0.0f;
          camera.RotatedY = M_PI/2;
        }
        freeCamera = false;
      }

    // check if key is the minus or =/+ button
    else if(key == '-' || key == '=')
      {
        if(key == '-')
          {
            camera.movementSpeed -= 0.2f;
            if(camera.movementSpeed <= 0.0f)
              camera.movementSpeed = 0.2f;
          }
        else
          {
            camera.movementSpeed += 0.2f;
          }
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
      camera.MoveForward(-2.1);
    break;
    case GLUT_KEY_DOWN:
      camera.MoveForward(2.1);
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


void getMousePos(int x, int y){

  if(mouseX < x)
  {
      physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x(), 0.0, physics->objects[3].rigidBody->getLinearVelocity().z()+0.50 ));
  }
  if(mouseX > x)
  {
      physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x(), 0.0, physics->objects[3].rigidBody->getLinearVelocity().z()-0.50 ));
  }
  if(mouseY < y)
  {
      physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x()-0.50, 0.0, physics->objects[3].rigidBody->getLinearVelocity().z() ));
  }
  if(mouseY > y)
  {
      physics->objects[3].rigidBody->setLinearVelocity(btVector3( physics->objects[3].rigidBody->getLinearVelocity().x()+0.50, 0.0, physics->objects[3].rigidBody->getLinearVelocity().z() ));
  }

  mouseX = (float)x;
  mouseY = (float)y;
}

void createMenu()
  {
    // create color submenu entries
    int submenuPaddleSelector = glutCreateMenu(menuListener);
    glutAddMenuEntry("Circle",5);
    glutAddMenuEntry("Square",6);
    glutAddMenuEntry("Triangle",7);

    // create color submenu entries
    int submenuPuckSelector = glutCreateMenu(menuListener);
    glutAddMenuEntry("Circle",8);
    glutAddMenuEntry("Square",9);
    glutAddMenuEntry("Triangle",10);

    // create main menu entries
    glutCreateMenu(menuListener);
    glutAddMenuEntry("Toggle AI/Two Player", 1);
    glutAddMenuEntry("Pause Game", 2);
    glutAddMenuEntry("Resume Game", 3);
    glutAddMenuEntry("Restart Game", 4);
    glutAddSubMenu("Change Puck Shape", submenuPaddleSelector);
    glutAddSubMenu("Change Paddle Shape", submenuPuckSelector);
    glutAddMenuEntry("Change Theme", 11);
    glutAddMenuEntry("Quit", 12);

    // set right mouse click to open menu
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void menuListener(int selection)
  {
    string temp, temp2;

    // check which menu option was selected
    switch(selection)
      {
        // toggle between AI and 2 player
        case 1:
            if(physics->twoPlayer)
              physics->twoPlayer = false;
            else
              physics->twoPlayer = true;
        break;

        // pause
        case 2:
            physics->engine->setAllSoundsPaused(1);
            physics->paused = true;

        break;

        // resume
        case 3:
            physics->engine->setAllSoundsPaused(0);
            physics->paused = false;
        break;

        // restart
        case 4:
            temp = physics->paddleShape;
            temp2 = physics->puckShape;
            physics->cleanup();
            delete physics;
            physics = new Physics();
            physics->paddleShape = temp;
            physics->puckShape = temp2;
            physics->initializePhysicsEngine();
            chooseSong();
            zoomCount = 0;
            camera.Position.x = -35.0f;
            camera.Position.y = 35.0f;
            camera.Position.z = 0.0f;
            camera.ViewDir.x = 0.7f;
            camera.ViewDir.y = -0.7f;
            camera.ViewDir.z = -0.00f;
            camera.UpVector.x = 0.70f;
            camera.UpVector.y = 0.70f;
            camera.UpVector.z = 0.0f;
            cameraZoom = true;

        break;

        // change puck
        case 5:
          temp = physics->paddleShape;
          physics->cleanup();
          delete physics;
          physics = new Physics();
          physics->puckShape = "red_round_puck.obj";
          physics->paddleShape = temp;
          physics->initializePhysicsEngine();
          chooseSong();

        break;

        case 6:
          temp = physics->paddleShape;
          physics->cleanup();
          delete physics;
          physics = new Physics();
          physics->puckShape = "square_puck.obj";
          physics->paddleShape = temp;
          physics->initializePhysicsEngine();
          chooseSong();

        break;

        case 7:
          temp = physics->paddleShape;
          physics->cleanup();
          delete physics;
          physics = new Physics();
          physics->puckShape = "triangle_puck.obj";
          physics->paddleShape = temp;
          physics->initializePhysicsEngine();
          chooseSong();

        break;

        // change paddle
        case 8:
          temp = physics->puckShape;
          physics->cleanup();
          delete physics;
          physics = new Physics();
          physics->puckShape = temp;
          physics->paddleShape = "round_paddle.obj";
          physics->initializePhysicsEngine();
          chooseSong();

        break;

        case 9:
          temp = physics->puckShape;
          physics->cleanup();
          delete physics;
          physics = new Physics();
          physics->puckShape = temp;
          physics->paddleShape = "square_paddle.obj";
          physics->initializePhysicsEngine();
          chooseSong();

        break;

        case 10:
          temp = physics->puckShape;
          physics->cleanup();
          delete physics;
          physics = new Physics();
          physics->puckShape = temp;
          physics->paddleShape = "triangle_paddle.obj";
          physics->initializePhysicsEngine();
          chooseSong();

        break;

        // change theme
        case 11:

            theme ^= 1;
            room.deleteModel();
            physics->engine->setAllSoundsPaused(1);

            if(!theme){
              room.loadModel("beach_room.obj");
            }
            else{
              room.loadModel("room.obj");
            }
            chooseSong();
            zoomCount = 0;
            camera.Position.x = -35.0f;
            camera.Position.y = 35.0f;
            camera.Position.z = 0.0f;
            camera.ViewDir.x = 0.7f;
            camera.ViewDir.y = -0.7f;
            camera.ViewDir.z = -0.00f;
            camera.UpVector.x = 0.70f;
            camera.UpVector.y = 0.70f;
            camera.UpVector.z = 0.0f;
            cameraZoom = true;
        break;
        // exit the program
        case 12:
            physics->engine->setAllSoundsPaused(1);
            physics->engine->play2D("../src/audio_files/media/Game.ogg");
            sleep(2);
            glutDestroyWindow(window);
            return;
        break;

      }
    glutPostRedisplay();
  }

void chooseSong()
{
  if(theme)
  {
    physics->engine->play2D("../src/audio_files/media/13_Kakumei.ogg", true);
  }
  else
  {
    physics->engine->play2D("../src/audio_files/media/surfin.ogg", true);
  }
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
