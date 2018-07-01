#include "vectors.h"
#include <math.h>

struct Point {
	Flt x, y, z, a;
	Point() {
		x = y = z = a = 1;
	}
};

struct Cube {
	Point	center;
	int		scaling[3], axis[3];
	Flt		height, width, length;
	int		type, surface, npoints;
	Vec		pos[4], loc, ang, rot;
	Vec		verts, normal, spec, floor;
	bool	inside, specular, moving, fog, checker;
	Flt		color[4];

	Cube() {
		height	= 10.0;
		width 	= 20.0;
		length	= 20.0;
		inside	= specular	= moving = false;
		fog		= checker	= false;
		vecMake(  2.0,  2.0,  2.0, verts);
		vecMake(  0.0,  0.0,  0.0, ang);
		vecMake(  3.0,  5.0,-45.0, pos[0]);
		vecMake(-28.0,  5.0,-30.0, pos[1]);
		vecMake(-58.0,  5.0,-15.0, pos[2]);
		vecMake(-88.0,  5.0,  0.0, pos[3]);
		vecMake(  0.0,  0.0,-30.0, floor);
		vecMake(  0.0,  0.0,  0.0, loc);
		vecMake(  0.4,  0.4,  0.4, spec);		
		vecMake(-15.0,  0.0,  0.0, rot);
		vecMake(  0.0,  0.0,  0.0, color);
		color[3] = 0.5;
	}
};
