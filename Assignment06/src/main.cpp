#define GLM_FORCE_RADIANS
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <chrono>
#include <assimp/Importer.hpp> //includes the importer, which is used to read our obj file
#include <assimp/scene.h> //includes the aiScene object
#include <assimp/postprocess.h> //includes the postprocessing variables for the importer
#include <assimp/color4.h> //includes the aiColor4 object, which is used to handle the colors from the mesh objects
#include <ImageMagick/Magick++.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier


//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat uv[2];
};



//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
int numFaces;
std::vector<unsigned int> indices;
GLint windowHeight, windowWidth;
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
GLuint elementBuffer;
GLuint uvBuffer;
GLuint normalBuffer;


// image global variables
GLuint m_textureObj;
Magick::Image* m_pImage;
Magick::Blob m_blob;

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_tex;
GLint loc_sampler;

//transform matrices
glm::mat4 Model;//obj->world each object should have its own model matrix
glm::mat4 moonModel;
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 Mvp;//premultiplied modelviewprojection


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
    glutCreateWindow("Assimp Model Loading Example");

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
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    Mvp = projection * view * Model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(Mvp));


    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(loc_sampler, 0);
    glBindTexture(GL_ELEMENT_ARRAY_BUFFER, m_textureObj);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_pImage->columns(), m_pImage->rows(), 0, GL_RGB, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameteri(GL_ARRAY_BUFFER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_ARRAY_BUFFER, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    static float angle = 0.0;
    float dt = getDT();// if you have anything moving, use dt.

    angle += dt * M_PI/2; //move through 90 degrees a second


    Model = (glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angle), 0.0, 4.0 * cos(angle))));
    Model = (glm::rotate(Model, (4.f*angle), glm::vec3(0.0, 1.0, 0.0)));
    // Update the state of the scene
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
    std::string fileName = "capsule.obj";
    Assimp::Importer importer;
    std::vector<float> vertices;
    std::vector<float> uv;
    std::vector<float> normals;

    const aiScene *scene = importer.ReadFile(fileName.c_str() ,aiProcess_Triangulate);

    if(!scene){ // make sure file was read
      printf("Error parsing '%s': '%s'\n", fileName.c_str(), importer.GetErrorString());
    }

    //initialize the mesh in scene
    const aiMesh* mesh = scene->mMeshes[0];

    //load texture image
    std::string m_fileName;


        try {
        m_pImage = new Magick::Image("./capsule0.jpg");
        m_pImage->write(&m_blob, "RGBA");

    }
    catch (Magick::Error& Error) {
        std::cout << "Error loading texture '" << "capsule0.jpg" << "': " << Error.what() << std::endl;
        return false;
    }


    numFaces = mesh->mNumFaces;
    int numIndices = numFaces*3;
    indices.resize(numIndices);

    for(unsigned int i = 0; i < mesh->mNumFaces; i++){
      const aiFace& face = mesh->mFaces[i];
      assert(face.mNumIndices == 3);
      indices[i*3+0] = face.mIndices[0];
      indices[i*3+1] = face.mIndices[1];
      indices[i*3+2] = face.mIndices[2];
    }

    int numVertices = mesh->mNumVertices;
    vertices.resize(numVertices*3);
    normals.resize(numVertices*3);
    uv.resize(numVertices*2);

    for(unsigned int i = 0; i < mesh->mNumVertices; i++){
      if(mesh->HasPositions()){
        vertices[i*3+0] = mesh->mVertices[i].x;
        vertices[i*3+1] = mesh->mVertices[i].y;
        vertices[i*3+2] = mesh->mVertices[i].z;

      }
      if(mesh->HasNormals()){
        normals[i*3+0] = mesh->mNormals[i].x;
        normals[i*3+1] = mesh->mNormals[i].y;
        normals[i*3+2] = mesh->mNormals[i].z;
      }
      if(mesh->HasTextureCoords(0)){
        uv[i*2+0] = mesh->mTextureCoords[0][i].x;
        uv[i*2+1] = mesh->mTextureCoords[0][i].y;
      }

    }


    //create index buffer object
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(aiVector3D), &vertices[0], GL_STATIC_DRAW);

    // create uv buffer object
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(aiVector2D), &uv[0], GL_STATIC_DRAW);

    // create normal buffer object
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(aiVector3D), &normals[0], GL_STATIC_DRAW);

    glGenTextures(1, &m_textureObj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_ELEMENT_ARRAY_BUFFER, m_textureObj);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_pImage->columns(), m_pImage->rows(), 0, GL_RGB, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameterf(GL_ARRAY_BUFFER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_ARRAY_BUFFER, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    std::ifstream in("fragment.glsl");
    std::string content( (std::istreambuf_iterator<char>(in) ),
      std::istreambuf_iterator<char>() );

    const char *fs = content.c_str();

    std::ifstream in2("vertex.glsl");
    std::string content2( (std::istreambuf_iterator<char>(in2) ),
      std::istreambuf_iterator<char>() );

    const char *vs = content2.c_str();


    /*const char* fs = "varying vec3 color; void main(void){ gl_FragColor = vec4(color.rgb, 1.0); }";
    const char* vs = "attribute vec3 v_position; attribute vec3 v_color; varying vec3 color; uniform mat4 mvpMatrix; void main(void){ gl_Position = mvpMatrix * vec4(v_position, 1.0); color = v_color; }";
*/



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
    char buffer[512]; // buffer for error

if(!shader_status){

glGetShaderInfoLog(fragment_shader, 512, NULL, buffer); // inserts the error into the buffer

std::cerr << buffer << std::endl; // prints out error

return false;

}
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

    loc_tex = glGetAttribLocation(program,
                    const_cast<const char*>("v_tex"));
    if(loc_tex == -1)
    {
        std::cerr << "[F] V_TEX NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }

    loc_sampler = glGetUniformLocation(program,
                    const_cast<const char*>("gSampler"));
    if(loc_sampler == -1)
    {
        std::cerr << "[F] SAMPLER NOT FOUND" << std::endl;
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
  glDeleteBuffers(1, &uvBuffer);
  glDeleteBuffers(1, &normalBuffer);

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
