#define GLM_FORCE_RADIANS
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier


//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};

	
struct interact{
	bool rot = true, trans = true;

	
	// reverse rotation function
	void revRot(){
		rot ^= 0b1;
	}
	
	//reverse translation function
	void revTrans(){
		trans ^= 0b1;
	}
	
};


//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLint windowHeight, windowWidth;
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
// create an interactable struct for key presses
interact kb_press;

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
glm::mat4 earthMvp;//premultiplied modelviewprojection
glm::mat4 moonMvp;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouseClick(int button, int state, int x, int y);
void menuOptions(int id);

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
    glutMouseFunc(mouseClick); //Called if a mouse button is clicked
    glutCreateMenu(menuOptions);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Start Spinning", 2);
	glutAddMenuEntry("Stop Spinning", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

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
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    earthMvp = projection * view * earthModel;
    moonMvp = projection * view * moonModel;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader EARTH
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(earthMvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count
    
    //upload the matrix to the shader MOON
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(moonMvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float earthTransAngle = 0.0, moonTransAngle = 0.0, rotAngle = 0.0, moonRot;
    
    float dt = getDT();// if you have anything moving, use dt.
    
    //interaction to reverse translation
    if(kb_press.trans){ earthTransAngle += (dt * M_PI/2); }//move through 90 degrees a second
	else if(!kb_press.trans){ earthTransAngle -= (dt* M_PI/2); }
	
	//interaction for reversing moon rotation
	if(kb_press.trans){ moonTransAngle -= (dt * M_PI); }//move through 90 degrees a second
	else if(!kb_press.trans){ moonTransAngle += (dt* M_PI); }
	
	//interaction to reverse rotation
    if(kb_press.rot){ rotAngle += (dt * M_PI/2); }//move through 90 degrees a second
    else if(!kb_press.rot){ rotAngle -= (dt * M_PI/2); }

	moonRot += (dt * M_PI);
    
    earthModel = (glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(earthTransAngle), 0.0, 4.0 * cos(earthTransAngle)))); 
    moonModel = glm::translate( earthModel, glm::vec3(4.0 * sin(moonTransAngle), 0.0, 4.0 * cos(moonTransAngle))) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); ; 
    earthModel = (glm::rotate(earthModel, (4.f*rotAngle), glm::vec3(0.0, 1.0, 0.0)));
    moonModel = glm::rotate(moonModel, (2.f*moonRot), glm::vec3(0.0, 1.0, 0.0));
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
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    Vertex geometry[] = { {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},

                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}}
                        };
                        

    
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    // Put these into files and write a loader in the future
    // Note the added uniform!
    //prompt user for shader filenames
    std::ifstream in;
    std::string vContents, fContents;
    
    //vertex shader first
    //open and make sure file is good
    in.clear();
    in.open("vertex.txt");
    if(!in.good()){ std::cerr << "FAILED TO OPEN VERTEX SHADER FILE" << std::endl; }
    
    //if file is good, read contents into a string
    while(in.good()){ vContents += in.get(); }
    
    //close file once done
    in.close();
    
    //convert string to char*
    int vLen = vContents.length() -1;
    char* vs_temp = new char[vContents.length()];
    for(int x=0; x < vLen; x++){
		vs_temp[x] = vContents[x];
	}
	
	//store shader as const char* 
	const char* vs = vs_temp;
	
	//repeat for fragment shader
	//open and make sure file is good
	in.clear();
	in.open("fragment.txt");
	if(!in.good()){ std::cerr << "FAILED TO OPEN FRAGMENT SHADER FILE" << std::endl; }
    
    //if file is good, read contents into a string
    while(in.good()){ fContents += in.get(); }
    
    //close file once done
    in.close();
    
    //convert string to char*
    int fLen = fContents.length()-1;
    char* fs_temp = new char[fContents.length()];
    for(int x=0; x < fLen; x++){
		fs_temp[x] = fContents[x];
	}
	
	//store shader as const char*
	const char* fs = fs_temp;



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

//handles mouse input
void mouseClick(int button, int state, int x, int y){
	switch (button){
		case GLUT_LEFT_BUTTON : 
			if(state == GLUT_DOWN){kb_press.revRot();}
			break;
		case GLUT_MIDDLE_BUTTON :
			if(state == GLUT_DOWN){kb_press.revTrans();}
			break;
		}
}

void menuOptions(int id){
	float dt = getDT();// makes the spinning restart from stopping position
	switch(id){
		case 1:
			cleanUp();
			exit(0);
			break;
		case 2:
			// allows for rotation and translation while spinning is paused
			
			static float transAngle = 0.0, rotAngle = 0.0;
			//interaction to reverse translation
			if(kb_press.trans){ transAngle += (dt * M_PI/2); }//move through 90 degrees a second
			else if(!kb_press.trans){ transAngle -= (dt* M_PI/2); }
	
			//interaction to reverse rotation
			if(kb_press.rot){ rotAngle += (dt * M_PI/2); }//move through 90 degrees a second
			else if(!kb_press.rot){ rotAngle -= (dt * M_PI/2); }
    
			earthModel = (glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(transAngle), 0.0, 4.0 * cos(transAngle))));    
			earthModel = (glm::rotate(earthModel, (4.f*rotAngle), glm::vec3(0.0, 1.0, 0.0)));
			glutIdleFunc(update);
			break;
		case 3:
			glutIdleFunc(NULL);
			break;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    switch (key){
		case 27:
		case 'q':
		case 'Q':
			cleanUp();
			exit(0);
			break;
		case 'r':
		case 'R':
			kb_press.revRot();
			break;
		case 't':
		case 'T':
			kb_press.revTrans();
			break;
			
    }
    

}