//Author:		David A. Hernandez II
//Date:			15 October 2017
//Class:		3480 Computer Graphics
//Assignment:	Project
//This is a <project>.
//
//May have to install the following packages depending on your Linux distribution:
//
//	libx11-dev
//	libglew1.6
//	libglew1.6-dev
//You will have to run this on your local machine rather than Sleipnir in order to use the appropriate drivers

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include </usr/include/AL/alut.h>
#include "ppm.h"
#include "fonts.h"
//#include "log.h"
using namespace std;

#define rnd() ((Flt)rand() / (Flt)RAND_MAX)
#define PI 3.14159265358979323846264338327950
#define GRAVITY 0.1
#define MAX_POINTS 10000
#define MAX_CUBES 32
#define vecComb(A,a,B,b,c)	(c)[0] = (A) * (a)[0] + (B) * (b)[0]; \
							(c)[1] = (A) * (a)[1] + (B) * (b)[1]; \
							(c)[2] = (A) * (a)[2] + (B) * (b)[2]

//Variable Types
typedef float Flt;
typedef Flt	  Vec[3];

//Function Definitions
void initXWindows(void);
void initOpenGL(void);
void init_texture(void);
void cleanupXWindows(void);
void checkResize(XEvent *);
void checkMouse(XEvent *);
void checkKeys(XEvent *);
void clearScreen();
void physics(void);
void render(void);
void showMenu();

//vector Functions...
Flt vecLength(Vec);
Flt vecDot(Vec, Vec);
void vecZero(Vec);
void vecCopy(Vec, Vec);
void vecNegate(Vec);
void vecMake(Flt, Flt, Flt, Vec);
void vecSub(Vec, Vec, Vec);
void vecAdd(Vec, Vec, Vec);
void vecCross(Vec, Vec);
void vecNormalize(Vec);

struct Point {
	Flt x, y, z;
};

struct Cube {
	int npoints;
	Point center;
	Point vert[MAX_POINTS];
	Flt height, width, length, offset;
	Vec verts, normal, color, specular;
	Vec position, location, angle, rotation;
	bool inside, isSpec, isMoving;
	int	scaling[3], axis[3];

	Cube() {
		//height = width = length = 50.0;
		height = 20.0;
		width  = 50.0;
		length = 50.0;
		offset = 10.0;
		inside = isSpec = isMoving = false;
		vecMake(1.0, 1.0,   1.0, verts);
		vecMake(0.0, 0.0,   0.0, angle);
		vecMake(1.0, 0.0,   0.0, color);
		vecMake(0.0, 0.0, -10.0, position);
		vecMake(0.0, 0.0,   0.0, location);
		vecMake(0.9, 0.9,   0.9, specular);
		vecMake(0.0, 0.0,   0.0, rotation);
	}
};

class Camera {
	public:
		Vec from, at, up, eye;
		Flt angle;
		Camera() {
			vecMake(5.0, 10.0,  15.0, from);
			vecMake(0.0,  0.0,-100.0, at);
			vecMake(0.0,  1.0,   0.0, up);			
			vecMake(0.0,  0.0,   0.0, eye);
			angle = 30.0;
		}
} cam;

class Light {
	public:
		Vec amb, diff, spec, pos;
		Light() {
			vecMake(  0.8,   0.8,   0.8, amb);
			vecMake(  0.8,   0.8,   0.8, diff);
			vecMake(  0.9,   0.9,   0.9, spec);
			vecMake(100.0, 100.0, 100.0, pos);
		}
} light;

//X11 Variables/Function Definitions
Display		*dpy;
Window		 win;
GLXContext	 glc;
int done = 0;
int mode = 0;
int xres = 800;
int yres = 600;

//Object Definitions
Cube cube;
Cube cubes[MAX_CUBES];

//OpenAL Functions
//extern ALuint alSource[];

/*
//Physics Timers
struct timespec tStart, tCurrent, tPause;
const double	physicsRate			= 1.0 / 30.0;
const double	oobillion			= 1.0 /  1e9;
double			physicsCountdown	= 0.0;
double			timeSpan			= 0.0;
unsigned int	upause				= 0;

double timeDiff(struct timespec *start, struct timespec *end)
{
return (double)(end->tv_sec - start->tv_sec) +
(double)(end->tv_nsec - start->tv_nsec) * oobillion;
}

void timeCopy(struct timespec *dest, struct timespec *src)
{
memcpy(dest, src, sizeof(struct timespec));
}

//Constant Definitions
const int	MAXOBJECTS	 =   300;
const int	MAXPARTICLES =	5000;
const float timeslice	 =  1.0f;
const float gravity		 = -0.2f;
*/

//MAIN 
//-----------------------------------------------------------------------------
int main(void)
{
	initXWindows();
	initOpenGL();
	glEnable(GL_TEXTURE_2D);	//Necessary for allowing fonts
	initialize_fonts();
	//init_texture();
	//initSounds();
	//clock_gettime(CLOCK_REALTIME, &tPause);
	//clock_gettime(CLOCK_REALTIME, &tStart);

	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			checkResize(&e);
			checkMouse(&e);
			checkKeys(&e);
		}

		//Apply physics at a consistent rate.
		//clock_gettime(CLOCK_REALTIME, &tCurrent);	//Get current time
		//timeSpan = timeDiff(&tStart, &tCurrent);	//Determine time since start of instance
		//timeCopy(&tStart, &tCurrent);				//Save current time as starting time
		//physicsCountdown = musicSpeed;			//Add timespace to countdown

		//Check if countdown extends past physics rate
		//If true, apply physics, reduce the countdown by physics rate, and break when countdown < physics rate
		/*Otherwise, don't apply physics to this frame
		  while (physicsCountdown >= physicsRate) {
		  physics();
		  physicsCountdown -= physicsRate;
		  }
		  */

		physics();
		render();		
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	cleanup_fonts();
	//cleanSounds();
	//deletePPM();
	//deleteTitle();
	return 0;
}

//XWindows Functions for OpenGL
//-----------------------------------------------------------------------------
void cleanupXWindows(void) 
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void set_title(void) 
{
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "CS3480 - EQ Spectrum Analyzer Project");
}

void setupScreen(const int w, const int h)
{
	xres = w;
	yres = h;
}

void initXWindows(void)
{
	Window root;
	GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;

	setupScreen(xres, yres);
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) { printf("Unable to open display or connect to X server\n"); exit(EXIT_FAILURE); }
	root = DefaultRootWindow(dpy);
	vi	 = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) { printf("Unable to find appropriate visual\n"); exit(EXIT_FAILURE); }

	cmap			= XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap	= cmap;
	swa.event_mask	= ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | SubstructureNotifyMask;

	win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

//Resizes the window then necessary
void reshape_window(int width, int height) 
{
	setupScreen(width, height);
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat) width / (GLfloat) height, 1.0, 20.0);
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	set_title();
}

//OpenGL Initialization
void initOpenGL(void) 
{
	//glViewport(0, 0, xres, yres);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);	

	//Enable to make material colors the same as vert colors
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light.amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light.diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light.spec);
	glLightfv(GL_LIGHT0, GL_POSITION, light.pos);

	glEnable(GL_LIGHT0);

	//To set fog parameters;
	//glFog{if}{v} (GLenum pname, TYPE param);	
}

//ConfigureNotify is sent by the server when the window is resized
void checkResize(XEvent *e)
{
	if (e->type != ConfigureNotify) return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != xres || xce.height != yres) { reshape_window(xce.width, xce.height); }
}

void checkMouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	if (e->type == ButtonRelease) { return; }
	//Checks if Left(1) or Right(3) Buttons are pressed
	if (e->type == ButtonPress) {
		if (e->xbutton.button == 1) { }
		if (e->xbutton.button == 3) { }
	}
	//Check when mouse moves
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
	}
}

//Check if there is input from the keyboard
void checkKeys(XEvent *e)
{	
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch(key) {
			case XK_1:	mode = 1; initOpenGL(); break;							

			case XK_2:	mode = 2; 
						cube.axis[1] ^= 1;
						if (cube.axis[1] == 1) {
							cube.isMoving	= true;
							cube.scaling[1] = 1;
						}
						initOpenGL(); break;

			case XK_3:	mode = 3; initOpenGL(); break;
			case XK_4:	mode = 4; break;
			case XK_5:	mode = 5; break;
			case XK_6:	mode = 6; break;
			case XK_7:	mode = 7; break;

			/* Cube is using inverted rotation on key press */
			case XK_Left:	
							cube.rotation[1] -= 2.0;
							if (cube.rotation[1] <= 0.0)  { cube.rotation[1] += 360.0; } break;			
			case XK_Right:				
							cube.rotation[1] += 2.0;
							if (cube.rotation[1] > 360.0) { cube.rotation[1] -= 360.0; } break;

			case XK_Up:	
							cube.rotation[0] -= 2.0;
							if (cube.rotation[0] <= 0.0)  { cube.rotation[0] += 360.0; } break;
			case XK_Down:	
							cube.rotation[0] += 2.0;
							if (cube.rotation[0] > 360.0) {	cube.rotation[0] -= 360.0; } break;

			/* Not working */
			case XK_plus:	cube.position[2] += 1.0; break;
			case XK_minus:	cube.position[2] -= 1.0; break;

			case XK_Escape:	mode = 0; done = 1; break;
		}
	}
}

void physics(void) 
{
	/* Check to see if music is playing
	 * If so, render EQ moving
	 * 
	 * if (music.isPlaying)
	 *		moveEQ();
	 *
	 */
}

void makeCubes() 
{
	

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();


	/*	
		glPushMatrix();
		glTranslatef();
		glTranslatef();
	*/

	for (int i = 0; i < MAX_CUBES; i++) {

	}
}

//Constructing Objects Function
//-----------------------------------------------------------------------------
void lightedCube()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();	

	glTranslatef(cube.position[0], cube.position[1], cube.position[2]);
	glRotatef(cube.rotation[0], 1.0f, 0.0f, 0.0f);
	glRotatef(cube.rotation[1], 0.0f, 1.0f, 0.0f);
	glRotatef(cube.rotation[2], 0.0f, 0.0f, 1.0f);

	//Anchors the cube to the bottom when it scales
	glTranslatef(cube.location[0], cube.location[1], cube.location[2]);
	glColor3f(cube.color[0], cube.color[1], cube.color[2]);	
	glScalef(1.0f, cube.location[1], 1.0f);
	

	//If the cube is scaling upwards
	if (cube.isMoving) {
		if (cube.scaling[1] == 1) {	
			cube.location[1] += 0.03f;
			if (cube.location[1] > 2.0f) { cube.location[1] = 2.0f; cube.scaling[1] = 0; }
		}
		if (cube.scaling[1] == 0) {	
			cube.location[1] -= 0.03f;
			if (cube.location[1] < 0.25f) { cube.location[1] = 0.25f; cube.scaling[1] = 1;	}
		}		
	} 

	//If the cube is scaling downwards
	if (cube.scaling[1] == 0 && cube.isMoving == false) {
		cube.location[1] -= 0.03f;
		if (cube.location[1] < 0.25f) {
			cube.location[1] = 0.25f;
			cube.scaling[1] = 1;
			cube.isMoving = true;
		}		
	}

	if (cube.axis[1] == 0) {
		cube.scaling[1] = 0;
		cube.isMoving = false;
	}

	glBegin(GL_QUADS);
	//Top of cube
	glNormal3f(  0.0f,   1.0f,   0.0f);
	glVertex3f( cube.verts[0],  cube.verts[1], -cube.verts[2]);
	glVertex3f(-cube.verts[0],  cube.verts[1], -cube.verts[2]);
	glVertex3f(-cube.verts[0],  cube.verts[1],  cube.verts[2]);
	glVertex3f( cube.verts[0],  cube.verts[1],  cube.verts[2]);

	//Bottom of cube
	glNormal3f(  0.0f,  -1.0f,   0.0f);

	glVertex3f( cube.verts[0], -cube.verts[1],  cube.verts[2]);
	glVertex3f(-cube.verts[0], -cube.verts[1],  cube.verts[2]);
	glVertex3f(-cube.verts[0], -cube.verts[1], -cube.verts[2]);
	glVertex3f( cube.verts[0], -cube.verts[1], -cube.verts[2]);

	//Front of cube	
	glNormal3f(  0.0f,   0.0f,   1.0f);
	glVertex3f( cube.verts[0],  cube.verts[1],  cube.verts[2]);
	glVertex3f(-cube.verts[0],  cube.verts[1],  cube.verts[2]);
	glVertex3f(-cube.verts[0], -cube.verts[1],  cube.verts[2]);
	glVertex3f( cube.verts[0], -cube.verts[1],  cube.verts[2]);

	//Back of cube
	glNormal3f(  0.0f,   0.0f,  -1.0f);
	glVertex3f( cube.verts[0], -cube.verts[1], -cube.verts[2]);
	glVertex3f(-cube.verts[0], -cube.verts[1], -cube.verts[2]);
	glVertex3f(-cube.verts[0],  cube.verts[1], -cube.verts[2]);
	glVertex3f( cube.verts[0],  cube.verts[1], -cube.verts[2]);
		
	//Left of Cube
	glNormal3f(  -1.0f,   0.0f,  0.0f);
	glVertex3f(-cube.verts[0],  cube.verts[1],  cube.verts[2]);
	glVertex3f(-cube.verts[0],  cube.verts[1], -cube.verts[2]);
	glVertex3f(-cube.verts[0], -cube.verts[1], -cube.verts[2]);
	glVertex3f(-cube.verts[0], -cube.verts[1],  cube.verts[2]);
	
	//Right of Cube
	glNormal3f(  1.0f,   0.0f,   0.0f);
	glVertex3f( cube.verts[0],  cube.verts[1], -cube.verts[2]);
	glVertex3f( cube.verts[0],  cube.verts[1],  cube.verts[2]);
	glVertex3f( cube.verts[0], -cube.verts[1],  cube.verts[2]);
	glVertex3f( cube.verts[0], -cube.verts[1], -cube.verts[2]);
	
	glEnd();
	glPopMatrix();
}

//Vector Functions
//-----------------------------------------------------------------------------
Flt vecLength(Vec vec) {					return sqrt(vecDot(vec, vec));}
Flt vecDot(Vec u, Vec v) {					return (u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);}
void vecZero(Vec v) {						v[0] =   0.0; v[1] =   0.0; v[2] =   0.0;}
void vecCopy(Vec s, Vec d) {				d[0] =  s[0]; d[1] =  s[1]; d[2] =  s[2];}
void vecNegate(Vec v) {						v[0] = -v[0]; v[1] = -v[1]; v[2] = -v[2];}
void vecMake(Flt a, Flt b, Flt c, Vec v) {	v[0] =     a; v[1] =     b; v[2] =     c;}
void vecSub(Vec u, Vec v, Vec d) {			d[0] = u[0]-v[0]; d[1] = u[1]-v[1]; d[2] = u[2]-v[2];}
void vecAdd(Vec u, Vec v, Vec d) {			d[0] = u[0]+v[0]; d[1] = u[1]+v[1]; d[2] = u[2]+v[2];}
void vecCross(Vec u, Vec v, Vec d) {		d[0] = u[1]*v[2]-v[1]*u[2]; d[1] = u[2]*v[0]-v[2]*u[0]; d[2] = u[0]*v[1]-v[0]*u[1];}

void vecNormalize(Vec vec)
{
	Flt len = vecLength(vec);
	if (len == 0.0) { vecMake(1,0,0,vec); return; }
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

//Render Function
//-----------------------------------------------------------------------------
void render(void)
{
	Rect r;

	//Rendering of Cube(s)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	
	/* gluPerspective(fovY, aspect, near, far)
	 *		fovy = angle of fov in x-z plane between 0-180
	 *		aspect = ratio of the frustrum, w/h
			case XK_Right: 	cam.from[0] -= 1.0; break;
	 *		near & far = the distances between the viewpoint and the clipping planes along the negative x-axis
	 * */
	gluPerspective(45.0f, (GLfloat) xres / (GLfloat) yres, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();		
	gluLookAt(5, 10, 15, 0, 0, 0, 0, 1, 0);
	glEnable(GL_LIGHTING);
	lightedCube();	

	//GL Rendering of Menu as a 2D Rectangle
	glMatrixMode(GL_PROJECTION);	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);		glLoadIdentity();
	glDisable(GL_LIGHTING);
	glOrtho(0, xres, 0, yres, -1, 1);

	r.left = 10;
	r.center = 0;
	r.bot = yres - 20;
	ggprint8b(&r, 16, 0x00887766, "CS3480 Spectrum Analyzer Project");
	ggprint8b(&r, 16, 0x008877aa, "1 - Scale Cube (X-Axis)");
	ggprint8b(&r, 16, 0x008877aa, "2 - Scale Cube (Y-Axis)");
	ggprint8b(&r, 16, 0x008877aa, "3 - Scale Cube (Z-Axis)");
	ggprint8b(&r, 16, 0x008877aa, "P - Play");
	ggprint8b(&r, 16, 0x008877aa, "ESC - Quit");

}
