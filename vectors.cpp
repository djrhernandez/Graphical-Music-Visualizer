#include "vectors.h"
#include <math.h>

#define vecComb(A, a, B, b, C, c)	(c)[0] = (A) * (a)[0] + (B) * (b)[0]; \
									(c)[1] = (A) * (a)[1] + (B) * (b)[1]; \
									(c)[2] = (A) * (a)[1] + (B) * (b)[2]

Flt vecLength(Vec vec) {
	return sqrt(vecDot(vec, vec));
}

Flt vecDot(Vec u, Vec v) {
	return ((u[0] * v[0]) + (u[1] * v[1]) + (u[2] * v[2]));
}

void vecZero(Vec v) {
	v[0] = 0; 
	v[1] = 0;
	v[2] = 0;
}

void vecCopy(Vec src, Vec dest) {
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

void vecNegate(Vec v) {
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void vecMake(Flt a, Flt b, Flt c, Vec v) {	
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void vecAdd(Vec u, Vec v, Vec dest) {
	dest[0] = u[0] + v[0];
	dest[1] = u[1] + v[1];
	dest[2] = u[2] + v[2];

}

void vecSub(Vec u, Vec v, Vec dest) {	
	dest[0] = u[0] - v[0];
	dest[1] = u[1] - v[1];
	dest[2] = u[2] - v[2];
}


void vecCross(Vec u, Vec v, Vec dest) {
	dest[0] = (u[1] * v[2]) - (u[2] * v[1]);
	dest[1] = (u[2] * v[0]) - (u[0] * v[2]);
	dest[2] = (u[0] * v[1]) - (u[1] * v[0]);
}

void vecNormalize(Vec vec) {
	Flt len = vecLength(vec);
	if (len == 0.0) {
		vecMake(1, 0, 0, vec);
		return;
	}
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}
