// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL
#include <SOIL/SOIL.h>

// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// STL
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>

float deltaTime = 1.0f / 60.0f;

GLFWwindow *window;
int windowWidth = 640;
int windowHeight = 480;

GLuint program;
GLuint mvpUniform;
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

double mouseX, mouseY;
float speed = 0.09f;
float mouseSpeed = 0.04f;

glm::vec3 position = glm::vec3(0, 0, 5);
glm::vec3 direction, right, up;

float horizontalAngle = 3.14159f;
float verticalAngle = 0.0f;
float fov = 60.0f;

bool wKeyPressed;
bool sKeyPressed;
bool aKeyPressed;
bool dKeyPressed;

void initialize()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cout << "Error: GLFW failed to initialize.\n";
        return;
    }

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Hacks", NULL, NULL);
    if (!window)
    {
        std::cout << "Error: Failed to create window.\n";
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL functions
    if( GLEW_OK != glewInit() )
    {
        std::cout << "Error: Failed to load OpenGL functions.\n";
        glfwTerminate();
        return;
    }
}

void windowSizeChange(GLFWwindow *window, int width, int height)
{
    if (windowWidth != width || windowHeight != height)
    {
        windowWidth = width;
        windowHeight = height;

        projectionMatrix = glm::perspective
            (
            fov,
            (float)windowWidth / windowHeight,
            0.1f,
            100.0f
            );

        glUseProgram(program);
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUseProgram(0);

        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    }
}

void handleKeyboardInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)
        wKeyPressed = true;
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
        sKeyPressed = true;
    else if (key == GLFW_KEY_A && action == GLFW_PRESS)
        aKeyPressed = true;
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
        dKeyPressed = true;
    else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
        wKeyPressed = false;
    else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
        sKeyPressed = false;
    else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        aKeyPressed = false;
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
        dKeyPressed = false;
}

void getInput()
{
    if (wKeyPressed)
        position += direction * deltaTime * speed;
    if (sKeyPressed)
        position -= direction * deltaTime * speed;
    if (aKeyPressed)
        position -= right * deltaTime * speed;
    if (dKeyPressed)
        position += right * deltaTime * speed;

    direction = glm::vec3
        (
        std::cos(verticalAngle) * std::sin(horizontalAngle),
        std::sin(verticalAngle),
        std::cos(verticalAngle) * std::cos(horizontalAngle)
        );
    right = glm::vec3
        (
        std::sin(horizontalAngle - 3.14159f / 2.0f),
        0.0f,
        std::cos(horizontalAngle - 3.14159f / 2.0f)
        );
    up = glm::cross(right, direction);

    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
    horizontalAngle += mouseSpeed * deltaTime * float(windowWidth / 2 - mouseX);
    // Prevent from going upside down
    if (!up.y >= -0.5f)
        verticalAngle += mouseSpeed * deltaTime * float(windowHeight / 2 - mouseY);
    else if (up.y == -1.0f)
        up = glm::vec3(up.x, 1, up.z);
}

GLuint loadImage()
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char bytes[] =
    {
        255, 0, 0,
        0, 255, 0,
        0, 0, 255,
        255, 255, 0
    };

    textureID = SOIL_create_OGL_texture
        (
        bytes,
        2, 2, 3,
        textureID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
        );

    glBindTexture(GL_TEXTURE_2D, 0);
    if (textureID == 0)
        printf("SOIL Loading Error: %s\n", SOIL_last_result());

    return textureID;
}

void loadOBJ
    (
    const char *path,
    std::vector<unsigned int> &outIndices,
    std::vector<float> &outVertices,
    std::vector<float> &outUVs,
    std::vector<float> &outNormals
    ) {

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast);
    aiMesh* mesh = scene->mMeshes[0];

    int numOfFaces = mesh->mNumFaces;
    int numOfIndices = numOfFaces * 3;
    outIndices.resize(numOfIndices);

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace &face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        outIndices[i * 3 + 0] = face.mIndices[0];
        outIndices[i * 3 + 1] = face.mIndices[1];
        outIndices[i * 3 + 2] = face.mIndices[2];
    }

    int numOfVertices = mesh->mNumVertices;
    outVertices.resize(numOfVertices * 3);
    outNormals.resize(numOfVertices * 3);
    outUVs.resize(numOfVertices * 2);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        if (mesh->HasPositions()) {
            outVertices[i * 3 + 0] = mesh->mVertices[i].x;
            outVertices[i * 3 + 1] = mesh->mVertices[i].y;
            outVertices[i * 3 + 2] = mesh->mVertices[i].z;
        }

        if (mesh->HasNormals()) {
            outNormals[i * 3 + 0] = mesh->mNormals[i].x;
            outNormals[i * 3 + 1] = mesh->mNormals[i].x;
            outNormals[i * 3 + 2] = mesh->mNormals[i].x;
        }

        if (mesh->HasTextureCoords(0)) {
            outUVs[i * 2 + 0] = mesh->mTextureCoords[0][i].x;
            outUVs[i * 2 + 1] = mesh->mTextureCoords[0][i].y;
        }
    }
}

// GLSL shader program loader
struct Program
{
    static GLuint Load( const char* vert, const char* geom, const char* frag )
    {
        GLuint prog = glCreateProgram();
        if( vert ) AttachShader( prog, GL_VERTEX_SHADER, vert );
        if( geom ) AttachShader( prog, GL_GEOMETRY_SHADER, geom );
        if( frag ) AttachShader( prog, GL_FRAGMENT_SHADER, frag );
        glLinkProgram( prog );
        CheckStatus( prog );
        return prog;
    }

private:
    static void CheckStatus( GLuint obj )
    {
        GLint status = GL_FALSE, len = 10;
        if( glIsShader(obj) )   glGetShaderiv( obj, GL_COMPILE_STATUS, &status );
        if( glIsProgram(obj) )  glGetProgramiv( obj, GL_LINK_STATUS, &status );
        if( status == GL_TRUE ) return;
        if( glIsShader(obj) )   glGetShaderiv( obj, GL_INFO_LOG_LENGTH, &len );
        if( glIsProgram(obj) )  glGetProgramiv( obj, GL_INFO_LOG_LENGTH, &len );
        std::vector< char > log( len, 'X' );
        if( glIsShader(obj) )   glGetShaderInfoLog( obj, len, NULL, &log[0] );
        if( glIsProgram(obj) )  glGetProgramInfoLog( obj, len, NULL, &log[0] );
        std::cerr << &log[0] << std::endl;
        exit( -1 );
    }

    static void AttachShader( GLuint program, GLenum type, const char* src )
    {
        GLuint shader = glCreateShader( type );
        glShaderSource( shader, 1, &src, NULL );
        glCompileShader( shader );
        CheckStatus( shader );
        glAttachShader( program, shader );
        glDeleteShader( shader );
    }
};
#define GLSL(version, shader) "#version " #version "\n" #shader

const char* vert = GLSL
(
    330 core,
    layout (location = 0) in vec3 position_modelspace;
    layout (location = 1) in vec2 uv;
    layout (location = 2) in vec3 normal_modelspace;

    out vec2 UV;
    out vec3 Position_worldspace;
    out vec3 Normal_cameraspace;
    out vec3 EyeDirection_cameraspace;
    out vec3 LightDirection_cameraspace;

    uniform mat4 mvp;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
    uniform vec3 lightPosition_worldspace;

    void main()
    {
        gl_Position = mvp * vec4(position_modelspace, 1.0f);

        Position_worldspace = (modelMatrix * vec4(position_modelspace, 1.0f)).xyz;

        vec3 position_modelspaceCamera = (viewMatrix * modelMatrix * vec4(position_modelspace, 1.0f)).xyz;
        EyeDirection_cameraspace = vec3(0, 0, 0) - position_modelspaceCamera;

        vec3 lightPosition_worldspaceCamera = (viewMatrix * vec4(lightPosition_worldspace, 1.0f)).xyz;
        LightDirection_cameraspace = lightPosition_worldspaceCamera + EyeDirection_cameraspace;

        Normal_cameraspace = (viewMatrix * modelMatrix * vec4(normal_modelspace, 0.0f)).xyz;
        UV = uv;
    }
);

const char* frag = GLSL
(
    330 core,
    in vec2 UV;
    in vec3 Position_worldspace;
    in vec3 Normal_cameraspace;
    in vec3 EyeDirection_cameraspace;
    in vec3 LightDirection_cameraspace;

    out vec3 color;

    uniform sampler2D textureSampler;
    uniform mat4 modelViewMatrix;
    uniform vec3 lightPosition_worldspace;

    void main()
    {
        vec3 lightColor = vec3(1, 1, 1);
        float lightPower = 50.0f;

        vec3 materialDiffuseColor = texture2D(textureSampler, UV).rgb;
        vec3 materialAmbientColor = vec3(0.1, 0.1, 0.1) * materialDiffuseColor;
        vec3 materialSpecularColor = vec3(0.3, 0.3, 0.3);

        float distance = length(lightPosition_worldspace - Position_worldspace);

        vec3 n = normalize(Normal_cameraspace);
        vec3 l = normalize(LightDirection_cameraspace);

        float cosTheta = clamp(dot(n, l), 0, 1);

        vec3 eye = normalize(EyeDirection_cameraspace);
        vec3 reflection = reflect(-l, n);

        float cosAlpha = clamp(dot(eye, reflection), 0, 1);

        color = materialAmbientColor +
                materialDiffuseColor * lightColor * lightPower * cosTheta / (distance * distance) +
                materialSpecularColor * lightColor * lightPower * pow(cosAlpha, 5) / (distance * distance);
    }
);



int main(int argc, char *argv[])
{
    initialize();
    glfwSetWindowSizeCallback(window, windowSizeChange);
    glfwSetKeyCallback(window, handleKeyboardInput);

    // Z order buffering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Vertex Array Object
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    program = Program::Load( vert, NULL, frag );
    mvpUniform = glGetUniformLocation(program, "mvp");
    GLuint modelMatrixUniform = glGetUniformLocation(program, "modelMatrix");
    GLuint viewMatrixUniform = glGetUniformLocation(program, "viewMatrix");
    GLuint lightPositionUniform = glGetUniformLocation(program, "lightPosition_worldspace");

    GLuint texture = loadImage();
    GLuint textureSamplerUniform = glGetUniformLocation(program, "textureSampler");

    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> normals;
    loadOBJ("table2.obj", indices, vertices, uvs, normals);

    GLuint elementBuffer;
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(aiVector3D), &vertices[0], GL_STATIC_DRAW);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(aiVector2D), &uvs[0], GL_STATIC_DRAW);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(aiVector3D), &normals[0], GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window))
    {
        getInput();

        projectionMatrix = glm::perspective(fov, (float)windowWidth / windowHeight, 0.1f, 1000.0f);
        viewMatrix = glm::lookAt(position, position + direction, up);
        modelMatrix = glm::mat4(1.0f);

        // This is actually model -> view -> projection, not the other way around
        glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

        glfwPollEvents();

        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glm::vec3 lightPosition = glm::vec3(4, 4, 4);
        glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureSamplerUniform, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vertexArrayObject);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
