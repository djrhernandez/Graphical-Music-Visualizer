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
#include <string>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <cmath>
#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include </usr/include/AL/alut.h>
#include <complex>
#include <valarray>
#include <vector>
#include "ppm.h"
#include "fonts.h"
#include "vectors.cpp"
#include "object.cpp"
#include "fog.cpp"
#include "camera.cpp"
#include "light.cpp"
using namespace std;

#define rnd()		((Flt)rand() / (Flt)RAND_MAX)
#define PI			3.1415926535897932384626433832795
#define PI2			PI * 2.0
#define GRAVITY		0.1
#define MAX_CUBES	32
#define MAX_POINTS	10000

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
void setupFog();
void drawFloor();
void makeCubes();
void makeReflection();

enum { 	TYPE_NONE = 0,	TYPE_FLOOR,	TYPE_CUBE,	TEX_NONE,
		TEX_CHECKER, 	TEX_GLOW,	TEX_FOG,	TEX_REFLECT
};

//X11 Variables/Function Definitions
Display		*dpy;
Window		 win;
GLXContext	 glc;
int done = 0;
int mode = 0;
int xres = 1000;
int yres = 800;

//Object Definitions
Fog 	fog;
Cube 	cube;
Cube 	cubes[MAX_CUBES];
Cube	reflected[MAX_CUBES];
GLuint	cubeList;

/* --- Definitions for Fast Fourier Transform (FFT) --- */
const int nSamples = 128;

//The function's real part, imaginary part, and amplitude
double fReal[nSamples];
double fImaginary[nSamples];
double fAmplitude[nSamples];
//The FT's real part, imaginary part, and amplitude
double FReal[nSamples];
double FImaginary[nSamples];
double FAmplitude[nSamples];

//Calculates the DFT
void DFT(int n, bool inverse, const double *gReal, const double *gImag, double *GReal, double *GImag);
//Calculates the FFT
void FFT(int n, bool inverse, const double *gReal, const double *gImag, double *GReal, double *GImag);

//Plots the function onto the screen
void plot(int y, const double *g, double scale, bool trans, Flt color[4]);

//Calculates the amplitude of the complex function
void calcAmp(int n, double *gAmp, const double *gReal, const double *gImag);


/* --- Main Function --- */
int main(void)
{
	initXWindows();
	initOpenGL();
	glEnable(GL_TEXTURE_2D);	//Necessary for allowing fonts
	initialize_fonts();
		
	/*	Generates a sine signal. Can probably extract wav file from here. */
	for(int i = 0; i < nSamples; i++) {		
		fReal[i] = fImaginary[i] = 0;
	}
	bool endloop = 0, changed = 1;
	int n = nSamples;
	double p1 = 25.0, p2 = 2.0;
	
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			checkResize(&e);
			checkMouse(&e);
			checkKeys(&e);
		}
		physics();
		render();
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	cleanup_fonts();
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
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	//Enables perspective correction for rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//Sets the blend function

	//Enable Lighting Model
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	//Enable to make material colors the same as vert colors
	glLightfv(GL_LIGHT0, GL_AMBIENT,	light.amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, 	light.diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, 	light.spec);
	glLightfv(GL_LIGHT0, GL_SHININESS,	light.shine);
	glLightfv(GL_LIGHT0, GL_POSITION,	light.pos);

	//Track material ambient and diffuse from surface color
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,		GL_AMBIENT_AND_DIFFUSE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,	light.amb);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	GLfloat mata[3] = {0.1745,		0.01175,	0.01175};
	GLfloat matb[3] = {0.61424,		0.04136,	0.04136};
	GLfloat matc[3] = {0.727811,	0.626959,	0.626959};

	glMaterialfv(GL_FRONT,	GL_AMBIENT,		mata);
	glMaterialfv(GL_FRONT,	GL_DIFFUSE,		matb);
	glMaterialfv(GL_FRONT,	GL_SPECULAR,	matc);
	glMaterialf(GL_FRONT,	GL_SHININESS,	0.6 * 128.0);

	//Sets up the display list for the cubes (WIP)
	cubeList = glGenLists(1);			//Allocates memory for the cubes
	glNewList(cubeList, GL_COMPILE);	//Begins compiling the display list
	makeCubes();						//Add commands below for drawing the cubes to the display list
	makeReflection();
	glEndList();						//Ends the display list after giving it commands

	//Colors for each cube
	Flt grad[8];
	for (int i = 0; i < 8; i++) { grad[i] = 0.0; }
	for (int i = 0; i < MAX_CUBES; i++) {
		switch(i % 8) {
				case 0:
						vecMake(1.00, 0.00 + grad[0], 0.00, cubes[i].color);
						vecMake(1.00, 0.00 + grad[0], 0.00, reflected[i].color);
//						grad[0] += 0.25;
						break;
				case 1:
						vecMake(1.00, 0.50 + grad[1], 0.00, cubes[i].color);
						vecMake(1.00, 0.50 + grad[1], 0.00, reflected[i].color);
//						grad[1] += 0.125;
						break;
				case 2: vecMake(1.00 - grad[2], 1.00, 0.00, cubes[i].color);
						vecMake(1.00 - grad[2], 1.00, 0.00, reflected[i].color);
//						grad[2] += 0.25;
						break;
				case 3: vecMake(0.00, 1.00, 0.00 + grad[3], cubes[i].color);
						vecMake(0.00, 1.00, 0.00 + grad[3], reflected[i].color);
//						grad[3] += 0.25;
						break;
				case 4: vecMake(0.00, 1.00 - grad[4], 1.00, cubes[i].color);
						vecMake(0.00, 1.00 - grad[4], 1.00, reflected[i].color);
//						grad[4] += 0.125;
						break;
				case 5: vecMake(0.00 + grad[5], 0.50 - grad[5], 1.00, cubes[i].color);
						vecMake(0.00 + grad[5], 0.50 - grad[5], 1.00, reflected[i].color);
//						grad[5] += 0.125;
						break;
				case 6: vecMake(0.50 + grad[6], 0.00, 1.00, cubes[i].color);
						vecMake(0.50 + grad[6], 0.00, 1.00, reflected[i].color);
//						grad[6] += 0.125;
						break;
				case 7: vecMake(1.00, 0.00, 1.00 - grad[7], cubes[i].color);
						vecMake(1.00, 0.00, 1.00 - grad[7], reflected[i].color);
//						grad[7] += 0.125;
						break;
				case 8: vecMake(1.00, 0.00 + grad[8], 0.50, cubes[i].color);
						vecMake(1.00, 0.00 + grad[8], 0.50, reflected[i].color);
						grad[8] += 0.20;
//						break;
			}
	}

	printf("from:\t %f\t %f\t %f\n",	cam.from[0],	cam.from[1],	cam.from[2]);
	printf("at:\t %f\t %f\t %f\n",		cam.at[0],		cam.at[1],		cam.at[2]);
	printf("up:\t %f\t %f\t %f\n\n",	cam.up[0],		cam.up[1],		cam.up[2]);

	//Sets up fog
	if (mode == 3) { setupFog(); }

	//Possible Glow/Bloom Effect
	//http://prideout.net/archive/bloom/
	//http://www.humus.name/index.php?page=3D&&start=48
	//http://www.ozone3d.net/tutorials/image_filtering_p2.php
	//https://learnopengl.com/#!Advanced-Lighting/Bloom
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
			case XK_3:	mode = 3; initOpenGL(); break;
			case XK_4:	mode = 4; break;
			case XK_5:	mode = 5; break;
			case XK_6:	mode = 6; break;
			case XK_7:	mode = 7; break;

			/* Cube is using inverted rot on key press */
			case XK_w:	//UP
						for (int i = 0; i < MAX_CUBES; i++) {
							cubes[i].rot[0] 	-= 2.0;
							reflected[i].rot[0] -= 2.0;
							if (cubes[i].rot[0] <= 0.0)  {
								cubes[i].rot[0] 	+= 360.0;
								reflected[i].rot[1] += 360.0;
							}
						} break;
						
			case XK_a:	//LEFT
						for (int i = 0; i < MAX_CUBES; i++) {
							cubes[i].rot[1] 	-= 2.0;
							reflected[i].rot[1] -= 2.0;
							if (cubes[i].rot[1] <= 0.0)  {
								cubes[i].rot[1] 	+= 360.0;
								reflected[i].rot[1] += 360.0;
							}
						} break;
						
			case XK_s:	//DOWN
						for (int i = 0; i < MAX_CUBES; i++) {
							cubes[i].rot[0] 	+= 2.0;
							reflected[i].rot[0] += 2.0;
							if (cubes[i].rot[0] > 360.0) {
								cubes[i].rot[0] 	-= 360.0;
								reflected[i].rot[1] += 360.0;
							}
						} break;
						
			case XK_d:	//RIGHT
						for (int i = 0; i < MAX_CUBES; i++) {
							cubes[i].rot[1] 	+= 2.0;
							reflected[i].rot[1] += 2.0;
							if (cubes[i].rot[1] > 360.0) {
								cubes[i].rot[1] 	-= 360.0;
								reflected[i].rot[1] -= 360.0;
							}
						} break;

			case XK_q:	//Scales the cubes
						mode = 2;
						for (int i = 0; i < MAX_CUBES; i++) {
							cubes[i].axis[1] 		^= 1;
							reflected[i].axis[1] 	^= 1;
							if (cubes[i].axis[1] == 1 && reflected[i].axis[1] == 1) {
								cubes[i].moving		= true; cubes[i].scaling[1] 	= 1;
								reflected[i].moving	= true; reflected[i].scaling[1] = 1;
							}
						}
						initOpenGL(); break;

			case XK_e:	//Enables Fog
						if (mode != 3)	{ mode = 3; initOpenGL(); }
						else			{ mode = 0; glDisable(GL_FOG); }
						break;	

			case XK_r:	//Enables wireframe rendering
						if (mode != 4)	{ mode = 4; initOpenGL(); }
						else			{ mode = 0; initOpenGL(); }
						break;
			
			case XK_minus:	
							for (int i = 0; i < MAX_CUBES; i++) {
								for (int j = 0; j < 4; j++) {
									cubes[i].pos[j][2] 		-= 1.0;
									reflected[i].pos[j][2] 	-= 1.0;
								}
							} break;
							
			/* Not working */
			case XK_plus:	
							for (int i = 0; i < MAX_CUBES; i++) {
								for (int j = 0; j < 3; j++) {
									cubes[i].pos[j][2] 		+= 1.0;
									reflected[i].pos[j][2] 	+= 1.0;
								}
							} break;
							
							
			case XK_z: 	//Zero
						for (int i = 0; i < nSamples; i++) { fReal[i] = 0; changed = 1; }	
						break;
						
			case XK_x:	//Constant
						for (int i = 0; i < nSamples; i++) { fReal[i] = p1; changed = 1; }	
						break;
						
			case XK_c: 	//Sine
						for (int i = 0; i < nSamples; i++) { fReal[i] = p1 * sin(i / p2);  changed = 1; } 
						break;

						
			case XK_Space:	endloop = 1; break;
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

void setupFog()
{
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE,		GL_EXP);
	glFogfv(GL_FOG_COLOR, 	fog.color);
	glFogf(GL_FOG_DENSITY, 	0.01);
	glHint(GL_FOG_HINT, 	GL_NICEST);

	//Depth values of fog (start --> end == near --> far)
	glFogf(GL_FOG_START,	5.0f);
	glFogf(GL_FOG_END, 		110.0f);
	//glDisable(GL_FOG);
}

void drawFloor()
{	
	float x, y, z;
	glPushMatrix();
	x = 40;	y = 1; z = 40;

	glTranslatef(cube.floor[0], cube.floor[1], cube.floor[2]);
	glScalef(1.0f, 1.0f, 1.0f);

	/*Grid for floor
	glPushMatrix();
	glBegin(GL_LINES);
	glColor3f(0.8, 0.8, 0.8);
	for(GLfloat i = -35; i <= z; i++) {
		glVertex3f(i, 1.05, z); glVertex3f( i, 1.05, -z);
		glVertex3f(x, 1.05, i); glVertex3f(-z, 1.05,  i);
	}
	glEnd();
	glPopMatrix();
	//X,Y,Z Coordinate Lines in the center of the floor
	glPushMatrix();
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 5, 0); glVertex3f(20,  5,  0);
	glColor3f(0, 1, 0); glVertex3f(0, 5, 0); glVertex3f( 0, 26,  0);
	glColor3f(0, 0, 1); glVertex3f(0, 5, 0); glVertex3f( 0,  5, 20);
	glEnd();
	glPopMatrix();
	*/

	//Rendering of floor
	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f( x, y, -z);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, y,  z);
	glVertex3f( x, y,  z);
	glEnd();
	glPopMatrix();
}

//Constructing Objects Function
//-----------------------------------------------------------------------------
/*void lightedCube()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();

	glTranslatef(cube.pos[0], cube.pos[1], cube.pos[2]);
	glRotatef(cube.rot[0], 1.0f, 0.0f, 0.0f);
	glRotatef(cube.rot[1], 0.0f, 1.0f, 0.0f);
	glRotatef(cube.rot[2], 0.0f, 0.0f, 1.0f);

	//Anchors the cube to the bottom when it scales
	glTranslatef(cube.loc[0], cube.loc[1], cube.loc[2]);
	glColor3f(cube.color[0], cube.color[1], cube.color[2]);
	glScalef(1.0f, cube.loc[1], 1.0f);

	//If the cube is scaling upwards
	if (cube.moving) {
		if (cube.scaling[1] == 1) {
			cube.loc[1] += 0.03f;
			if (cube.loc[1] > 2.0f) { cube.loc[1] = 2.0f; cube.scaling[1] = 0; }
		}
		if (cube.scaling[1] == 0) {
			cube.loc[1] -= 0.03f;
			if (cube.loc[1] < 0.25f) { cube.loc[1] = 0.25f; cube.scaling[1] = 1;	}
		}
	}

	//If the cube is scaling downwards
	if (cube.scaling[1] == 0 && cube.moving == false) {
		cube.loc[1] -= 0.03f;
		if (cube.loc[1] < 0.25f) {
			cube.loc[1] = 0.25f;
			cube.scaling[1] = 1;
			cube.moving = true;
		}
	}

	if (cube.axis[1] == 0) {
		cube.scaling[1] = 0;
		cube.moving = false;
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
}*/

void makeCubes() {
	int ncubes = 0;
	int c = 0;
	Flt offset = -12.0;
	for (int i = 0; i < MAX_CUBES; i++) {
		glPushMatrix();
		if (i < 8) {
			glTranslatef(cubes[i].pos[0][0] + 2.75*offset, cubes[i].pos[0][1], cubes[i].pos[0][2]);
		} else if (i < 16) {
			glTranslatef(cubes[i].pos[1][0] + 2.75*offset, cubes[i].pos[1][1], cubes[i].pos[1][2]);
		} else if (i < 24) {
			glTranslatef(cubes[i].pos[2][0] + 2.75*offset, cubes[i].pos[2][1], cubes[i].pos[2][2]);
		} else {
			glTranslatef(cubes[i].pos[3][0] + 2.75*offset, cubes[i].pos[3][1], cubes[i].pos[3][2]);
		}

		glRotatef(cubes[i].rot[0], 1.0f, 0.0f, 0.0f);
		glRotatef(cubes[i].rot[1], 0.0f, 1.0f, 0.0f);
		glRotatef(cubes[i].rot[2], 0.0f, 0.0f, 1.0f);

		/* X,Y,Z Orientation for each cube
		glPushMatrix();
		glBegin(GL_LINES);
		glColor3f(1, 0, 0); glVertex3f(cube.center.x, 0, 0); glVertex3f(cube.center.x + 2, 0,  0);
		glColor3f(0, 1, 0); glVertex3f(0, cube.center.y, 0); glVertex3f( 0, cube.center.y + 2,  0);
		glColor3f(0, 0, 1); glVertex3f(0, 0, cube.center.z); glVertex3f( 0,  0, cube.center.z + 2);
		glEnd();
		glPopMatrix();
		*/

		//Anchors the cube to the bottom when it scales
		glTranslatef(cubes[i].loc[0], cubes[i].loc[1], cubes[i].loc[2]);
		glColor4f(cubes[i].color[0], cubes[i].color[1], cubes[i].color[2], cubes[i].color[3]);
		glScalef(2.0f, cubes[i].loc[1], 2.0f);

		if (cubes[i].moving) {
			switch (cubes[i].scaling[1]) {
				case 0:	/* Checks if the cube is scaling downwards */
						//cubes[i].loc[1] -= ((double)rnd() * (0.1f - 0.085f));
						cubes[i].loc[1] -= ((double)rnd() * (0.2f - 0.025f));
						if (cubes[i].loc[1] < 0.25f) {
							cubes[i].loc[1] 	= 0.25f;
							cubes[i].scaling[1] = 1;
						}
						break;
				case 1:	/* Checks if the cube is scaling upwards*/
						//cubes[i].loc[1] += ((double)rnd() * (0.1f - 0.085f));
						cubes[i].loc[1] += ((double)rnd() * (0.2f - 0.025f));
						if (cubes[i].loc[1] > 2.5f) {
							cubes[i].loc[1] 	= 2.5f;
							cubes[i].scaling[1] = 0;
						}
						break;
			}
		}

		//If the cube is scaling downwards and the user wants to stop the cubes from moving
		if (cubes[i].scaling[1] == 0 && cubes[i].moving == false) {
			cubes[i].loc[1] -= ((double)rnd() * (0.2f - 0.025f));
			if (cubes[i].loc[1] < 0.25f) {
				cubes[i].loc[1] = 0.25f;
				cubes[i].scaling[1] = 1;
				cubes[i].moving = true;
			}
		}

		if (cubes[i].axis[1] == 0) {
			cubes[i].scaling[1] = 0; cubes[i].moving = false;
		}


		glBegin(GL_QUADS);
		//top of cube
		glNormal3f( 0.0f,  1.0f,  0.0f);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-cubes[i].center.x,  cubes[i].center.y, -cubes[i].center.z);
		glVertex3f(-cubes[i].center.x,  cubes[i].center.y,  cubes[i].center.z);
		glVertex3f( cubes[i].center.x,  cubes[i].center.y,  cubes[i].center.z);
		glVertex3f( cubes[i].center.x,  cubes[i].center.y, -cubes[i].center.z);

		//bottom of cube
		glNormal3f( 0.0f, -1.0f,  0.0f);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-cubes[i].center.x, -cubes[i].center.y, -cubes[i].center.z);
		glVertex3f( cubes[i].center.x, -cubes[i].center.y, -cubes[i].center.z);
		glVertex3f( cubes[i].center.x, -cubes[i].center.y,  cubes[i].center.z);
		glVertex3f(-cubes[i].center.x, -cubes[i].center.y,  cubes[i].center.z);

		//front of cube
		glNormal3f( 0.0f,  0.0f,  1.0f);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-cubes[i].center.x, -cubes[i].center.y,  cubes[i].center.z);
		glVertex3f( cubes[i].center.x, -cubes[i].center.y,  cubes[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f( cubes[i].center.x,  cubes[i].center.y,  cubes[i].center.z);
		glVertex3f(-cubes[i].center.x,  cubes[i].center.y,  cubes[i].center.z);

		//back of cube
		glNormal3f( 0.0f,  0.0f, -1.0f);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-cubes[i].center.x, -cubes[i].center.y, -cubes[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-cubes[i].center.x,  cubes[i].center.y, -cubes[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f( cubes[i].center.x,  cubes[i].center.y, -cubes[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( cubes[i].center.x, -cubes[i].center.y, -cubes[i].center.z);

		//left of cube
		glNormal3f(-1.0f,  0.0f,  0.0f);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-cubes[i].center.x, -cubes[i].center.y, -cubes[i].center.z);
		glVertex3f(-cubes[i].center.x, -cubes[i].center.y,  cubes[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-cubes[i].center.x,  cubes[i].center.y,  cubes[i].center.z);
		glVertex3f(-cubes[i].center.x,  cubes[i].center.y, -cubes[i].center.z);

		//right of cube
		glNormal3f( 1.0f,  0.0f,  0.0f);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( cubes[i].center.x, -cubes[i].center.y, -cubes[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f( cubes[i].center.x,  cubes[i].center.y, -cubes[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f( cubes[i].center.x,  cubes[i].center.y,  cubes[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( cubes[i].center.x, -cubes[i].center.y,  cubes[i].center.z);
		glEnd();
		glPopMatrix();
		offset += 1.75;
		ncubes++;
		c++;
	}
}

void makeReflection()
{
	int ncubes = 0;
	int c = 0;
	Flt offset = -12.0;
	for (int i = 0; i < MAX_CUBES; i++) {
		reflected[i].color[3] = 0.2;
		glPushMatrix();
		if (i < 8) {
			glTranslatef(reflected[i].pos[0][0] + 2.75*offset, reflected[i].pos[0][1] - 2, reflected[i].pos[0][2]);
		} else if (i < 16) {
			glTranslatef(reflected[i].pos[1][0] + 2.75*offset, reflected[i].pos[1][1] - 2, reflected[i].pos[1][2]);
		} else if (i < 24) {
			glTranslatef(reflected[i].pos[2][0] + 2.75*offset, reflected[i].pos[2][1] - 2, reflected[i].pos[2][2]);
		} else {
			glTranslatef(reflected[i].pos[3][0] + 2.75*offset, reflected[i].pos[3][1] - 2, reflected[i].pos[3][2]);
		}

		glRotatef(reflected[i].rot[0], 1.0f, 0.0f, 0.0f);
		glRotatef(reflected[i].rot[1], 0.0f, 1.0f, 0.0f);
		glRotatef(reflected[i].rot[2], 0.0f, 0.0f, 1.0f);

		//Anchors the cube to the bottom when it scales
		glTranslatef(reflected[i].loc[0], reflected[i].loc[1], reflected[i].loc[2]);
		glColor4f(reflected[i].color[0], reflected[i].color[1], reflected[i].color[2], reflected[i].color[3]);
		glScalef(2.0f, -reflected[i].loc[1], 2.0f);

		//printf("moving[%d] scaling[%d] reflected[0].loc[1] = %f\n", reflected[i].moving, reflected[i].scaling[1], reflected[0].loc[1]);
		if (reflected[i].moving) {
			switch (reflected[i].scaling[1]) {
				case 0:	/* Checks if the cube is scaling downwards */
						reflected[i].loc[1] = cubes[i].loc[1];
						if (reflected[i].loc[1] > -0.25f) {
							reflected[i].loc[1] 	= -0.25f;
							reflected[i].scaling[1] = 1;
						}
						break;
				case 1:	/* Checks if the cube is scaling upwards*/
						reflected[i].loc[1] = -cubes[i].loc[1];
						if (reflected[i].loc[1] < -2.5f) {
							reflected[i].loc[1] 	= -2.5f;
							reflected[i].scaling[1] = 0;
						}
						break;
			}
		}

		//If the cube is scaling downwards and the user wants to stop the cubes from moving
		if (reflected[i].scaling[1] == 0 && reflected[i].moving == false) {
			reflected[i].loc[1] += ((double)rnd() * (0.2f - 0.025f));
			if (reflected[i].loc[1] > -0.25f) {
				reflected[i].loc[1] = -0.25f;
				reflected[i].scaling[1] = 1;
				reflected[i].moving = true;
			}
		}

		if (reflected[i].axis[1] == 0) {
			reflected[i].scaling[1] = 0; reflected[i].moving = false;
		}

		glBegin(GL_QUADS);
		//top of cube
		glNormal3f( 0.0f,  1.0f,  0.0f);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-reflected[i].center.x,  reflected[i].center.y, -reflected[i].center.z);
		glVertex3f(-reflected[i].center.x,  reflected[i].center.y,  reflected[i].center.z);
		glVertex3f( reflected[i].center.x,  reflected[i].center.y,  reflected[i].center.z);
		glVertex3f( reflected[i].center.x,  reflected[i].center.y, -reflected[i].center.z);

		//bottom of cube
		glNormal3f( 0.0f, -1.0f,  0.0f);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-reflected[i].center.x, -reflected[i].center.y, -reflected[i].center.z);
		glVertex3f( reflected[i].center.x, -reflected[i].center.y, -reflected[i].center.z);
		glVertex3f( reflected[i].center.x, -reflected[i].center.y,  reflected[i].center.z);
		glVertex3f(-reflected[i].center.x, -reflected[i].center.y,  reflected[i].center.z);

		//front of cube
		glNormal3f( 0.0f,  0.0f,  1.0f);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-reflected[i].center.x, -reflected[i].center.y,  reflected[i].center.z);
		glVertex3f( reflected[i].center.x, -reflected[i].center.y,  reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( reflected[i].center.x,  reflected[i].center.y,  reflected[i].center.z);
		glVertex3f(-reflected[i].center.x,  reflected[i].center.y,  reflected[i].center.z);

		//back of cube
		glNormal3f( 0.0f,  0.0f, -1.0f);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-reflected[i].center.x, -reflected[i].center.y, -reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-reflected[i].center.x,  reflected[i].center.y, -reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( reflected[i].center.x,  reflected[i].center.y, -reflected[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f( reflected[i].center.x, -reflected[i].center.y, -reflected[i].center.z);

		//left of cube
		glNormal3f(-1.0f,  0.0f,  0.0f);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-reflected[i].center.x, -reflected[i].center.y, -reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-reflected[i].center.x, -reflected[i].center.y,  reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f(-reflected[i].center.x,  reflected[i].center.y,  reflected[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f(-reflected[i].center.x,  reflected[i].center.y, -reflected[i].center.z);

		//right of cube
		glNormal3f( 1.0f,  0.0f,  0.0f);
		glColor4fv(cubes[c%8].color);
		glVertex3f( reflected[i].center.x, -reflected[i].center.y, -reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( reflected[i].center.x,  reflected[i].center.y, -reflected[i].center.z);
		glColor4fv(cubes[c+1%8].color);
		glVertex3f( reflected[i].center.x,  reflected[i].center.y,  reflected[i].center.z);
		glColor4fv(cubes[c%8].color);
		glVertex3f( reflected[i].center.x, -reflected[i].center.y,  reflected[i].center.z);
		glEnd();
		glPopMatrix();
		offset += 1.75;
		ncubes++;
		c++;
	}
}

//Render Function
//-----------------------------------------------------------------------------
void render(void)
{
	Rect r;
	Camera cam;

	//Rendering of Cube(s)
	glViewport(0, 0, xres, yres);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat) xres / (GLfloat) yres, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);		glLoadIdentity();

	GLdouble fr[3], at[3], up[3];
	for (int i = 0; i < 3; i++) { fr[i] = cam.from[i]; at[i] = cam.at[i]; up[i] = cam.up[i]; }
	gluLookAt(fr[0],fr[1],fr[2], at[0],at[1],at[2], up[0],up[1],up[2]);

	glEnable(GL_LIGHTING);

	if (mode == 4) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPolygonOffset(1.0, 1.0);
		makeCubes();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		glDisable(GL_POLYGON_OFFSET_FILL);
		makeCubes();
	}

	/* Draws the reflection of each cubes by setting up stencil buffer */
	/* --------------------------------------------------------------------------------------------- */
	glEnable(GL_STENCIL_TEST);					//Enable the stencil buffer
	glStencilFunc(GL_ALWAYS, 1, 0xFF);			//Allow the stencil test to always pass
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	//Set pixels in the buffer to 1 when the test passes
	glStencilMask(0xFF);						//Don't update the buffer when renderng the floor
	glClear(GL_STENCIL_BUFFER_BIT);

	//Draw the floor, disable glDepthMask to get reflection on floor
	glDepthMask(GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonOffset(1.0, 1.0);
	drawFloor();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);

	//Update stencil function for reflections
	glStencilFunc(GL_EQUAL, 1, 0xFF);			//Pass stencil test if stencil value = 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);		//Prevent pixels in the buffer from changing
	glStencilMask(0x00);						//Allow the buffer to be updated
	glClear(GL_STENCIL_BUFFER_BIT);

	if (mode == 4) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonOffset(1.0, 1.0);
		makeReflection();							//Draw reflection of each cube	
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		glDisable(GL_POLYGON_OFFSET_FILL);
		makeReflection();							//Draw reflection of each cube
	}
	glDisable(GL_STENCIL_TEST);					//Disable stencil testing
	/* --------------------------------------------------------------------------------------------- */

	glEnable(GL_BLEND);
	glColor4f(1.0, 1.0, 1.0, 0.5f);

	glLightfv(GL_LIGHT0, GL_AMBIENT,	light.amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, 	light.diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, 	light.spec);
	glLightfv(GL_LIGHT0, GL_SHININESS,	light.shine);
	glLightfv(GL_LIGHT0, GL_POSITION,	light.pos);

	//GL Rendering of Menu as a 2D Rectangle
	glMatrixMode(GL_PROJECTION);	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);		glLoadIdentity();
	glDisable(GL_LIGHTING);
	glOrtho(0, xres, 0, yres, -1, 1);

	r.left = 10;
	r.center = 0;
	r.bot = yres - 20;
	ggprint8b(&r, 16, 0x00887766, "CS3480 Spectrum Analyzer Project");
	ggprint8b(&r, 16, 0x008877aa, "Q - Scale Cubes");	
	ggprint8b(&r, 16, 0x008877aa, "A - Rotate Cubes CW");
	ggprint8b(&r, 16, 0x008877aa, "D - Rotate Cubes CCW");
	ggprint8b(&r, 16, 0x008877aa, "S - Rotate Cube Downwards");
	ggprint8b(&r, 16, 0x008877aa, "W - Rotate Cube Upwards");
	ggprint8b(&r, 16, 0x008877aa, "E - Toggle Fog (On/Off)");
	ggprint8b(&r, 16, 0x008877aa, "R - Wireframe Mode");	
	ggprint8b(&r, 16, 0x008877aa, "ESC - Quit");

}

/*	
	1. Select N that is a power of two (Calculating an N-point FFT).
	2. Gather samples into a buffer of size N
	3. Sort the samples in bit-reversed order, put them in a complex N-point buffer 
			(Set the imaginary numbers to zero)
	4. Apply the first stage butterfly using adjacent pairs of numbers in the buffer
	5. Apply the second stage butterfly using pairs that are separated by 2
	6. Apply the third stage butterfly using pairs that are separated by 4
	7. Continue butterflying the numbers in the buffer until you get to separation of N/2
	8. The buffer will contain the Fourier Transform

*/


/*	Calculates the Fast Fourier Transform

	- Recursively splits the signal into two halves: 
		* one contains all the even index values, 
		* the other all the odd index values.
	- The requirement is that the size of the signal, n, must be a power of 2 b/c of this.
	
	1. Calculates the log of n
	2. Performs Bit Reversal
		* By splitting the signal in its even and odd components every time, the alg. requires that 
			the input signal is alreadly in the form where all odd components come first, followed by the evens
		* Similar to reversing the bits of the index of each value.
	3. Perform the FFT algorithm
		* Technically a recursive function, but this is a modified version that works non-recursively
			~ It would be way too hard to pass complete arrays to a recursive function every time
		* ca and sa represent the cosine and sine, but are calculated with the half-angle formulas
			and initial values -1.0 and 0.0 (cos and sin of pi)
			~ Within each loop, ca and sa represent the cosine and sine of half the angles as the previous loop
	4. Divide the values through n if it is not the inverse DFT
	
	The function will read from *gReal and *gImag again and store the result in *GReal & *GImag
*/

void FFT(int n, bool inverse, const double *gReal, const double *gImag, double *GReal, double *GImag) {
	int m = 0, p = 1, j = 0;
	
	//Splits the signal into even and odd halves
	//Will calculate runtime to m = log_2(n)
	while (p < n) {
		p *= 2;
		m++;
	}
	//Debug for runtime
	//printf("M: %d/n", m);
	
	/*	Bit Reversal
		* The algorithm requires that the input signal is alreadly in the form 
			where all odd components come first followed by the evens. 
		
		* This is the same as reversing the bits of the index of each value
	*/
	GReal[n - 1] = gReal[n - 1];
	GImag[n - 1] = gImag[n - 1];
	for(int i = 0; i < n - 1; i++) {
		GReal[i] = gReal[j];
		GImag[i] = gImag[j];
		int k = n / 2;
		while (k <= j) {
			j -= k;
			k /= 2;
		}
		j += k;
	}
	
	/* 	Calculate the FFT (Non-recursive function)
		* Cosine and sine are calculated with half-angle formulas 
			and initial values -1.0 and 0.0 (cosine and sine of pi)
		
		* With each loop, ca and sa represent the cosine and sine 
			of half the angle as the previous loop
	*/
	double ca = -1.0;
	double sa =  0.0;
	int left1 = 1, left2 = 1;
	
	for(int l = 0; l < m; l++) {
		left1 = left2;
		left2 *= 2;
		double u1 = 1.0;
		double u2 = 0.0;
		
		for(int j = 0; j < left2; j++) {
			
			for(int i = j; i < n; i += left2) {
				int i1 = i + left1;
				double t1 = u1 * GReal[i1] - u2 * GImag[i1];
				double t2 = u1 * GImag[i1] + u2 * GReal[i1];
				GReal[i1] = GReal[i] - t1;
				GImag[i1] = GImag[i] - t2;
				GReal[i1] += t1;
				GImag[i1] += t2;
			}
			
			double z = u1 * ca - u2 * sa;
			u2 = u1 * sa + u2 * ca;
			u1 = z;
		}
		sa = sqrt((1.0 - ca) / 2.0);
		if (!inverse) sa =- sa;
		ca = sqrt((1.0 + ca) / 2.0);
	}
	
	//Divide the values through n if not inverse DFT
	if (!inverse) {
		for(int i = 0; i < n; i++) {
			GReal[i] /= n;
			GImag[i] /= n;
		}
	}
}

//Plots the function onto the screen
void plot(int y, const double *g, double scale, bool trans, ColorRGB color) {
	//drawLine(0, yPos, w - 1, yPos, ColorRGB(128, 128, 255));
	for(int i = 1; i < N; i++) {
		int x1, x2, x3, x4;
		int y1, y2, y3, y4;
		x1 = (x - 1) * 5;
		
		//Get first endpoint, use the one half a period away if shift is used
		if (!shift) {
			y1 = -int(scale * g[i - 1]) + yPos;
		} else {
			y1 = -int(scale * g[i - 1 + N / 2] % N) + yPos;
		}
		
		x2 = x * 5;
		//Get next endpoint, use the one half a period away if shift is used
		if (!shift) {
			y2 = -int(scale * g[i]) + yPos;
		} else {
			y2 = -int(scale * g[i + N / 2] % N) + yPos;
		}
		
		//Clip the line to the screen in order to prevent drawing outside of the screen (??)
		//clipLine(x1, y1, x2, y2, x3, y3, x4, y4);
		//drawLine(x3, y3, x4, y4, color)
		
		//Draw circles to display that the function is made out of discrete points
		//drawDisk(x4, y4, 2, color);
	}
}

//Calculates the amplitude of the complex function
void calcAmp(int n, double *gAmp, const double *gReal, const double *gImag) {
	for(int i = 0; i < n; i++) {
		gAmp[i] = sqrt(gReal[i] * gReal[i] + gImag[i] * gImag[i]);
	}
}