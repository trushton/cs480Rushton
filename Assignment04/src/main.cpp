#define GLM_FORCE_RADIANS
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include "shader.h"
#include "objloader.h"



#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier







//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLint windowHeight, windowWidth;
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
int numFaces;


//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 earthModel;//obj->world each object should have its own model matrix
glm::mat4 moonModel;
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 model;//premultiplied modelviewprojection


//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);


//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
{
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Model Loading Example");

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

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
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
    glClearColor(0.0, 0.5, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    model = projection * view * earthModel;
    //moonMvp = projection * view * moonModel;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader EARTH
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(model));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(glm::vec3)*2,//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(glm::vec3)*2,
                           (void*)sizeof(glm::vec3));

    glDrawArrays(GL_TRIANGLES, 0, numFaces*3);//mode, starting index, count


    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);

    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    static float rotation = 0.0;
    float dt = getDT();
    rotation += dt * M_PI /2;
    model = glm::rotate(model, 4.f*rotation, glm::vec3(0.0,1.0,0.0));
    glutPostRedisplay();//call the display callback
}






void reshape(GLsizei w, GLsizei h)
{
windowWidth = w;
windowHeight = h;
glViewport(0, 0, windowWidth, windowHeight);
}

bool initialize()
{
   
    std::vector< Vertex > vertices;
    std::vector< Vertex > uvs;
    std::vector< Vertex > normals; // Won't be used at the moment.

    numFaces = loadOBJ("table2.obj", vertices, uvs, normals);
    if(!numFaces){ std::cerr << "failed to load object"; }

    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    shader v,f;
    v.readIn("vertex.txt");
    f.readIn("fragment.txt");
    
    const char* vs = v.get();
    const char* fs = f.get();

    /*const char* fs = "varying vec3 color; void main(void){ gl_FragColor = vec4(color.rgb, 1.0); }";
    const char* vs = "attribute vec3 v_position; attribute vec3 v_color; varying vec3 color; uniform mat4 mvpMatrix; void main(void){ gl_Position = mvpMatrix * vec4(v_position, 1.0); color = v_color; }";*/

    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }

    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic
    //  for this project having them static will be fine
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

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
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



