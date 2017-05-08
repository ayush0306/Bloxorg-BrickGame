#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projectionO, projectionP;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;
int proj_type;
glm::vec3 tri_pos, rect_pos;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
	{
	    std::string Line = "";
	    while(getline(VertexShaderStream, Line))
		VertexShaderCode += "\n" + Line;
	    VertexShaderStream.close();
	}

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
	std::string Line = "";
	while(getline(FragmentShaderStream, Line))
	    FragmentShaderCode += "\n" + Line;
	FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void initGLEW(void){
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK){
	fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
    }
    if(!GLEW_VERSION_3_3)
	fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

double mouse1X =0, mouse1Y = 0,mouse2X =0, mouse2Y = 0;
int currLevel = 2, moveRight = 0, moveUp = 0, numOfSteps=0;
int leftClick = 0, rightClick = 0;
int lastMoveUp=0, lastMoveRight = 0;
int level[10][20], currView= 3;
float blockTransY, blockTransX;
// bool triangle_rot_status = true;
int endGame=0, win=0;
double overTime= -10.0;
float gameTime = 0, startTime;

VAO *triangle, *Tile, *fragile, *blockVer, *blockAlongy, *blockAlongx, *horSwitch, *verSwitch;
int currblock ;
int checkH = 0, checkS = 0;
float r1 = 0.3f , g1 = 0.0f , b1 = 0.15f ;

VAO *retCurrBlock(int value)
{
  if(value==1)
    return blockVer;
  if(value==2)
    return blockAlongy;
  return blockAlongx;
}

void move_block();

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
	        default:
	          break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
	        case GLFW_KEY_ESCAPE:
	            quit(window);
	            break;
          case GLFW_KEY_LEFT:
              numOfSteps++;
              moveRight = -1;
              move_block();
              break;
          case GLFW_KEY_RIGHT:
              numOfSteps++;
              moveRight = 1;
              move_block();
              break;
          case GLFW_KEY_UP:
              numOfSteps++;
              moveUp = 1;
              move_block();
              break;
          case GLFW_KEY_DOWN:
              numOfSteps++;
              moveUp = -1;
              move_block();
              break;
      	default:
      	    break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
    case 'Q':
    case 'q':
	   quit(window);
	   break;
    case ' ':
//	proj_type ^= 1;
	   break;
    case 'z':
	currView= 1;
	break;
    case 'x':
    currView= 2;
    break;
    case 'c':
    currView= 3;
	break;
    case 'v':
	currView= 4;
	break;
    case 'b':
    currView= 5;
    default:
	break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
	if (action == GLFW_PRESS)
        leftClick=1;
    if(action == GLFW_RELEASE)
        leftClick = 0;
	break;
    case GLFW_MOUSE_BUTTON_RIGHT:
	if (action == GLFW_PRESS)
        rightClick=1;
    if(action == GLFW_RELEASE)
        rightClick = 0;
	break;
    default:
	break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projectionP = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projectionO = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}


void createTile(int col)
{
    // GL3 accepts only Triangles. Quads are not supported
    GLfloat vertex_buffer_data [] = {
	0, 0, 0, // vertex 1
	0.95, 0, 0, // vertex 2
	0.95, 0.95, 0, // vertex 3

	0, 0, 0, // vertex 0.95
	0.95, 0.95, 0, // vertex 3
  0, 0.95, 0,  // vertex 4

  0, 0, -0.2,
  0.95, 0, -0.2,
  0.95, 0.95, -0.2,

  0, 0, -0.2,
  0.95, 0.95, -0.2,
  0, 0.95, -0.2,

  0, 0, 0,
  0, 0 ,-0.2,
  0.95, 0 , 0,

  0.95, 0, 0,
  0.95, 0, -0.2,
  0, 0, -0.2,

  0.95, 0, 0,
  0.95, 0, -0.2,
  0.95, 0.95, -0.2,

  0.95, 0.95, -0.2,
  0.95, 0.95, 0,
  0.95, 0, 0,

  0.95, 0.95, 0,
  0.95, 0.95, -0.2,
  0, 0.95, 0,

  0, 0.95, 0,
  0, 0.95, -0.2,
  0.95, 0.95, -0.2,

  0, 0, 0,
  0, 0, -0.2,
  0, 0.95, 0,

  0, 0.95, 0,
  0, 0.95, -0.2,
  0, 0, -0.2
    };

    GLfloat color_buffer_data [] = {
	0.5,0.25*col,0, // color 1
	0.5,0.25*col,0, // color 2
	0.5,0.25*col,0, // color 3

  0.5,0.25*col,0, // color 1
	0.5,0.25*col,0, // color 2
	0.5,0.25*col,0,   // color 4

  0.5,0.25*col,0, // color 1
  0.5,0.25*col,0, // color 2
  0.5,0.25*col,0,

  0.5,0.25*col,0, // color 1
  0.5,0.25*col,0, // color 2
  0.5,0.25*col,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,

  0,0,0,
  0,0,0,
  0,0,0,
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    if(col==1)
      Tile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
    else
      fragile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBlock_Alongy()
{
  GLfloat vertex_buffer_data[]={
    1, 0, 0,
    0, 0, 0,
    0, 0, 1,

    1, 0, 0,
    0, 0, 1,
    1, 0, 1,

    1, 2, 0,
    0, 2, 0,
    0, 2, 1,

    1, 2, 0,
    0, 2, 1,
    1, 2, 1,

    1, 0, 0,
    0, 0, 0,
    0, 2, 0,

    1, 0, 0,
    0, 2, 0,
    1, 2, 0,

    1, 0, 1,
    0, 0, 1,
    0, 2, 1,

    1, 0, 1,
    0, 2, 1,
    1, 2, 1,

    0, 0, 1,
    0, 0, 0,
    0, 2, 0,

    0, 0, 1,
    0, 2, 0,
    0, 2, 1,

    1, 0, 1,
    1, 0, 0,
    1, 2, 0,

    1, 0, 1,
    1, 2, 0,
    1, 2, 1,
  };

  GLfloat color_buffer_data [] = {
    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  blockAlongy = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBlock_Alongx()
{
  GLfloat vertex_buffer_data[]={
    0, 1, 0,
    0, 0, 0,
    0, 0, 1,

    0, 1, 0,
    0, 0, 1,
    0, 1, 1,

    2, 1, 0,
    2, 0, 0,
    2, 0, 1,

    2, 1, 0,
    2, 0, 1,
    2, 1, 1,

    0, 1, 0,
    0, 0, 0,
    2, 0, 0,

    0, 1, 0,
    2, 0, 0,
    2, 1, 0,

    0, 1, 1,
    0, 0, 1,
    2, 0, 1,

    0, 1, 1,
    2, 0, 1,
    2, 1, 1,

    0, 0, 1,
    0, 0, 0,
    2, 0, 0,

    0, 0, 1,
    2, 0, 0,
    2, 0, 1,

    0, 1, 1,
    0, 1, 0,
    2, 1, 0,

    0, 1, 1,
    2, 1, 0,
    2, 1, 1
  };

  GLfloat color_buffer_data [] = {
    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  blockAlongx = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBlock_Ver()
{
  GLfloat vertex_buffer_data[]={
    0, 0, 0, // vertex 1
  	1, 0, 0, // vertex 2
  	1, 1, 0, // vertex 3

  	0, 0, 0, // vertex 1
  	1, 1, 0, // vertex 3
    0, 1, 0,  // vertex 4

    0, 0, 2,
    1, 0, 2,
    1, 1, 2,

    0, 0, 2,
    1, 1, 2,
    0, 1, 2,

    0, 0, 0,
    0, 0 ,2,
    1, 0 , 0,

    1, 0, 0,
    1, 0, 2,
    0, 0, 2,

    1, 0, 0,
    1, 0, 2,
    1, 1, 2,

    1, 1, 2,
    1, 1, 0,
    1, 0, 0,

    1, 1, 0,
    1, 1, 2,
    0, 1, 0,

    0, 1, 0,
    0, 1, 2,
    1, 1, 2,

    0, 0, 0,
    0, 0, 2,
    0, 1, 0,

    0, 1, 0,
    0, 1, 2,
    0, 0, 2
  };

  GLfloat color_buffer_data [] = {
    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,

    r1,g1,b1,
    r1,g1,b1,
    r1,g1,b1,
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  blockVer = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createVerSwitch()
{
    GLfloat vertex_buffer_data[]={
      0.25,0.25,0.1,
      0.25,0.75,0.1,
      0.75,0.75,0.1,

      0.25,0.25,0.1,
      0.75,0.25,0.1,
      0.75,0.75,0.1
    };

    GLfloat color_buffer_data[]={
      0.0, 0.0, 0,
      0.0, 0.0, 0,
      0.0, 0.0, 0,

      0.0, 0.0, 0,
      0.0, 0.0, 0,
      0.0, 0.0, 0
    };

    verSwitch = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createHorSwitch()
{
    // printf("x : %lf\ny : %lf\n",xshift,yshift);
    GLfloat vertex_buffer_data[]={
      0.25,0.25,0.1,
      0.5,0.5,0.1,
      0.75,0.25,0.1
    };

    GLfloat color_buffer_data[]={
        0.0, 0.0, 0,
        0.0, 0.0, 0,
        0.0, 0.0, 0
    };

    horSwitch = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90.0;

void checkSwitch()
{
  int blockx = 7 - blockTransX ;
  int blocky = 4 - blockTransY ;

  if(level[blocky][blockx]==5 && currblock==1)
    checkH = 1 - checkH ;
  else if(currblock==2)
  {
    if(level[blocky][blockx]==6 || level[blocky-1][blockx]==6)
      checkS = 1 - checkS ;
  }
  else if(currblock==3)
  {
    if(level[blocky][blockx]==6 || level[blocky][blockx-1]==6)
      checkS = 1 - checkS ;
  }
  return ;
}

void checkGameOver()
{
    int blockx = 7 - blockTransX ;
    int blocky = 4 - blockTransY ;

    if(level[blocky][blockx]==0 || blockx >= 20 || blockx < 0 || blocky >=10 || blocky<0)
      endGame = 1;
    if(level[blocky][blockx]==7 && checkH == 0)
      endGame = 1;
    if(level[blocky][blockx]==8 && checkS == 0)
      endGame = 1;
    else if(currblock == 1)
    {
      if(level[blocky][blockx]==3)
      {
        endGame = 1;
        win = 1;
      }
      else if(level[blocky][blockx]==4)
      {
        endGame = 1;
        level[blocky][blockx]=0;
      }
    }
    else if(currblock == 2)
    {
        if(level[blocky-1][blockx]==0)
          endGame = 1;
        if(level[blocky-1][blockx]==7 && checkH==0)
          endGame = 1;
        if(level[blocky-1][blockx]==8 && checkS==0)
          endGame = 1;
        else if(blocky < 0)
          endGame = 1;
    }
    else
    {
      if(level[blocky][blockx-1]==0)
        endGame = 1;
      if(level[blocky][blockx-1]==7 && checkH==0)
        endGame = 1;
      if(level[blocky][blockx-1]==8 && checkS==0)
        endGame = 1;
      else if(blockx < 0)
        endGame = 1;
    }
    if(endGame == 1 && overTime<0)
    {
        if(win==1)
          printf("CONGRATULATIONS YOU WON!\n");
        else
          printf("BETTER LUCK NEXT TIME :(\n");
        printf("Number Of Steps Taken: %d\n",numOfSteps);
        printf("Time Taken: %lf\n",gameTime);
        overTime = glfwGetTime();
    }
}

void move_block()
{
    if(moveUp!=0)
    {
        if(currblock == 1)
        {
          blockTransY += moveUp - (1-moveUp)/2 ;
          currblock = 2;
        }
        else if(currblock == 2)
        {
            blockTransY += moveUp + (1+moveUp)/2 ;
            currblock = 1;
        }
        else
        {
          blockTransY += moveUp ;
        }
        lastMoveUp = moveUp;
        moveUp = 0;
    }
    else if(moveRight!=0)
    {
        if(currblock == 1)
        {
          blockTransX += moveRight - (1-moveRight)/2;
          currblock = 3;
        }
        else if(currblock == 3)
        {
          blockTransX += moveRight + (1+moveRight)/2 ;
          currblock = 1;
        }
        else
        {
          blockTransX += moveRight ;
        }
        lastMoveRight = moveRight;
        moveRight =0;
    }
    checkGameOver();
    checkSwitch();
    return ;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window, float x, float y, float w, float h)
{
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));
    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram(programID);

    glm::vec3 eye,up,target;
    // Eye - Location of camera. Don't change unless you are sure!!
    if(currView-1==0)
    {
        if(currblock-2==0)
        {
            up = glm::vec3(0,0,1);
            if(lastMoveUp-1>=0)
                eye = glm::vec3(blockTransX, blockTransY+1,0.5);
            else
                eye = glm::vec3(blockTransX, blockTransY,0.5);
            target = glm::vec3(-2000,0,0);
        }
        else if(currblock-1==0)
        {
            up = glm::vec3(0,0,1);
            if(!currLevel);
            else
            {
                eye = glm::vec3(blockTransX, blockTransY+0.5, 0.5);
                target = glm::vec3(-2000,0,0);
            }
        }
        else if(currblock-3==0)
        {
            up = glm::vec3(0,0,1);
                if(lastMoveRight-1 == 0)
                    eye = glm::vec3(blockTransX, blockTransY+0.5, 0.5);
                else
                    eye = glm::vec3(blockTransX-1, blockTransY+0.5,0.5);
                target = glm::vec3(-2000,0,0);
        }
    }
    else if(currView-2==0)
    {
        eye  = glm::vec3(0,0,10);
        up = glm::vec3(0,1,0);
        target = glm::vec3(0,0,0);
    }
    else if(currView-3==0)
    {
        eye= glm::vec3( 0, -7, 7 );
        up= glm::vec3(0, 1, 0);
        target = glm::vec3(0,0,0);
    }
    else if(currView-4==0)
    {
        if(currblock-1 == 0)
        {
            up = glm::vec3(0,0,1);
            if(!currLevel);
            eye = glm::vec3(blockTransX+5, blockTransY+0.5, 4);
            target = glm::vec3(-2000,0,0);
        }
        else if(currblock-2 == 0)
        {
            up = glm::vec3(0,0,1);
            if(!currLevel);
            if(lastMoveUp==1)
                eye = glm::vec3(blockTransX+5, blockTransY+1,4);
            else
                eye = glm::vec3(blockTransX+5, blockTransY,4);
            target = glm::vec3(-2000,0,0);

        }
        else if(currblock-3 == 0)
        {
            up = glm::vec3(0,0,1);
            if(lastMoveRight -1== 0)
                eye = glm::vec3(blockTransX+5, blockTransY+0.5, 4);
            else
                eye = glm::vec3(blockTransX+4, blockTransY+0.5,4);
            target = glm::vec3(-2000,0,0);
        }
    }
    else if(currView-5==0)
    {
        if(leftClick-1==0)
            camera_rotation_angle=camera_rotation_angle + 1;
        if(rightClick-1 == 0)
            camera_rotation_angle=camera_rotation_angle- 1;
        eye = glm::vec3(7*cos(camera_rotation_angle*M_PI/180.0), -7*sin(camera_rotation_angle*M_PI/180.0), 7);
        target = glm::vec3(0,0,0);
        up = glm::vec3(-1*cos(camera_rotation_angle*M_PI/180.0),sin(camera_rotation_angle*M_PI/180.0),0);
    }

    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = (proj_type?Matrices.projectionP:Matrices.projectionO) * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model
    if(endGame-1==0)
    {
        double currTime = glfwGetTime();
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBlock = glm::translate (glm::vec3(blockTransX, blockTransY, 0 - 5*(currTime - overTime)));        // glTranslatef
        Matrices.model *= (translateBlock);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(retCurrBlock(currblock));
        if(currTime - overTime > 2.0)
            exit(0);
    }
    else
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBlock = glm::translate (glm::vec3(blockTransX, blockTransY, 0));        // glTranslatef
        // glm::mat4 rotateBlock = glm::rotate(blockRotAngle, blockRotAxis); // rotate about vector (-1,1,1)
        Matrices.model *= (translateBlock);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(retCurrBlock(currblock));
    }

    //Draw the Tile
    int i=0,j=0;
    while(i<10)
    {
        j=0; i++;
        while(j<20)
        {
            if(level[i-1][j] && level[i-1][j]-3)
            {
              Matrices.model = glm::mat4(1.0f);
              glm::mat4 translateRectangle = glm::translate (glm::vec3(7-j,4-i+1,0));         // glTranslatef
              Matrices.model *= (translateRectangle);
              MVP = VP * Matrices.model;
              glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
              if(level[i-1][j]==4)
                draw3DObject(fragile);
              else if(level[i-1][j]==7)
               {
                 if(checkH == 1)
                 draw3DObject(Tile);
               }
              else if(level[i-1][j]==8)
              {
                if(checkS == 1)
                draw3DObject(Tile);
              }
              else if(level[i-1][j]!=4)
              {
                if(level[i-1][j]!=7)
                {
                   if(level[i-1][j]!=8)
                   {
                     draw3DObject(Tile);
                   }
              }
              if(level[i-1][j]+2==7)
                draw3DObject(verSwitch);
              if(level[i-1][j]==6)
                draw3DObject(horSwitch);
            }
          }
          j++;
        }
    }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
	exit(EXIT_FAILURE);
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    createTile (1);
    createTile(0);
    createBlock_Ver();
    createBlock_Alongy();
    createBlock_Alongx();
    // bridgeBinding();
    // cout<<level1[28];
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    reshapeWindow (window, width, height);
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
}

void selectLevel(int lev)
{
  FILE * file;
  char c;
  switch (lev) {
      case 1:
          file = fopen("level01.txt", "r");
          break;
      case 2:
          file = fopen("level02.txt", "r");
          break;
      case 3:
          file = fopen("level03.txt", "r");
          break;
      case 4:
          file = fopen("level04.txt", "r");
          break;
      default:
          file = fopen("level10.txt", "r");
          break;
  }

  int i=0,j=0;
  while(i<10)
  {
    j=0;
    while(j<20)
    {
      level[i][j] = 0;
      j++;
    }
    i++;
  }
  i=0;
  while(i<10)
  {j=0;
    while(j<20)
    {
      c = getc(file);
      if(c=='-')
        level[i][j] = 0;
      else if(c=='o')
        level[i][j] = 1;
      else if(c=='S')
      {
        level[i][j] = 2;
        blockTransY = 4 - i;
        blockTransX = 7 - j;
      }
      else if(c=='T')
        level[i][j] = 3;
      else if(c=='.')
        level[i][j] = 4;
      else if(c=='h')
      {
        level[i][j] = 5;
        createVerSwitch();
      }
      else if(c=='s')
      {
        level[i][j] = 6;
        createHorSwitch();
      }
      else if(c=='H')
        level[i][j] = 7;
      else if(c=='B')
        level[i][j] = 8;
      else if(c == '\n' || c == EOF)
      {
        break;
      }
      j++;
    }
    if(c == EOF)
    {
      fclose(file);
      break;
    }
    i++;
  }
  return ;
}

int main (int argc, char** argv)
{
    printf("Select the level you want to play : ");
    scanf("%d",&currLevel);
    currblock = 1;
    int width = 600;
    int height = 600;
    GLFWwindow* window = initGLFW(width, height);
    proj_type = 1;
    initGLEW();
    initGL (window, width, height);
    selectLevel(currLevel);

    double last_update_time = glfwGetTime(), current_time;
    startTime = last_update_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

	     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       draw(window, 0, 0, 1, 1);
       glfwSwapBuffers(window);
       glfwPollEvents();
       current_time = glfwGetTime(); // Time in seconds
       gameTime = current_time - startTime;
    }

    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}
