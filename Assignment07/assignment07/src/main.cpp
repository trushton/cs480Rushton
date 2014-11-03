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

#include "shader.cpp"
#include <string.h>
#include <vector>
#include "model.h"
#include "camera.cpp"
#include "planetStruct.h"

using namespace std;


/******************** Global Variables and Constants ********************/

// Window variables
int window;
int w = 640, h = 480; // Window size
vector<Planet> planetList;
Model skyBox;
Camera camera;

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
bool rotationFlag = true;
bool actualScale = true;
bool drawOrbits = true;

vector<colorVertex> circleGeom;
GLuint VB;
glm::mat4 ring; // eye->clip
glm::mat4 ringMvp; // eye->clip

// camera variables
int planetNum = -1;
bool freeCamera = true;
float yOffest = 0.0f;
float zoom = 1.0f;

// keyboard variables
bool keys[256];

/******************** Function Declarations ********************/
// GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboardListener(unsigned char key, int x_pos, int y_pos);
void keyboardUpListener(unsigned char key, int x_pos, int y_pos);

void checkKeyboard();

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

/******************** Main Program ********************/
int main(int argc, char **argv)
{
  // get commandline args
  bool validArgs = getCommandlineArgs(argc, argv);

  // check to see if args are valid
  if(validArgs)
    {
      // Initialize glut
      glutInit(&argc, argv);
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
      glutInitWindowSize(w, h);

      // Name and create the Window
      window = glutCreateWindow("Assignment 7 - Solar System");

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
    // load all the planet data
    if(!loadRealisticData("solar_system_data_actual.txt"))
      {
        return false;
      }

    // load all the planet models
    for(unsigned int i = 0; i < planetList.size(); i++)
      {
        planetList[i].object.loadModel(planetList[i].name + ".obj");

        for (unsigned j = 0; j < planetList[i].satellites.size(); j++)
        {
          planetList[i].satellites[j].moon.loadModel("moon.obj");
        }
      }

    // create vertices for the oribit paths
    glCircle( 360, 1 );
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * circleGeom.size(), &circleGeom[0], GL_STATIC_DRAW);

    // load the skybox
    skyBox.loadModel("skybox.obj");

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

void glCircle( unsigned int numPoints, unsigned int radius )
{
    colorVertex v;
    float angle;

    // circle is about the y-axis
    for (unsigned int i = 0; i <= numPoints; i++)
    {
        angle = i * ((2.0f * M_PI) / numPoints);

        // set xyz and color values
        v.position[0] = radius*cos(angle);
        v.position[1] = 1;
        v.position[2] = radius*sin(angle);
        v.color[0] = 1.0f;
        v.color[1] = 1.0f;
        v.color[2] = 1.0f;

        circleGeom.push_back(v);
    }
}

void render()
  {
    //clear the screen
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the shader program
    glUseProgram(texture_program);

    // loop and render all the planets
    for(unsigned int i = 0; i < (planetList.size()); i++)
      {
        planetList[i].object.mvp = projection * view * planetList[i].object.model;
        glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(planetList[i].object.mvp));

        planetList[i].object.renderModel(texture_loc_position, loc_texture_coord);

        // loop and render all the moons for each planet
        for(unsigned j = 0; j < planetList[i].satellites.size(); j++)
          {
            planetList[i].satellites[j].moon.mvp = projection * view * planetList[i].satellites[j].moon.model;
            glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(planetList[i].satellites[j].moon.mvp));
            planetList[i].satellites[j].moon.renderModel(texture_loc_position, loc_texture_coord);
          }
      }

    mvp = projection * view * model;
    glUniformMatrix4fv(texture_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    skyBox.renderModel(texture_loc_position, loc_texture_coord);

    // draw the planet orbit paths
    if(drawOrbits)
      {
        // set up the Vertex Buffer Object so it can be drawn
        glEnableVertexAttribArray(color_loc_position);
        glEnableVertexAttribArray(loc_color);


        glBindBuffer(GL_ARRAY_BUFFER, VB);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colorVertex) * circleGeom.size(), &circleGeom[0], GL_STATIC_DRAW);

        // set pointers into the vbo for each of the attributes(position and color)
        glVertexAttribPointer(color_loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(colorVertex), 0);//offset
        glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(colorVertex), (void*)offsetof(colorVertex, color));

        for(unsigned i = 0; i < planetList.size() - 1; i++)
          {
            glm::mat4 temp = glm::scale(ring, glm::vec3(planetList[i].orbitRadius, 0, planetList[i].orbitRadius));
            ringMvp = projection * view * temp;

            glUseProgram(color_program);

            // upload the matrix to the shader
            glUniformMatrix4fv(color_loc_mvpmat, 1, GL_FALSE, glm::value_ptr(ringMvp));

            glDrawArrays(GL_LINE_LOOP, 0, circleGeom.size());
          }

        glDisableVertexAttribArray(color_loc_position);
        glDisableVertexAttribArray(loc_color);
      }

    glutSwapBuffers();
  }


void update()
  {
    //total time
    static float angle = 0.0;
    float dt = getDT(); // if you have anything moving, use dt.

    // move through 90 degrees a second
    if(rotationFlag)
      angle += (dt/2.0f) * M_PI/2 * direction;

    for (unsigned int i = 0; i < planetList.size(); i++)
      {
        // update planet position in orbit
        planetList[i].object.model = glm::translate(glm::mat4(1.0f), glm::vec3(planetList[i].orbitRadius * sin((angle + planetList[i].orbitAngle)/planetList[i].orbitSpeed), 0.0,
                                                      planetList[i].orbitRadius * cos((angle + planetList[i].orbitAngle)/planetList[i].orbitSpeed )));

        // scale planet according to its diameter
        planetList[i].object.model = glm::scale(planetList[i].object.model, glm::vec3(planetList[i].diameter, planetList[i].diameter, planetList[i].diameter));

        // update moons
        for (unsigned j = 0; j < planetList[i].satellites.size(); j++)
          {
            // update planet position
            if(i == 2 || i == 3)
              {
              planetList[i].satellites[j].moon.model = glm::translate(planetList[i].object.model, glm::vec3((planetList[i].diameter+planetList[i].satellites[j].distFromPlanet) * sin(angle*planetList[i].satellites[j].orbitSpeed), 0.0,
                                                                     (planetList[i].diameter + planetList[i].satellites[j].distFromPlanet) * cos(angle*planetList[i].satellites[j].orbitSpeed)));
              }
            else
              {
                if (actualScale) {
                planetList[i].satellites[j].moon.model = glm::translate(planetList[i].object.model, glm::vec3(((planetList[i].diameter/2.0f) + planetList[i].satellites[j].distFromPlanet) * sin(angle*planetList[i].satellites[j].orbitSpeed), 0.0,
                                                                       ((planetList[i].diameter/2.0f) + planetList[i].satellites[j].distFromPlanet) * cos(angle*planetList[i].satellites[j].orbitSpeed)));
                }
                else {
                  planetList[i].satellites[j].moon.model = glm::translate(planetList[i].object.model, glm::vec3((planetList[i].diameter * planetList[i].satellites[j].distFromPlanet) * sin(angle*planetList[i].satellites[j].orbitSpeed), 0.0,
                                                                       (planetList[i].diameter * planetList[i].satellites[j].distFromPlanet) * cos(angle*planetList[i].satellites[j].orbitSpeed)));
                }
              }

            // scale moon according to its diameter
            planetList[i].satellites[j].moon.model = glm::scale(planetList[i].satellites[j].moon.model, glm::vec3(planetList[i].satellites[j].diameter, planetList[i].satellites[j].diameter, planetList[i].satellites[j].diameter));
          }

        // update planet's rotation
        planetList[i].object.model = glm::rotate(planetList[i].object.model, angle/planetList[i].rotSpeed, glm::vec3(0, 1, 0));
      }

    // update the sky box
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0 * sin(angle), 0.0, 0.0 * cos(angle)));
    model = glm::scale(model, glm::vec3(150.f, 150.f, 150.f));

    ring = glm::translate(glm::mat4(1.0f), glm::vec3(0.0 * sin(angle), 0.0, 0.0 * cos(angle)));


    // render camera postition
    if(freeCamera)
      {
        checkKeyboard();
      }
    else
      {
        camera.Position = glm::vec3(planetList[planetNum].orbitRadius * sin((angle + planetList[planetNum].orbitAngle)/planetList[planetNum].orbitSpeed), yOffest,
                                                      zoom+planetList[planetNum].orbitRadius * cos((angle + planetList[planetNum].orbitAngle)/planetList[planetNum].orbitSpeed ));
          camera.ViewDir.x = 0.0f;
          camera.ViewDir.y = -0.48f;
          camera.ViewDir.z = -0.87f;
          camera.RotatedX = camera.RotatedY = camera.RotatedZ = 0.0f;
      }

    glm::vec3 ViewPoint = camera.Position + camera.ViewDir;
    view = glm::lookAt( glm::vec3(camera.Position.x,camera.Position.y,camera.Position.z),
                        glm::vec3(ViewPoint.x,ViewPoint.y,ViewPoint.z),
                        glm::vec3(camera.UpVector.x,camera.UpVector.y,camera.UpVector.z));


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
    // Strafe left
    if(keys['a'])
      camera.StrafeRight(-2.1);

    // Strafe right
    if(keys['d'])
        camera.StrafeRight(2.1);

    // Move forward
    if(keys['w'])
        camera.MoveForward( -2.1 ) ;

    // Move backward
    if(keys['s'])
        camera.MoveForward( 2.1 ) ;

    // move up
    if(keys['q'])
        camera.MoveUpward(3.3);

    // move down
    if(keys['e'])
        camera.MoveUpward(-3.3);

    // look up
    if(keys['i'])
        camera.RotateX(1.0);

    // look down
    if(keys['k'])
        camera.RotateX(-1.0);

    // roll right
    if(keys['l'])
        camera.RotateZ(-2.0);

    // roll left
    if(keys['j'])
        camera.RotateZ(2.0);

    // look left
    if(keys['u'])
        camera.RotateY(2.0);

    // look right
    if(keys['o'])
        camera.RotateY(-2.0);
  }


void keyboardListener(unsigned char key, int x_pos, int y_pos)
  {
    // check if key is escape
    if(key == 27)
      {
        glutDestroyWindow(window);
        return;
      }

    // check if key is the 1-9 keys
    if(key >= '1' && key <= '9')
      {
        if(key == '1')
        {
          planetNum = 0;
          yOffest = 2.0f;
          zoom = 5;
        }
        else if(key == '2')
        {
          planetNum = 1;
          yOffest = 2.0f;
          zoom = 5;
        }
        else if(key == '3')
        {
          planetNum = 2;
          yOffest = 2.0f;
          zoom = 5;
        }
        else if(key == '4')
        {
          planetNum = 3;
          yOffest = 2.0f;
          zoom = 5;
        }
        else if(key == '5')
        {
          planetNum = 4;
          yOffest = 20.0f;
          zoom = 40;
        }
        else if(key == '6')
        {
          planetNum = 5;
          yOffest = 20.0f;
          zoom = 40;
        }
        else if(key == '7')
        {
          planetNum = 6;
          yOffest = 20.0f;
          zoom = 40;
        }
        else if(key == '8')
        {
          planetNum = 7;
          yOffest = 20.0f;
          zoom = 40;
        }
        else
        {
          planetNum = 8;
          yOffest = 2.0f;
          zoom = 5;
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


void createMenu()
  {
    // create main menu entries
    glutCreateMenu(menuListener);
    glutAddMenuEntry("Start Rotation", 2);
    glutAddMenuEntry("Stop Rotation", 3);
    glutAddMenuEntry("Change Scale", 4);
    glutAddMenuEntry("Toggle Orbit Paths", 5);
    glutAddMenuEntry("Quit", 1);

    // set right mouse click to open menu
    glutAttachMenu(GLUT_RIGHT_BUTTON);
  }


void menuListener(int selection)
  {
    // check which menu option was selected
    switch(selection)
      {
        // exit the program
        case 1:
            glutDestroyWindow(window);
            return;
        break;

        // start rotation
        case 2:
            rotationFlag = true;
        break;

        // stop rotation
        case 3:
            rotationFlag = false;
        break;

        // change scale
        case 4:
            if (actualScale)
              loadData("solar_system_data_scaled.txt");
            else
              loadRealisticData("solar_system_data_actual.txt");

            // load all the planet models
            for(unsigned int i = 0; i < planetList.size(); i++)
            {
              planetList[i].object.loadModel(planetList[i].name + ".obj");

              for (unsigned j = 0; j < planetList[i].satellites.size(); j++)
              {
                planetList[i].satellites[j].moon.loadModel("moon.obj");
              }
            }
            actualScale = !actualScale;
        break;

        case 5:
            drawOrbits = !drawOrbits;
        break;
      }
    glutPostRedisplay();
  }


void cleanUp()
  {
    for(unsigned int i = 0; i < planetList.size(); i++)
      {
        planetList[i].object.deleteModel();
      }
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

bool loadRealisticData(string fileName)
  {
    // initialize variables
    Planet temp;
    ifstream fin;
    string line, data;
    int numMoons;

    planetList.clear();

    // open file
    fin.open(fileName.c_str());
    if(fin.is_open())
      {
        // check if the file is not empty
        if(!fin.eof())
          {
            // get the 3 lines of comments
            for(int i = 0; i < 3; i++)
              {
                getline(fin, line);
              }

            // loop through all the planets and the sun
            for(int i = 0; i < 10; i++)
              {
                // get all the planet data
                fin >> temp.name >> temp.orbitSpeed >> temp.orbitAngle >> temp.diameter
                       >> temp.mass >> temp.rotSpeed >> temp.rotAngle >> temp.orbitRadius;
                fin >> numMoons;

                // adjust orbit radius
                temp.orbitRadius /= 1000000;

                // make planets relative to earth
                temp.orbitSpeed /= 365.0f;
                temp.rotSpeed /= 24.0f;

                // convert angles to radians
                temp.orbitAngle *= M_PI/2;
                temp.rotAngle *= M_PI/2;

                temp.diameter /= 12756.0f;

                // get all the data for every moon for each planet
                vector<Satellite> tempSat(numMoons);
                for( int x = 0; x < numMoons; x++ )
                  {
                    fin >> tempSat[x].name >> tempSat[x].distFromPlanet >> tempSat[x].orbitSpeed >> tempSat[x].diameter;

                    tempSat[x].distFromPlanet = (tempSat[x].distFromPlanet / 1000000);

                    // make size relative to earth
                    tempSat[x].diameter /= (temp.diameter*12756);

                    tempSat[x].orbitSpeed = 365.26f/tempSat[x].orbitSpeed;
                  }
                temp.satellites = tempSat;

              planetList.push_back(temp);
            }
          }

        fin.close();
        return true;
      }
    else
      {
        cout << "Cannot Open File: " << fileName << endl;
        return false;
      }
  }



bool loadData(string fileName)
  {
    // initialize variables
    Planet temp;
    ifstream fin;
    string line, data;
    int numMoons;

    planetList.clear();

    // open file
    fin.open(fileName.c_str());
    if(fin.is_open())
      {
        // check if the file is not empty
        if(!fin.eof())
          {
            // get the 3 lines of comments
            for(int i = 0; i < 3; i++)
              {
                getline(fin, line);
              }

            // loop through all the planets and the sun
            for(int i = 0; i < 10; i++)
              {
                // get all the planet data
                fin >> temp.name >> temp.orbitSpeed >> temp.orbitAngle >> temp.diameter
                       >> temp.mass >> temp.rotSpeed >> temp.rotAngle >> temp.orbitRadius;
                fin >> numMoons;

                // adjust orbit radius
                temp.orbitRadius *= 35.0f;

                // make planets relative to earth
                temp.orbitSpeed /= 365.0f;
                temp.rotSpeed /= 24.0f;

                // convert angles to radians
                temp.orbitAngle *= M_PI/2;
                temp.rotAngle *= M_PI/2;

                temp.diameter *= 0.72f;


                // get all the data for every moon for each planet
                vector<Satellite> tempSat(numMoons);
                for( int x = 0; x < numMoons; x++ )
                  {
                    fin >> tempSat[x].name >> tempSat[x].distFromPlanet >> tempSat[x].orbitSpeed >> tempSat[x].diameter;

                    tempSat[x].distFromPlanet = (tempSat[x].distFromPlanet / 149597871.f) + .03;

                    tempSat[x].orbitSpeed = 365.26f/tempSat[x].orbitSpeed;
                  }
                temp.satellites = tempSat;

              planetList.push_back(temp);
            }
          }

        fin.close();
        return true;
      }
    else
      {
        cout << "Cannot Open File: " << fileName << endl;
        return false;
      }
  }
