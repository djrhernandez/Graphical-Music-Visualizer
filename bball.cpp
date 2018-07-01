//cs371 Fall 2013
//
//program: bball.cpp
//author:  Gordon Griesel
//date:    2013
//         fall 2017
//
//This program demonstrates a sphere with texture
//
//Depending on your Linux distribution,
//may have to install these packages:
// libx11-dev
// libglew1.6
// libglew1.6-dev
//
//
//
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "ppm.h"
#include "fonts.h"
//#include "log.h"

typedef float Flt;
typedef Flt Vec[3];
#define rnd() (float)rand() / (float)RAND_MAX
#define PI 3.14159265358979323846264338327950
//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

void initXWindows(void);
void init_opengl(void);
void init_textures(void);
void cleanupXWindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
void check_keys(XEvent *e);
void physics(void);
void render(void);

int done=0;
int xres=640, yres=480;
float pos[3]={20.0,200.0,0.0};
float vel[3]={3.0,0.0,0.0};

int lesson_num=0;

float rtri = 0.0f;
float rquad = 0.0f;
float cubeRot[3]={2.0,0.0,0.0};
float cubeAng[3]={0.0,0.0,0.0};
GLfloat LightAmbient[]  = {  0.0f, 0.0f, 0.0f, 1.0f };
GLfloat LightDiffuse[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = {  0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightPosition[] = { 100.0f, 40.0f, 40.0f, 1.0f };

Ppmimage *bballImage=NULL;
Ppmimage *courtImage=NULL;
Ppmimage *standsImage=NULL;
GLuint bballTextureId;
GLuint courtTextureId;
GLuint standsTextureId;


int main(void)
{
	initXWindows();
	init_opengl();
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
	init_textures();
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_resize(&e);
			check_mouse(&e);
			check_keys(&e);
		}
		physics();
		render();
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	cleanup_fonts();
	return 0;
}

void cleanupXWindows(void)
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "OpenGL 3D Demo");
}

void setup_screen_res(const int w, const int h)
{
	xres = w;
	yres = h;
}

void initXWindows(void)
{
	Window root;
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;

	setup_screen_res(640, 480);
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
		exit(EXIT_FAILURE);
	}
	root = DefaultRootWindow(dpy);
	vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		exit(EXIT_FAILURE);
	} 
	//else {
	//	// %p creates hexadecimal output like in glxinfo
	//	printf("\n\tvisual %p selected\n", (void *)vi->visualid);
	//}
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
						StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0,
							vi->depth, InputOutput, vi->visual,
							CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void reshape_window(int width, int height)
{
	//window has been resized.
	setup_screen_res(width, height);
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, xres, 0, yres, -1, 1);
	set_title();
}

void init_textures(void)
{
	//load the images file into a ppm structure.
	bballImage = ppm6GetImage("./bball.ppm");
	courtImage = ppm6GetImage("./court.ppm");
	//
	//create opengl texture elements
	glGenTextures(1, &bballTextureId);
	int w = bballImage->width;
	int h = bballImage->height;
	//
	glBindTexture(GL_TEXTURE_2D, bballTextureId);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, bballImage->data);
	//
	//create opengl texture elements
	glGenTextures(1, &courtTextureId);
	w = courtImage->width;
	h = courtImage->height;
	//
	glBindTexture(GL_TEXTURE_2D, courtTextureId);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, courtImage->data);
}

#define VecCross(a,b,c) \
(c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1]; \
(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2]; \
(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]

void vecCrossProduct(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[1]*v1[2] - v1[1]*v0[2];
	dest[1] = v0[2]*v1[0] - v1[2]*v0[0];
	dest[2] = v0[0]*v1[1] - v1[0]*v0[1];
}

Flt vecDotProduct(Vec v0, Vec v1)
{
	return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}

void vecZero(Vec v)
{
	v[0] = v[1] = v[2] = 0.0;
}

void vecMake(Flt a, Flt b, Flt c, Vec v)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void vecCopy(Vec source, Vec dest)
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}

Flt vecLength(Vec v)
{
	return sqrt(vecDotProduct(v, v));
}

void vecNormalize(Vec v)
{
	Flt len = vecLength(v);
	if (len == 0.0) {
		vecMake(0,0,1,v);
		return;
	}
	len = 1.0 / len;
	v[0] *= len;
	v[1] *= len;
	v[2] *= len;
}

void vecSub(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[0] - v1[0];
	dest[1] = v0[1] - v1[1];
	dest[2] = v0[2] - v1[2];
}


void init_opengl(void)
{
	//OpenGL initialization
	switch(lesson_num) {
		case 0:
		case 1:
			glViewport(0, 0, xres, yres);
			glDepthFunc(GL_LESS);
			glDisable(GL_DEPTH_TEST);
			//Initialize matrices
			glMatrixMode(GL_PROJECTION); glLoadIdentity();
			glMatrixMode(GL_MODELVIEW); glLoadIdentity();
			//This sets 2D mode (no perspective)
			glOrtho(0, xres, 0, yres, -1, 1);
			//Clear the screen
			glClearColor(1.0, 1.0, 1.0, 1.0);
			//glClear(GL_COLOR_BUFFER_BIT);
			break;
		//case 5:
		//case 6:
		//case 7:
		//case 8:
		default:
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClearDepth(1.0);
			glDepthFunc(GL_LESS);
			glEnable(GL_DEPTH_TEST);
			glShadeModel(GL_SMOOTH);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0f,(GLfloat)xres/(GLfloat)yres,0.1f,100.0f);
			glMatrixMode(GL_MODELVIEW);
			//Enable this so material colors are the same as vert colors.
			glEnable(GL_COLOR_MATERIAL);
			glEnable( GL_LIGHTING );
			glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
			glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
			glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
			glEnable(GL_LIGHT0);
			break;
	}
}

void check_resize(XEvent *e)
{
	//The ConfigureNotify is sent by the
	//server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != xres || xce.height != yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	}
}

void check_mouse(XEvent *e)
{
	//Did the mouse move?
	//Was a mouse button clicked?
	static int savex = 0;
	static int savey = 0;
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
		}
		if (e->xbutton.button==3) {
			//Right button is down
		}
	}
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//Mouse moved
		savex = e->xbutton.x;
		savey = e->xbutton.y;
	}
}

void check_keys(XEvent *e)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		switch(key) {
			case XK_1:
				lesson_num = 1;
				init_opengl();
				break;
			case XK_3:
				lesson_num = 3;
				init_opengl();
				break;
			case XK_4:
				lesson_num = 4;
				init_opengl();
				break;
			case XK_5:
				lesson_num = 5;
				init_opengl();
				break;
			case XK_6:
				lesson_num = 6;
				init_opengl();
				break;
			case XK_7:
				lesson_num = 7;
				init_opengl();
				break;
			case XK_8:
				lesson_num = 8;
				init_opengl();
				break;
			case XK_Escape:
				done=1;
				break;
		}
	}
}

void physics(void)
{
	int addgrav = 1;
	//Update position
	pos[0] += vel[0];
	pos[1] += vel[1];
	//Check for collision with window edges
	if ((pos[0] < 0.0          && vel[0] < 0.0) ||
		(pos[0] >= (float)xres && vel[0] > 0.0)) {
		vel[0] = -vel[0];
		addgrav = 0;
	}
	if ((pos[1] < 0.0          && vel[1] < 0.0) ||
		(pos[1] >= (float)yres && vel[1] > 0.0)) {
		vel[1] = -vel[1];
		addgrav = 0;
	}
	//Gravity
	//do not apply gravity if bouncing on the floor.
	if (addgrav)
		vel[1] -= 0.4;
}

void DrawGLScene3()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(-1.5f,0.0f,-6.0f);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f( 0.0f, 1.0f, 0.0f);
		glColor3f(0.0f,1.0f,0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
	glEnd();
	glTranslatef(3.0f,0.0f,0.0f);
	glColor3f(0.5f,0.5f,1.0f);
	glBegin(GL_QUADS);
		glVertex3f(-1.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
	glEnd();
}

void DrawGLScene4()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glTranslatef(-1.5f,0.0f,-6.0f);
	glRotatef(rtri,0.0f,1.0f,0.0f);
	glColor3f(0.6f,0.7f,0.8f);
	glBegin(GL_TRIANGLES);
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		//back side
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f( 0.0f, 1.0f, 0.01f);
		glVertex3f( 1.0f,-1.0f, 0.01f);
		glVertex3f(-1.0f,-1.0f, 0.01f);
	glEnd();
	glLoadIdentity();
	glTranslatef(1.5f,0.0f,-6.0f);
	glRotatef(rquad,1.0f,0.0f,0.0f);
	glColor3f(1.0f,0.5f,0.5f);
	glBegin(GL_QUADS);
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		//back side
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f, 0.01f);
		glVertex3f( 1.0f, 1.0f, 0.01f);
		glVertex3f( 1.0f,-1.0f, 0.01f);
		glVertex3f(-1.0f,-1.0f, 0.01f);
	glEnd();
	rtri  += 4.0f;
	rquad -= 4.0f;
}

void DrawGLScene5()
{
	Vec v1,v2,v3,v4,v5,norm;
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
	glTranslatef(-1.5f,0.0f,-6.0f);
	glRotatef(rtri,0.0f,1.0f,0.0f);
	glColor3f(0.8f,0.0f,0.0f);
	glBegin(GL_TRIANGLES);
		//Notice the process here...
		//1. build verts
		//2. make 2 vectors
		//3. find cross product. that's the normal
		//4. normalize it to a length of 1
		vecMake( 0.0f, 1.0f, 0.0f, v1);
		vecMake(-1.0f,-1.0f, 1.0f, v2);
		vecMake( 1.0f,-1.0f, 1.0f, v3);
		vecSub(v2,v1,v4);
		vecSub(v3,v1,v5);
		vecCrossProduct(v4,v5,norm);
		vecNormalize(norm);
		glNormal3fv(norm);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
		vecMake( 0.0f, 1.0f, 0.0f,v1);
		vecMake( 1.0f,-1.0f, 1.0f,v2);
		vecMake( 1.0f,-1.0f,-1.0f,v3);
		vecSub(v2,v1,v4);
		vecSub(v3,v1,v5);
		vecCrossProduct(v4,v5,norm);
		vecNormalize(norm);
		glNormal3fv(norm);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
		vecMake( 0.0f, 1.0f, 0.0f,v1);
		vecMake( 1.0f,-1.0f,-1.0f,v2);
		vecMake(-1.0f,-1.0f,-1.0f,v3);
		vecSub(v2,v1,v4);
		vecSub(v3,v1,v5);
		vecCrossProduct(v4,v5,norm);
		vecNormalize(norm);
		glNormal3fv(norm);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
		vecMake( 0.0f, 1.0f, 0.0f,v1);
		vecMake(-1.0f,-1.0f,-1.0f,v2);
		vecMake(-1.0f,-1.0f, 1.0f,v3);
		vecSub(v2,v1,v4);
		vecSub(v3,v1,v5);
		vecCrossProduct(v4,v5,norm);
		vecNormalize(norm);
		glNormal3fv(norm);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
	glEnd();
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glTranslatef(1.5f,0.0f,-7.0f);
	glRotatef(rquad,1.0f,1.0f,1.0f);
	glColor3f(0.0f,0.5f,1.0f);
	glBegin(GL_QUADS);
		//top
		//notice the normal being set
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		// bottom of cube
		glNormal3f( 0.0f, -1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		// front of cube
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		// back of cube.
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		// left of cube
		glNormal3f( -1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		// Right of cube
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
	glEnd();
	rtri+=2.0f;
	rquad-=1.0f;
}

void LightedCube()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	glTranslatef(0.0f,0.0f,-7.0f);
	glRotatef(cubeAng[0],1.0f,0.0f,0.0f);
	glRotatef(cubeAng[1],0.0f,1.0f,0.0f);
	glRotatef(cubeAng[2],0.0f,0.0f,1.0f);

	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_QUADS);
		//top
		//notice the normal being set
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		// bottom of cube
		glNormal3f( 0.0f, -1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		// front of cube
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		// back of cube.
		glNormal3f( 0.0f, 0.0f, -1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		// left of cube
		glNormal3f( -1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		// Right of cube
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
	glEnd();
	rquad -= 2.0f;
	int i;
	if (rnd() < 0.01) {
		for (i=0; i<3; i++) {
			cubeRot[i] = rnd() * 4.0 - 2.0;
		}
	}
	for (i=0; i<3; i++) {
		cubeAng[i] += cubeRot[i];
	}
}

void DrawBasketball()
{
	static int firsttime=1;
	static float ballPos[3]={0.0,2.0,-7.0};
	static float ballVel[3]={0.2,0.0,0.0};
	//16 longitude lines.
	//8 latitude levels.
	//3 values each: x,y,z.
	int i, j, i2, j2, j3;
	static float verts[9][16][3];
	static float norms[9][16][3];
	static float    tx[9][17][2];
	if (firsttime) {
		//build basketball vertices here. only once!
		firsttime=0;
		float circle[16][2];
		float angle=0.0, inc = (PI * 2.0) / 16.0;
		for (i=0; i<16; i++) {
			circle[i][0] = cos(angle);
			circle[i][1] = sin(angle);
			angle += inc;
			printf("circle[%2i]: %f %f\n", i, circle[i][0], circle[i][1]);
		}
		//use the circle points to build all vertices.
		//8 levels of latitude...
		for (i=0; i<=8; i++) {
			for (j=0; j<16; j++) {
				verts[i][j][0] = circle[j][0] * circle[i][1]; 
				verts[i][j][2] = circle[j][1] * circle[i][1];
				verts[i][j][1] = circle[i][0];
				norms[i][j][0] = verts[i][j][0]; 
				norms[i][j][1] = verts[i][j][1];
				norms[i][j][2] = verts[i][j][2];
				tx[i][j][0] = (float)j / 16.0;
				tx[i][j][1] = (float)i / 8.0;
			}
			tx[i][j][0] = (float)j / 16.0;
			tx[i][j][1] = (float)i / 8.0;
		}
	}
	//
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
	glPushMatrix();
	glTranslatef(ballPos[0],ballPos[1],ballPos[2]);
	//glRotatef(cubeAng[0],1.0f,0.0f,0.0f);
	//glRotatef(cubeAng[1],0.0f,1.0f,0.0f);
	glRotatef(cubeAng[2],0.0f,0.0f,1.0f);
	//draw the ball, made out of quads...
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, bballTextureId);
	glBegin(GL_QUADS);
	for (i=0; i<8; i++) {
		for (j=0; j<16; j++) {
			i2 = i+1;
			j2 = j+1;
			if (j2 >= 16) j2 -= 16;
			j3 = j+1;
			glNormal3fv(norms[i ][j ]); glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
			glNormal3fv(norms[i2][j ]); glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
			glNormal3fv(norms[i2][j2]); glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
			glNormal3fv(norms[i ][j2]); glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
		}
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//
	glPushMatrix();
	glTranslatef(ballPos[0], 0.0, ballPos[2]);
	//
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(255,255,255,80);
	//
	//draw the shadow
	glBegin(GL_QUADS);
	for (i=0; i<8; i++) {
		for (j=0; j<16; j++) {
			i2 = i+1;
			j2 = j+1;
			if (j2 >= 16) j2 -= 16;
			j3 = j+1;
			glVertex3f(verts[i ][j ][0], -1.0, verts[i ][j ][2]);
			glVertex3f(verts[i2][j ][0], -1.0, verts[i2][j ][2]);
			glVertex3f(verts[i2][j2][0], -1.0, verts[i2][j2][2]);
			glVertex3f(verts[i ][j2][0], -1.0, verts[i ][j2][2]);
		}
	}
	glEnd();
	glDisable (GL_BLEND);
	glPopMatrix();
	//
	glBindTexture(GL_TEXTURE_2D, courtTextureId);
	glBegin(GL_QUADS);
		glNormal3f(0.0,1.0,0.0);
		glTexCoord2f( 0.0,  0.0); glVertex3f(-20.0, -1.031, -20.0);
		glTexCoord2f( 0.0, 10.0); glVertex3f(-20.0, -1.031,  20.0);
		glTexCoord2f(10.0, 10.0); glVertex3f( 20.0, -1.031,  20.0);
		glTexCoord2f(10.0,  0.0); glVertex3f( 20.0, -1.031, -20.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	//
	//
	//bounce ball...
	ballPos[0] += ballVel[0];
	ballPos[1] += ballVel[1];
	ballVel[0] *= 0.999;
	ballVel[1] *= 0.999;
	//
	if (ballPos[0] > 4.0) {
		if (ballVel[0] > 0.0) {
			ballVel[0] = -ballVel[0];
			cubeRot[0] = cubeRot[1] = cubeRot[2] = 0.0;
		}
	}
	if (ballPos[0] < -4.0) {
		if (ballVel[0] < 0.0) {
			ballVel[0] = -ballVel[0];
			cubeRot[0] = cubeRot[1] = cubeRot[2] = 0.0;
		}
	}
	int grav=1;
	if (ballPos[1] < 0.0) {
		if (ballVel[1] < 0.0) {
			ballVel[1] = -ballVel[1];
			grav=0;
			//rotate basketball
			//if (rnd() < 0.01) {
			//	for (i=0; i<3; i++) {
			//		cubeRot[i] = rnd();
			//	}
				cubeRot[2] = rnd() * ballVel[0] * -40.0;
			//}
		}
	}
	if (grav)
		ballVel[1] -= 0.01;
	for (i=0; i<3; i++) {
		cubeAng[i] += cubeRot[i];
	}

}

void finalExamQuestion()
{
	static float frot=0.0f;
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
	//build a small quad at 1, 2, -7
	float x=1.0f, y=2.0f, z=-7.0f;
	//---------------------------------
	glTranslatef(x, y, z);             //<--- put object back in place
	glRotatef(frot, 0.0f, 0.0f, 1.0f); //<--- rotate
	glTranslatef(-x,-y,-z);            //<--- put object at origin
	//---------------------------------
	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_QUADS);
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f(x-0.5, y-0.5, z);
		glVertex3f(x-0.5, y+0.5, z);
		glVertex3f(x+0.5, y+0.5, z);
		glVertex3f(x+0.5, y-0.5, z);
	glEnd();
	frot += 4.0f;
}

void render(void)
{
	Rect r;
	glClear(GL_COLOR_BUFFER_BIT);
	//
	r.bot = yres - 20;
	r.left = 10;
	r.center = 0;
	ggprint8b(&r, 16, 0x00887766, "CS-371 Lab7");
	ggprint8b(&r, 16, 0x008877aa, "3 - simple shapes");
	ggprint8b(&r, 16, 0x008877aa, "4 - animated shapes");
	ggprint8b(&r, 16, 0x008877aa, "5 - animated 3D shapes");
	ggprint8b(&r, 16, 0x008877aa, "6 - rotating lighted cube");
	ggprint8b(&r, 16, 0x008877aa, "7 - basketball on court");
	ggprint8b(&r, 16, 0x008877aa, "8 - rotation on chosen axis");
	//
	//Just draw a simple square
	switch(lesson_num) {
		case 0:
		case 1: {
				float wid = 40.0f;
				glColor3ub(30,60,90);
				glPushMatrix();
				glTranslatef(pos[0], pos[1], pos[2]);
				glBegin(GL_QUADS);
					glVertex2i(-wid,-wid);
					glVertex2i(-wid, wid);
					glVertex2i( wid, wid);
					glVertex2i( wid,-wid);
				glEnd();
				glPopMatrix();
			}
			break;
		case 3:
			DrawGLScene3();
			break;
		case 4:
			DrawGLScene4();
			break;
		case 5:
			DrawGLScene5();
			break;
		case 6:
			LightedCube();
			break;
		case 7:
			DrawBasketball();
			break;
		case 8:
			finalExamQuestion();
			break;
	}
}



