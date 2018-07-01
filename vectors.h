/* Create or alter vector objects within the main file */
#ifndef VECTORS_H
#define VECTORS_H

//Variable Types
typedef float Flt;
typedef Flt	 Vec[3];

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

#endif
