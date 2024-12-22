#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"
//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			John Klucinec

// title of these windows:
const char* WINDOWTITLE = "OpenGL / PixelBoard -- John Klucinec";
const char* GLUITITLE = "PixelBoard";

// what the glui package defines as true and false:
const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:
const int ESCAPE = 0x1b;

// initial window size:
const int INIT_WINDOW_SIZE = 600;

// multiplication factors for input interaction:
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:
const float MINSCALE = 0.05f;

// scroll wheel button values:
const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:
const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):
const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// which projection:
enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):
const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };

// line width for the axes:
const GLfloat AXES_WIDTH = 3.;

// the color numbers:
enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char* ColorNames[] =
{
	(char*)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:
const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE = GL_LINEAR;
const GLfloat FOGDENSITY = 0.30f;
const GLfloat FOGSTART = 1.5f;
const GLfloat FOGEND = 4.f;

// for lighting:
const float	WHITE[] = { 1.,1.,1.,1. };

// for animation:
const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
#define DEMO_Z_FIGHTING
#define DEMO_DEPTH_BUFFER


// non-constant global variables:
int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;			// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:
void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
float	ElapsedSeconds();
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);
void	Axes(float);
void	HsvRgb(float[3], float[3]);
void	Cross(float[3], float[3], float[3]);
float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
float	Unit(float[3]);


// utility to create an array from 3 separate values:
float*
Array3(float a, float b, float c)
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:
float*
MulArray3(float factor, float array0[])
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

float*
MulArray3(float factor, float a, float b, float c)
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

float
Ranf(float low, float high)
{
	float r = (float)rand();             // 0 - RAND_MAX
	float t = r / (float)RAND_MAX;       // 0. - 1.

	return   low + t * (high - low);
}

void
TimeOfDaySeed()
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time(&now);
	double seconds = difftime(now, mktime(&y2k));
	unsigned int seed = (unsigned int)(1000. * seconds);    // milliseconds
	srand(seed);
}

//#include "setmaterial.cpp"
//#include "setlight.cpp"
//#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
//#include "bmptotexture.cpp"
//#include "loadobjfile.cpp"
//#include "keytime.cpp"
#include "glslprogram.cpp"
#include "vertexbufferobject.cpp"
#include "blocks.h"
#include <stdexcept>

VertexBufferObject Cube;
GLSLProgram CubeProgram;
GLSLProgram FloorProgram;
GLuint blocksList;
GLuint sampleBlocksList;

// === Game Constraints === 
const int NUM = 5;  // Number of cube instances in each direction
const int BASESIZE = NUM * NUM * NUM;

// === Level Management === 
size_t currentLevelIndex = 0;
bool blocksChanged = false;

std::vector<Block>& nextLevel()
{
	currentLevelIndex = (currentLevelIndex + 1) % levels.size();
	return levels[currentLevelIndex];
}

// === Scene Objects === 

// Drawing containers
std::vector<Block> blocks;         // User's current drawing
std::vector<Block> sampleBlocks = levels[currentLevelIndex];  // Reference drawing to match
Block floorBlock;                  // Floor representation (using instancing)

// Current selection state
glm::vec3 currPosition = glm::vec3(NUM / 2.0f, NUM / 2.0f, NUM / 2.0f);
glm::vec4 currColor = glm::vec4(Colors[RED][0], Colors[RED][1], Colors[RED][2], 1.0f);

// Check to see if both drawings are the same
bool compareVectors(const std::vector<Block>& vec1, const std::vector<Block>& vec2)
{
	if (vec1.size() <= 2 || vec2.size() <= 2)
	{
		return false;
	}

	// Compare all but the last two blocks (which are the selection blocks)
	size_t compareSize = std::min(vec1.size(), vec2.size()) - 2;
	return std::equal(vec1.begin(), vec1.begin() + compareSize,
					  vec2.begin());
}

// Set the current position of the selection block
void SetBlockPosition(int index, const glm::vec3& position)
{
	if (index < blocks.size())
	{
		blocks[index].position = position;
	}
}

// Set the active state of the block
void SetBlockActive(int index, bool active)
{
	if (index < blocks.size())
	{
		blocks[index].active = active;
	}
}

// Set the color of the block
void SetBlockColor(int index, const glm::vec4& color)
{
	if (index < blocks.size())
	{
		blocks[index].color = color;
	}
}

// Initialize the blocks
void InitializeBlocks()
{
	const int totalBlocks = BASESIZE + 2;	// Add two for the selection blocks

	// Fill with inactive blocks
	Block emptyBlock;
	emptyBlock.active = false;
	emptyBlock.color = currColor;
	blocks.resize(totalBlocks, emptyBlock);

	// Set the initial color and location of the selection block
	SetBlockPosition(totalBlocks - 2, currPosition);
	SetBlockActive(totalBlocks - 2, true);
	SetBlockColor(totalBlocks - 2, glm::vec4(currColor[0], currColor[1], currColor[2], 0.6f));

	// Set the color and initial location of the overlay for the selection block
	SetBlockPosition(totalBlocks - 1, currPosition);
	SetBlockActive(totalBlocks - 1, true);
	SetBlockColor(totalBlocks - 1, glm::vec4(WHITE[0], WHITE[1], WHITE[2], 0.4f));
}

// Initialize the floor block
void InitializeFloor()
{
	floorBlock.active = true;
	floorBlock.color = glm::vec4(WHITE[0], WHITE[1], WHITE[2], 1.0f);
}

// Draw the floor
void drawFloor(GLSLProgram& program, float shift)
{
	program.Use();
	program.SetUniformVariable("width", 5);
	program.SetUniformVariable("shift", shift);
	Cube.DrawInstanced(25);
	program.UnUse();
}

void buildBlocksList(const std::vector<Block>& blocks, float shift, GLuint& list)
{
	list = glGenLists(1);
	glNewList(list, GL_COMPILE);

	glPushMatrix();
	for (const Block& block : blocks)
	{
		if (block.active)
		{
			glPushMatrix();
			glTranslatef(block.position.x, block.position.y, block.position.z);
			glColor4fv(glm::value_ptr(block.color));
			Cube.Draw();
			glPopMatrix();
		}
	}

	glPopMatrix();
	glEndList();
}

void drawBlocks(GLSLProgram& program, float shift, GLuint list)
{
	program.Use();
	program.SetUniformVariable("shift", shift);
	glCallList(list);
	program.UnUse();
}

// main program:
int
main(int argc, char* argv[])
{
	// turn on the glut package:
	glutInit(&argc, argv);

	// setup all the graphics stuff:
	InitGraphics();

	// create the display lists that **will not change**:
	InitLists();

	// init all the global variables used by Display( ):
	Reset();

	// setup all the user interface stuff:
	InitMenus();

	// draw the scene once and wait for some interaction:
	glutSetWindow(MainWindow);
	glutMainLoop();

	return 0;
}

void
Animate()
{
	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// force a call to Display( ) next time it is convenient:
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// draw the complete scene:
void
Display()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);

	// erase the background:
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif

	glShadeModel(GL_SMOOTH);

	// Test to see if both vectors are equal
	if (blocksChanged && compareVectors(blocks, sampleBlocks))
	{
		blocks = std::vector<Block>();
		InitializeBlocks();

		// Get the next level
		sampleBlocks = nextLevel();

		// Rebuild the sample blocks list (which display the reference drawing)
		buildBlocksList(sampleBlocks, -0.5f, sampleBlocksList);
	}

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei vw, vh;

	if (vx / 2 > vy)
	{
		vh = vy;
		vw = vh * 2;
	}
	else
	{
		vw = vx;
		vh = vw / 2;
	}

	GLint xl = (vx - vw) / 2;
	GLint yb = (vy - vh) / 2;
	glViewport(xl, yb, vw, vh);

	// set the viewing volume:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (NowProjection == ORTHO)
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	else
		gluPerspective(70.f, 2.0f, 0.1f, 1000.f);

	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:
	gluLookAt(0.f, 4.f, 5.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);


	// rotate the scene:
	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

	// uniformly scale the scene:
	if (Scale < MINSCALE) Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);

	// set the fog parameters:
	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}

	// possibly draw the axes:
	if (AxesOn != 0)
	{
		glColor3fv(&Colors[NowColor][0]);
		glCallList(AxesList);
	}

	// since we are using glScalef( ), be sure the normals get unitized:
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Only build the blocks list if it has changed
	if (blocksChanged)
	{
		buildBlocksList(blocks, 5.5f, blocksList);
		blocksChanged = false;
	}

	drawBlocks(CubeProgram, 5.5f, blocksList);
	drawBlocks(CubeProgram, -0.5f, sampleBlocksList);

	// Draw the floor
	glDisable(GL_BLEND);
	drawFloor(FloorProgram, 1.0f);
	drawFloor(FloorProgram, -5.0f);

	glDisable(GL_DEPTH_TEST);

	// swap the double-buffered framebuffers:
	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !
	glFlush();
}

void
DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoColorMenu(int id)
{
	NowColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoDepthBufferMenu(int id)
{
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthFightingMenu(int id)
{
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoDepthMenu(int id)
{
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// main menu callback:
void
DoMainMenu(int id)
{
	switch (id)
	{
		case RESET:
			Reset();
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow(MainWindow);
			glFinish();
			glutDestroyWindow(MainWindow);
			exit(0);
			break;

		default:
			fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoProjectMenu(int id)
{
	NowProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:
void
DoRasterString(float x, float y, float z, char* s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}

// use glut to display a string of characters using a stroke font:
void
DoStrokeString(float x, float y, float z, float ht, char* s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}

// return the number of seconds since the start of the program:
float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// Cube vertices for a unit cube
const float VERTICES[] = {
	// Front face
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	// Back face
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f
};

// Cube indices
const unsigned int INDICES[] = {
	0, 1, 2, 2, 3, 0,  // Front
	1, 5, 6, 6, 2, 1,  // Right
	5, 4, 7, 7, 6, 5,  // Back
	4, 0, 3, 3, 7, 4,  // Left
	3, 2, 6, 6, 7, 3,  // Top
	4, 5, 1, 1, 0, 4   // Bottom
};

// initialize the glut and OpenGL libraries:
//	also setup callback functions
void
InitGraphics()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc(Animate);

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

	Cube.Init();
	Cube.glBegin(GL_TRIANGLES);
	for (int i = 0; i < 36; i++)
	{
		unsigned int idx = INDICES[i];
		Cube.glVertex3f(VERTICES[idx * 3], VERTICES[idx * 3 + 1], VERTICES[idx * 3 + 2]);
	}
	Cube.glEnd();

	// Initialize shader
	CubeProgram.Init();
	bool valid = CubeProgram.Create("cube.vert", "cube.frag");
	if (!valid)
		fprintf(stderr, "Shader cannot be created!\n");
	else
		fprintf(stderr, "Shader created.\n");

	// Initialize the blocks
	InitializeBlocks();

	// Initialize floor shader
	FloorProgram.Init();
	bool valid2 = FloorProgram.Create("floor.vert", "cube.frag");
	if (!valid2)
		fprintf(stderr, "Shader cannot be created!\n");
	else
		fprintf(stderr, "Shader created.\n");

	// Initialize the blocks
	InitializeFloor();

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void InitLists()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	glutSetWindow(MainWindow);

	// create the axes:
	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();

	// create the blocks list:
	blocksList = glGenLists(1);
	glNewList(blocksList, GL_COMPILE);
	glPushMatrix();
	for (const Block& block : blocks)
	{
		if (block.active)
		{
			glPushMatrix();
			glTranslatef(block.position.x, block.position.y, block.position.z);
			glColor4fv(glm::value_ptr(block.color));
			Cube.Draw();
			glPopMatrix();
		}
	}
	glPopMatrix();
	glEndList();


	// create the blocks list:
	sampleBlocksList = glGenLists(1);
	glNewList(sampleBlocksList, GL_COMPILE);
	glPushMatrix();
	for (const Block& block : sampleBlocks)
	{
		if (block.active)
		{
			glPushMatrix();
			glTranslatef(block.position.x, block.position.y, block.position.z);
			glColor4fv(glm::value_ptr(block.color));
			Cube.Draw();
			glPopMatrix();
		}
	}
	glPopMatrix();
	glEndList();
}

// initialize the glui window:
void
InitMenus()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(float));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)
	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);
	glutAddSubMenu("Axis Colors", colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

	glutAddSubMenu("Depth Cue", depthcuemenu);
	glutAddSubMenu("Projection", projmenu);
	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Get the index of the block at the current position
int GetBlockIndex()
{
	int ix = static_cast<int>(std::floor(currPosition[0]));
	int iy = static_cast<int>(std::floor(currPosition[1]));
	int iz = static_cast<int>(std::floor(currPosition[2]));

	// Bounds checking
	if (ix < 0 || ix >= NUM ||
		iy < 0 || iy >= NUM ||
		iz < 0 || iz >= NUM)
	{
		return -1; // Invalid index
	}

	// Calculate 1D index from 3D coordinates
	return ix + (iy * NUM) + (iz * NUM * NUM);
}

// Update the color of the selection block
void UpdateBlockColor(glm::vec4 newColor)
{
	SetBlockColor(BASESIZE, glm::vec4(newColor[0], newColor[1], newColor[2], 0.6f));
	currColor = newColor;

	blocksChanged = true;
}

// Move the selection block in the specified direction
void MoveSelectionBlock()
{
	SetBlockPosition(BASESIZE, currPosition);
	SetBlockColor(BASESIZE, glm::vec4(currColor[0], currColor[1], currColor[2], 0.6f));

	SetBlockPosition(BASESIZE + 1, currPosition);
	SetBlockColor(BASESIZE + 1, glm::vec4(WHITE[0], WHITE[1], WHITE[2], 0.4f));

	blocksChanged = true;
}

// Place the block at the current position
void PlaceBlock()
{
	SetBlockPosition(GetBlockIndex(), currPosition);
	SetBlockActive(GetBlockIndex(), true);
	SetBlockColor(GetBlockIndex(), currColor);

	blocksChanged = true;
}

// Remove the block at the current position
void RemoveBlock()
{
	SetBlockActive(GetBlockIndex(), false);
	SetBlockPosition(GetBlockIndex(), glm::vec3(0.00f, 0.00f, 0.00f));
	SetBlockColor(GetBlockIndex(), glm::vec4(1.00f, 0.00f, 0.00f, 1.00f));

	blocksChanged = true;
}

void UpdateSelectionBlock(int currPos, bool increment)
{
	float delta = increment ? 1.0f : -1.0f;
	float newPos = currPosition[currPos] + delta;

	if (newPos > 0.0f && newPos < 5.0f)
	{
		currPosition[currPos] += delta;
		MoveSelectionBlock();
	}
}


// the keyboard callback:
void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;

		case 'f':
		case 'F':
			PlaceBlock();
			break;

		case 'g':
		case 'G':
			RemoveBlock();
			break;

		case 'd':
		case 'D':
			UpdateSelectionBlock(0, true);
			break;

		case 's':
		case 'S':
			UpdateSelectionBlock(2, true);
			break;

		case 'a':
		case 'A':
			UpdateSelectionBlock(0, false);
			break;

		case 'w':
		case 'W':
			UpdateSelectionBlock(2, false);
			break;

		case 'q':
		case 'Q':
			UpdateSelectionBlock(1, false);
			break;

		case 'e':
		case 'E':
			UpdateSelectionBlock(1, true);
			break;

		case '1':
			UpdateBlockColor(glm::vec4(Colors[RED][0], Colors[RED][1], Colors[RED][2], 1.0f));
			break;

		case '2':
			UpdateBlockColor(glm::vec4(Colors[YELLOW][0], Colors[YELLOW][1], Colors[YELLOW][2], 1.0f));
			break;

		case '3':
			UpdateBlockColor(glm::vec4(Colors[GREEN][0], Colors[GREEN][1], Colors[GREEN][2], 1.0f));
			break;

		case '4':
			UpdateBlockColor(glm::vec4(Colors[CYAN][0], Colors[CYAN][1], Colors[CYAN][2], 1.0f));
			break;

		case '5':
			UpdateBlockColor(glm::vec4(Colors[BLUE][0], Colors[BLUE][1], Colors[BLUE][2], 1.0f));
			break;

		case '6':
			UpdateBlockColor(glm::vec4(Colors[MAGENTA][0], Colors[MAGENTA][1], Colors[MAGENTA][2], 1.0f));
			break;

		case 't':
			printf("std::vector<Block> blocks = {\n");
			for (const Block& block : blocks)
			{
				printf("    {glm::vec3(%.2ff, %.2ff, %.2ff), glm::vec4(%.2ff, %.2ff, %.2ff, %.2ff), %s},\n",
					   block.position.x, block.position.y, block.position.z,
					   block.color.r, block.color.g, block.color.b, block.color.a,
					   block.active ? "true" : "false"
				);
			}
			printf("};\n");


			break;

		case ESCAPE:
			DoMainMenu(QUIT);	// will not return here
			break;				// happy compiler

		default:
			fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up:
void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:
void
MouseMotion(int x, int y)
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}

	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
void
Reset()
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:
void
Resize(int width, int height)
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:
void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = { 0.f, 1.f, 0.f, 1.f };

static float xy[] = { -.5f, .5f, .5f, -.5f };

static int xorder[] = { 1, 2, -3, 4 };

static float yx[] = { 0.f, 0.f, -.5f, .5f };

static float yy[] = { 0.f, .6f, 1.f, 1.f };

static int yorder[] = { 1, 2, 3, -2, 4 };

static float zx[] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)	h -= 6.;
	while (h < 0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:

	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.;			// red, green, blue
	switch ((int)i)
	{
		case 0:
			r = v;	g = t;	b = p;
			break;

		case 1:
			r = q;	g = v;	b = p;
			break;

		case 2:
			r = p;	g = v;	b = t;
			break;

		case 3:
			r = p;	g = q;	b = v;
			break;

		case 4:
			r = t;	g = p;	b = v;
			break;

		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit(float v[3])
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
