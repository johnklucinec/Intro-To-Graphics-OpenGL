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
//	Author:			John Klucinec

bool	Frozen;

// title of these windows:
const char* WINDOWTITLE = "OpenGL / Keyframes  -- John Klucinec";
const char* GLUITITLE = "User Interface Window";

const int GLUITRUE = true;
const int GLUIFALSE = false;
const int ESCAPE = 0x1b;
const int INIT_WINDOW_SIZE = 600;
const float BOXSIZE = 2.f;
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;
const float MINSCALE = 0.05f;
const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;
const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;
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
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values
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
// this order must match the menu order
const GLfloat Colors[][3] =
{
	{ .90, 0.25, 0.25 },		// red
	{ 1., 1., 0. },		// yellow
	{ 0.30, 0.90, 0.60 },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 0.60, 0.25, 0.90 },		// magenta
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

void			Axes(float);
void			HsvRgb(float[3], float[3]);
void			Cross(float[3], float[3], float[3]);
float			Dot(float[3], float[3]);
float			Unit(float[3], float[3]);
float			Unit(float[3]);


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
	float r = (float)rand();               // 0 - RAND_MAX
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

// Quaternion structure
struct Quaternion
{
	float w, x, y, z;
};

// Create a quaternion from axis-angle
Quaternion fromAxisAngle(float angle, float ax, float ay, float az)
{
	float halfAngle = angle / 2;
	float s = sin(halfAngle);
	Quaternion q;
	q.w = cos(halfAngle);
	q.x = ax * s;
	q.y = ay * s;
	q.z = az * s;
	return q;
}

// Normalize a quaternion
Quaternion normalize(Quaternion q)
{
	float magnitude = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	q.w /= magnitude;
	q.x /= magnitude;
	q.y /= magnitude;
	q.z /= magnitude;
	return q;
}

// Multiply two quaternions
Quaternion multiply(Quaternion q1, Quaternion q2)
{
	Quaternion q;
	q.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	q.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	q.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
	q.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
	return q;
}

// Rotate a vector using a quaternion
void rotateVector(float& vx, float& vy, float& vz, Quaternion q)
{
	Quaternion qVec = { 0, vx, vy, vz };
	Quaternion qConjugate = { q.w, -q.x, -q.y, -q.z };

	// q * vector * q_conjugate
	Quaternion temp = multiply(q, qVec);
	Quaternion rotated = multiply(temp, qConjugate);

	vx = rotated.x;
	vy = rotated.y;
	vz = rotated.z;
}

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
#include "osucone.cpp"
#include "osutorus.cpp"
//#include "bmptotexture.cpp"
#include "loadobjfile.cpp"
#include "keytime.cpp"
//#include "glslprogram.cpp"
//#include "vertexbufferobject.cpp"

// OBJ FILES
int WheatlyDL;
int CubeDL;
int PistonDL;
int PistonHeadDL;

// Grid
int GridDL;

const int MSEC = 10000;

// main program:
int
main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	InitGraphics();
	InitLists();
	Reset();
	InitMenus();
	glutSetWindow(MainWindow);
	glutMainLoop();
	return 0;
}

#define XSIDE	40				// length of the x side of the grid
#define X0      (-XSIDE/2.)		// where one side starts
#define NX		1000				// how many points in x
#define DX		( XSIDE/(float)NX )	// change in x between the points

#define YGRID	0.f				// y-height of the grid

#define ZSIDE	40				// length of the z side of the grid
#define Z0      (-ZSIDE/2.)		// where one side starts
#define NZ		1000				// how many points in z
#define DZ		( ZSIDE/(float)NZ )	// change in z between the points


// Global variables to store the current position
GLfloat CurrentX = 0.0f;
GLfloat CurrentY = 2.0f;
GLfloat CurrentZ = 0.0f;

GLfloat SpotlightDirX = 1.0f;
GLfloat SpotlightDirY = 0.0f;
GLfloat SpotlightDirZ = 0.0f;

float horizontalRotation = 0.0f;
float verticalRotation = 0.0f;

// Global variables to store the current position
GLfloat LightX = 10.0f;
GLfloat LightY = YGRID + 200.0f;
GLfloat LightZ = 50.0f;

// Light Properties
const float ColorWhite[] = { 1.0f, 1.0f, 1.0f };
const float* CurrentColor = ColorWhite;
float* WheatlyLight = const_cast<float*>(Colors[CYAN]);

GLfloat LookX = 0;
GLfloat LookY = 4;
GLfloat EyeX = 0;

// Block Colors
float* BlockColor1 = const_cast<float*>(Colors[RED]);
float* BlockColor2 = const_cast<float*>(Colors[MAGENTA]);
float* BlockColor3 = const_cast<float*>(Colors[GREEN]);

float PistonX = 0.0f;

// Global Keytimes
Keytimes Xpos1, Xrot1, Yrot1, Xlook, Ylook, Xeye, Ypos1, Zpos1, Xpos2, Red1, Red2, Red3, Green1, Green2, Green3, Blue1, Blue2, Blue3, Red4, Green4, Blue4;

void
Animate()
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	/* All this stuff is used for updating where the eye spotlight is looking */
	// Convert degrees to radians
	float horizontalRadians = horizontalRotation * (M_PI / 180.0f);
	float verticalRadians = verticalRotation * (M_PI / 180.0f);

	// Create quaternions for horizontal and vertical rotations
	Quaternion horizontalQuat = fromAxisAngle(horizontalRadians, 0.0f, 1.0f, 0.0f);
	Quaternion verticalQuat = fromAxisAngle(verticalRadians, 0.0f, 0.0f, 1.0f);
	Quaternion finalQuat = multiply(verticalQuat, horizontalQuat);
	finalQuat = normalize(finalQuat);

	// Initial direction of light
	float dirX = 1.0f, dirY = 0.0f, dirZ = 0.0f;

	// Rotate the direction vector
	rotateVector(dirX, dirY, dirZ, finalQuat);

	// Update spotlight direction
	SpotlightDirX = dirX;
	SpotlightDirY = dirY;
	SpotlightDirZ = dirZ;
	/* */

	// force a call to Display( ) next time it is convenient:
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
Display()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// Animation Time Stuff
	int msec = glutGet(GLUT_ELAPSED_TIME) % MSEC;
	float nowTime = (float)msec / 1000.;

	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif

	// specify shading to be flat:
	glShadeModel(GL_SMOOTH);

	// set the viewport to be a square centered in the window:
	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);

	// set the viewing volume:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (NowProjection == ORTHO)
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	else
		gluPerspective(70.f, 1.f, 0.1f, 1000.f);

	if (!Frozen) // Stop program from updating
	{
		// Wheatly Positioning
		CurrentX = Xpos1.GetValue(nowTime);
		CurrentY = Ypos1.GetValue(nowTime);
		CurrentZ = Zpos1.GetValue(nowTime);

		// Wheatly Angles (pain)
		horizontalRotation = Xrot1.GetValue(nowTime);
		verticalRotation = Yrot1.GetValue(nowTime);

		PistonX = Xpos2.GetValue(nowTime);

		// Update Eye Look Pos
		LookX = Xlook.GetValue(nowTime);
		LookY = Ylook.GetValue(nowTime);
		EyeX = Xeye.GetValue(nowTime);

		// Update Block Color
		float color1[] = { Red1.GetValue(nowTime), Green1.GetValue(nowTime), Blue1.GetValue(nowTime) };
		BlockColor1 = color1;

		float color2[] = { Red2.GetValue(nowTime), Green2.GetValue(nowTime), Blue2.GetValue(nowTime) };
		BlockColor2 = color2;

		float color3[] = { Red3.GetValue(nowTime), Green3.GetValue(nowTime), Blue3.GetValue(nowTime) };
		BlockColor3 = color3;

		// Update wheatly eye color
		float color4[] = { Red4.GetValue(nowTime), Green4.GetValue(nowTime), Blue4.GetValue(nowTime) };
		WheatlyLight = color4;
	}

	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:
	gluLookAt(EyeX, 5.f, 10.f, LookX, LookY, 0.f, 0.f, 1.f, 0.f);

	// rotate and scale the scene
	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);
	if (Scale < MINSCALE) Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);



	glEnable(GL_LIGHT0);	// Eye Spotlight
	glEnable(GL_LIGHT1);

	SetSpotLight(GL_LIGHT0, CurrentX, CurrentY, CurrentZ,
				 SpotlightDirX, SpotlightDirY, SpotlightDirZ,
				 WheatlyLight[0], WheatlyLight[1], WheatlyLight[2]);

	// Draw a small sphere to represent the light source
	glPushMatrix();
	glTranslatef(CurrentX, CurrentY, CurrentZ);
	glDisable(GL_LIGHTING);
	glColor3f(WheatlyLight[0], WheatlyLight[1], WheatlyLight[2]);
	OsuSphere(0.36f, 10, 10);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	SetPointLight(GL_LIGHT1, LightX, LightY, LightZ, CurrentColor[0], CurrentColor[1], CurrentColor[2]);

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

	// Enable the lighting stuff
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.f);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.f);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.f);

	// Wheatly
	glPushMatrix();
	SetMaterial(0.25, 0.25, 0.25, 5.0);
	glTranslatef(CurrentX, CurrentY, CurrentZ);
	glRotatef(verticalRotation, 0.0f, 0.0f, 1.0f);   // Rotate around X-axis (vertical)
	glRotatef(horizontalRotation, 0.0f, 1.0f, 0.0f); // Rotate around Y-axis (horizontal)
	glScalef(0.025f, 0.025f, 0.025f);
	glCallList(WheatlyDL);
	glPopMatrix();

	// Cube 1
	glPushMatrix();
	SetMaterial(BlockColor1[0], BlockColor1[1], BlockColor1[2], 10.0);
	glTranslatef(-2.5f, 4.0f, 4.5f);
	glScalef(0.425f, 0.425f, 0.425f);
	glCallList(CubeDL);
	glPopMatrix();

	// Cube 2
	glPushMatrix();
	SetMaterial(BlockColor2[0], BlockColor2[1], BlockColor2[2], 10.0);
	glTranslatef(2.0f, 1.5f, 0.0f);
	glScalef(0.425f, 0.425f, 0.425f);
	glCallList(CubeDL);
	glPopMatrix();

	// Cube 3
	glPushMatrix();
	SetMaterial(BlockColor3[0], BlockColor3[1], BlockColor3[2], 10.0);
	glTranslatef(1.5f, 6.0f, -1.0f);
	glScalef(0.425f, 0.425f, 0.425f);
	glCallList(CubeDL);
	glPopMatrix();

	// Piston
	glPushMatrix();
	SetMaterial(0.0, 0.0, 0.0, 100.0);
	glTranslatef(7.0f, 4.0f, 0.0f);
	glRotatef(90, 0.0f, 0.0f, 1.0f);
	glPushMatrix();
	glTranslatef(0.0f, PistonX, 0.0f);
	glCallList(PistonHeadDL);
	glPopMatrix();
	glCallList(PistonDL);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(GridDL);
	glTranslatef(0.0f, 5.0f, -5.0f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	glCallList(GridDL);
	glTranslatef(7.0f, 2.6f, 0.0f);
	glRotatef(90, 0.0f, 0.0f, 1.0f);
	glCallList(GridDL);
	glPopMatrix();

#ifdef DEMO_Z_FIGHTING
	if (DepthFightingOn != 0)
	{
		glPushMatrix();
		glRotatef(90.f, 0.f, 1.f, 0.f);
		glCallList(BoxList);
		glPopMatrix();
	}
#endif

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3f(0.f, 1.f, 1.f);

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, 100.f, 0.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1.f, 1.f, 1.f);

	// swap the double-buffered framebuffers:
	glutSwapBuffers();
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

// initialize the glut and OpenGL libraries:
void
InitGraphics()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// Wheatly Animation
	Xpos1.Init();
	Xpos1.AddTimeValue(0.0, -6.000);
	Xpos1.AddTimeValue(1.0, -4);
	Xpos1.AddTimeValue(2.0, -4);
	Xpos1.AddTimeValue(3.0, 0.0);
	Xpos1.AddTimeValue(4.0, 0.0);
	Xpos1.AddTimeValue(5.0, 2);
	Xpos1.AddTimeValue(6.0, 2);
	Xpos1.AddTimeValue(7.0, 4.8);
	Xpos1.AddTimeValue(9.0, 4.8);
	Xpos1.AddTimeValue(9.3, 4.8);
	Xpos1.AddTimeValue(9.5, 0);
	Xpos1.AddTimeValue(10.0, -6.000);

	Ypos1.Init();
	Ypos1.AddTimeValue(0.0, 1.000);
	Ypos1.AddTimeValue(1.0, 4.000);
	Ypos1.AddTimeValue(2.0, 4.000);
	Ypos1.AddTimeValue(3.0, 2.000);
	Ypos1.AddTimeValue(4.0, 2.000);
	Ypos1.AddTimeValue(5.0, 7.000);
	Ypos1.AddTimeValue(6.0, 7.000);
	Ypos1.AddTimeValue(7.0, 4.000);
	Ypos1.AddTimeValue(9.0, 4.000);
	Ypos1.AddTimeValue(9.3, 4.000);
	Ypos1.AddTimeValue(9.5, 8.000);
	Ypos1.AddTimeValue(10.0, 1.000);

	Zpos1.Init();
	Zpos1.AddTimeValue(0.0, 0.000);
	Zpos1.AddTimeValue(1.0, 5);
	Zpos1.AddTimeValue(2.0, 5);
	Zpos1.AddTimeValue(3.0, 0.0);
	Zpos1.AddTimeValue(4.0, 0.0);
	Zpos1.AddTimeValue(5.0, -3);
	Zpos1.AddTimeValue(6.0, -3);
	Zpos1.AddTimeValue(7.0, 0);
	Zpos1.AddTimeValue(9.0, 0);
	Zpos1.AddTimeValue(9.3, 0);
	Zpos1.AddTimeValue(9.5, 0);
	Zpos1.AddTimeValue(10.0, 0.000);

	Xrot1.Init();
	Xrot1.AddTimeValue(0.0, 0.000);
	Xrot1.AddTimeValue(1.0, 30.000);
	Xrot1.AddTimeValue(2.0, 30.000);
	Xrot1.AddTimeValue(3.0, -10.000);
	Xrot1.AddTimeValue(4.0, -10.000);
	Xrot1.AddTimeValue(5.0, -50.000);
	Xrot1.AddTimeValue(6.0, -50.000);
	Xrot1.AddTimeValue(7.0, 0.000);
	Xrot1.AddTimeValue(9.0, 0.000);
	Xrot1.AddTimeValue(9.3, 0.000);
	Xrot1.AddTimeValue(10.0, 360.000);

	Yrot1.Init();
	Yrot1.AddTimeValue(0.0, 0.000);
	Yrot1.AddTimeValue(1.0, -6.000);
	Yrot1.AddTimeValue(2.0, -6.000);
	Yrot1.AddTimeValue(3.0, -5.000);
	Yrot1.AddTimeValue(4.0, -5.000);
	Yrot1.AddTimeValue(5.0, -90.000);
	Yrot1.AddTimeValue(6.0, -90.000);
	Yrot1.AddTimeValue(7.0, 0.000);
	Yrot1.AddTimeValue(9.0, 0.000);
	Yrot1.AddTimeValue(9.3, 0.000);
	Yrot1.AddTimeValue(10.0, 360.000);

	// Piston Open/Close
	Xpos2.Init();
	Xpos2.AddTimeValue(0.0, -1.0);
	Xpos2.AddTimeValue(9.2, -1.0);
	Xpos2.AddTimeValue(9.3, 0.0);
	Xpos2.AddTimeValue(9.4, 0.0);
	Xpos2.AddTimeValue(10.0, -1.0);

	// Camera Movement
	Xeye.Init();
	Xeye.AddTimeValue(0.0, -6);
	Xeye.AddTimeValue(1.5, -5);
	Xeye.AddTimeValue(2.5, -3);
	Xeye.AddTimeValue(4.5, 0);
	Xeye.AddTimeValue(7.5, 4);
	Xeye.AddTimeValue(9.0, 4);
	Xeye.AddTimeValue(9.4, 4);
	Xeye.AddTimeValue(10.0, -6);

	Ylook.Init();
	Ylook.AddTimeValue(0.0, 1.000);
	Ylook.AddTimeValue(1.0, 4.000);
	Ylook.AddTimeValue(2.0, 4.000);
	Ylook.AddTimeValue(3.0, 2.000);
	Ylook.AddTimeValue(4.0, 2.000);
	Ylook.AddTimeValue(5.0, 7.000);
	Ylook.AddTimeValue(6.0, 7.000);
	Ylook.AddTimeValue(7.0, 4.000);
	Ylook.AddTimeValue(9.0, 4.000);
	Ylook.AddTimeValue(9.3, 4.000);
	Ylook.AddTimeValue(9.5, 7.000);
	Ylook.AddTimeValue(10.0, 1.000);

	Xlook.Init();
	Xlook.AddTimeValue(0.0, -6.000);
	Xlook.AddTimeValue(1.0, -4);
	Xlook.AddTimeValue(2.0, -4);
	Xlook.AddTimeValue(3.0, 0.0);
	Xlook.AddTimeValue(4.0, 0.0);
	Xlook.AddTimeValue(5.0, 2);
	Xlook.AddTimeValue(6.0, 2);
	Xlook.AddTimeValue(7.0, 3);
	Xlook.AddTimeValue(9.0, 3);
	Xlook.AddTimeValue(9.3, 3);
	Xlook.AddTimeValue(9.5, 0);
	Xlook.AddTimeValue(10.0, -6.000);

	// Change Block 1 Color
	Red1.Init();
	Red1.AddTimeValue(0.0, Colors[RED][0]);
	Red1.AddTimeValue(1.0, Colors[RED][0]);
	Red1.AddTimeValue(2.0, ColorWhite[0]);
	Red1.AddTimeValue(9.5, ColorWhite[0]);
	Red1.AddTimeValue(10, Colors[RED][0]);

	Green1.Init();
	Green1.AddTimeValue(0.0, Colors[RED][1]);
	Green1.AddTimeValue(1.0, Colors[RED][1]);
	Green1.AddTimeValue(2.0, ColorWhite[1]);
	Green1.AddTimeValue(9.5, ColorWhite[1]);
	Green1.AddTimeValue(10, Colors[RED][1]);

	Blue1.Init();
	Blue1.AddTimeValue(0.0, Colors[RED][2]);
	Blue1.AddTimeValue(1.0, Colors[RED][2]);
	Blue1.AddTimeValue(2.0, ColorWhite[2]);
	Blue1.AddTimeValue(9.5, ColorWhite[2]);
	Blue1.AddTimeValue(10, Colors[RED][2]);

	// Change Block 2 Color
	Red2.Init();
	Red2.AddTimeValue(0.0, Colors[MAGENTA][0]);
	Red2.AddTimeValue(3.0, Colors[MAGENTA][0]);
	Red2.AddTimeValue(4.0, ColorWhite[0]);
	Red2.AddTimeValue(9.5, ColorWhite[0]);
	Red2.AddTimeValue(10, Colors[MAGENTA][0]);

	Green2.Init();
	Green2.AddTimeValue(0.0, Colors[MAGENTA][1]);
	Green2.AddTimeValue(3.0, Colors[MAGENTA][1]);
	Green2.AddTimeValue(4.0, ColorWhite[1]);
	Green2.AddTimeValue(9.5, ColorWhite[1]);
	Green2.AddTimeValue(10, Colors[MAGENTA][1]);

	Blue2.Init();
	Blue2.AddTimeValue(0.0, Colors[MAGENTA][2]);
	Blue2.AddTimeValue(3.0, Colors[MAGENTA][2]);
	Blue2.AddTimeValue(4.0, ColorWhite[2]);
	Blue2.AddTimeValue(9.5, ColorWhite[2]);
	Blue2.AddTimeValue(10, Colors[MAGENTA][2]);

	// Change Block 3 Color
	Red3.Init();
	Red3.AddTimeValue(0.0, Colors[GREEN][0]);
	Red3.AddTimeValue(5.0, Colors[GREEN][0]);
	Red3.AddTimeValue(6.0, ColorWhite[0]);
	Red3.AddTimeValue(9.5, ColorWhite[0]);
	Red3.AddTimeValue(10, Colors[GREEN][0]);

	Green3.Init();
	Green3.AddTimeValue(0.0, Colors[GREEN][1]);
	Green3.AddTimeValue(5.0, Colors[GREEN][1]);
	Green3.AddTimeValue(6.0, ColorWhite[1]);
	Green3.AddTimeValue(9.5, ColorWhite[1]);
	Green3.AddTimeValue(10, Colors[GREEN][1]);

	Blue3.Init();
	Blue3.AddTimeValue(0.0, Colors[GREEN][2]);
	Blue3.AddTimeValue(5.0, Colors[GREEN][2]);
	Blue3.AddTimeValue(6.0, ColorWhite[2]);
	Blue3.AddTimeValue(9.5, ColorWhite[2]);
	Blue3.AddTimeValue(10, Colors[GREEN][2]);

	// Change Wheatly Eye/Light Color
	Red4.Init();
	Red4.AddTimeValue(0.0, Colors[CYAN][0]);
	Red4.AddTimeValue(1.0, Colors[CYAN][0]);
	Red4.AddTimeValue(2.0, Colors[RED][0]);
	Red4.AddTimeValue(3.0, Colors[RED][0]);
	Red4.AddTimeValue(4.0, Colors[MAGENTA][0]);
	Red4.AddTimeValue(5.0, Colors[MAGENTA][0]);
	Red4.AddTimeValue(6.0, Colors[GREEN][0]);
	Red4.AddTimeValue(9.5, Colors[GREEN][0]);
	Red4.AddTimeValue(10, Colors[CYAN][0]);

	Green4.Init();
	Green4.AddTimeValue(0.0, Colors[CYAN][1]);
	Green4.AddTimeValue(1.0, Colors[CYAN][1]);
	Green4.AddTimeValue(2.0, Colors[RED][1]);
	Green4.AddTimeValue(3.0, Colors[RED][1]);
	Green4.AddTimeValue(4.0, Colors[MAGENTA][1]);
	Green4.AddTimeValue(5.0, Colors[MAGENTA][1]);
	Green4.AddTimeValue(6.0, Colors[GREEN][1]);
	Green4.AddTimeValue(9.5, Colors[GREEN][1]);
	Green4.AddTimeValue(10, Colors[CYAN][1]);

	Blue4.Init();
	Blue4.AddTimeValue(0.0, Colors[CYAN][2]);
	Blue4.AddTimeValue(1.0, Colors[CYAN][2]);
	Blue4.AddTimeValue(2.0, Colors[RED][2]);
	Blue4.AddTimeValue(3.0, Colors[RED][2]);
	Blue4.AddTimeValue(4.0, Colors[MAGENTA][2]);
	Blue4.AddTimeValue(5.0, Colors[MAGENTA][2]);
	Blue4.AddTimeValue(6.0, Colors[GREEN][2]);
	Blue4.AddTimeValue(9.5, Colors[GREEN][2]);
	Blue4.AddTimeValue(10, Colors[CYAN][2]);

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

}


// initialize the display lists that will not change:
void
InitLists()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	// Wheatly
	WheatlyDL = glGenLists(1);
	glNewList(WheatlyDL, GL_COMPILE);
	LoadObjFile((char*)"wheatly.obj");
	glEndList();

	// Cube
	CubeDL = glGenLists(1);
	glNewList(CubeDL, GL_COMPILE);
	LoadObjFile((char*)"cube.obj");
	glEndList();


	// Piston
	PistonDL = glGenLists(1);
	glNewList(PistonDL, GL_COMPILE);
	LoadObjFile((char*)"piston.obj");
	glEndList();

	// Piston Head
	PistonHeadDL = glGenLists(1);
	glNewList(PistonHeadDL, GL_COMPILE);
	LoadObjFile((char*)"piston_head.obj");
	glEndList();

	// Grid
	GridDL = glGenLists(1);
	glNewList(GridDL, GL_COMPILE);
	SetMaterial(0.6f, 0.6f, 0.6f, 30.f);
	glNormal3f(0., 1., 0.);
	for (int i = 0; i < NZ; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < NX; j++)
		{
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
		}
		glEnd();
	}
	glEndList();

	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
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
			Frozen = !Frozen;
			if (Frozen)
				glutIdleFunc(NULL);
			else
				glutIdleFunc(Animate);
			break;

		case 'q':
		case 'Q':
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
	Frozen = false;
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
