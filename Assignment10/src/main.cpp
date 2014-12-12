#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include <sstream>

#include "model.h"
#include "lightingEngine.h"


// Global variables
int w = 640, h = 480;// Window size

int numModelVertices;
bool rotating_x = false;
bool rotating_y = false;
int rotate_direction_x = 1;
int rotate_direction_y = 1;

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader
GLint loc_model;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;
GLint loc_normal;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

glm::mat4 model2;//obj->world each object should have its own model matrix

glm::mat4 mvp2;//premultiplied modelviewprojection

Model object; 
Model room;

// index 0 is vertex, index 1 is fragment
LightingEngine lightingEngine[2];


GLuint m_samplerLocation;
GLuint m_dirLightColorLocation;
GLuint m_dirLightAmbientIntensityLocation;
GLuint m_dirLightDiffuseIntensityLocation;
GLuint m_dirLightDirectionLocation;
DirectionalLight m_directionalLight;

int toggleShader = 0;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void idle();
void specialKeyHandler(int key, int xx, int yy);
void specialKeyReleaseHandler(int key, int xx, int yy);

//--Resource management
bool initialize(string, string, string);
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
{
	 // get shader file names
	 // this code was retrieved from http://www.cplusplus.com/forum/articles/13355/
	 string vsFileName, fsFileName, objFileName;
	 for (int i = 1; i < argc; i++) {
	 	if ((i + 1) != argc) {
			if (string(argv[i]) == "-f")
				fsFileName = argv[i + 1];
			else if (string(argv[i]) == "-v")
				vsFileName = argv[i + 1];
      else if (string(argv[i]) == "-m")
        objFileName = argv[i + 1];
		}
	 }
	 if (vsFileName.empty())
	 	vsFileName = "vs.glsl";
	 if (fsFileName.empty())
	 	fsFileName = "fs.glsl";	
   if (objFileName.empty())
    objFileName = "woodCube.obj";

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);

    // Name and create the Window
    glutCreateWindow("Matrix Example");

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutSpecialFunc(specialKeyHandler);// Called if a special keyboard key is pressed
    glutSpecialUpFunc(specialKeyReleaseHandler);// Called if a special key is released

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(vsFileName, fsFileName, objFileName);
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program

    glUseProgram(lightingEngine[toggleShader].shaderProgram);
    object.Render(mvp, model, lightingEngine[toggleShader]); 


    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float angle = 0.0;
    static float rotate_angle_x = 0.0;
    static float rotate_angle_y = 0.0;
    float dt = getDT();// if you have anything moving, use dt.

    angle += dt * M_PI/2; // move through 90 degrees a second	

    // orbit the model about the x and z axes
    model = glm::translate( glm::mat4(1.0f), glm::vec3(2.0 * sin(0), 0.0, 2.0 * 0));

    // only update if rotation is turned on to preserve position
    if (rotating_x) {
        rotate_angle_x += ((dt * M_PI/2) * rotate_direction_x);
    }
    if (rotating_y) {
        rotate_angle_y += ((dt * M_PI/2) * rotate_direction_y);
    }
    model = glm::rotate(model, rotate_angle_x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, rotate_angle_y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::scale(model, glm::vec3(3.0, 3.0, 3.0));

    // Update the state of the scene
    glutPostRedisplay();//call the display callback
}


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

// Handle keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos)
{
    if((key == 'q') || (key == 'Q') || (key == 27))//ESC
    {
        exit(0);
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

    else if (key == 's')
    {
        // toggle spot light
        toggleShader = (toggleShader+1)%2 ;
    }
}

void specialKeyHandler(int key, int xx, int yy) {
    switch(key) {
        case GLUT_KEY_LEFT:
            rotate_direction_y = 1;
            rotating_y = true;
        break;

        case GLUT_KEY_RIGHT:
            rotate_direction_y = -1;
            rotating_y = true;
        break;

        case GLUT_KEY_UP:
            rotate_direction_x = 1;
            rotating_x = true;
        break;

        case GLUT_KEY_DOWN:
            rotate_direction_x = -1;
            rotating_x = true;
        break;
    }
    
}

void specialKeyReleaseHandler(int key, int xx, int yy) {
    switch(key) {
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            rotating_y = false;
        break;
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
            rotating_x = false;
        break;
    }
}

bool initialize(string vertShaderFileName, string fragShaderFileName, string objFileName)
{
    // load models
    object.LoadMesh("earth.obj");

    lightingEngine[0].initialize("vs_vertex_shading.glsl", "fs_vertex_shading.glsl");  
    lightingEngine[1].initialize("vs_fragment_shading.glsl", "fs_fragment_shading.glsl");  

    // initialize the view and projection matrices
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

void idle () {
	glutPostRedisplay();
}

void cleanUp()
{
    // Clean up, Clean up
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}
