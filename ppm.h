#ifndef _PPM_H_
#define _PPM_H_
//ppm.h
//Read and write PPM files. Plain format.
//#include <sys/types.h>
typedef struct t_ppmimage {
	int width;
	int height;
	void *data;
} Ppmimage;

extern Ppmimage *ppm1GetImage(char *filename);
extern void ppm1CleanupImage(Ppmimage *image);
extern Ppmimage *ppm1CreateImage(int width, int height);
extern void ppm1SaveImage(Ppmimage *image, char *filename);
extern void ppm1ClearImage(Ppmimage *image, unsigned char color);
extern void ppm1Setpixel(Ppmimage *image, int x, int y, unsigned char val);
//
extern Ppmimage *ppm3CreateImage(int width, int height);
extern Ppmimage *ppm3GetImage(char *filename);
extern void ppm3ClearImage(Ppmimage *image, unsigned char r, unsigned char g, unsigned char b);
extern void ppm3SaveImage(Ppmimage *image, char *filename);
extern void ppm3Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val);
extern void ppm3CleanupImage(Ppmimage *image);
//
extern Ppmimage *ppm6CreateImage(int width, int height);
extern Ppmimage *ppm6GetImage(const char *filename);
extern void ppm6ClearImage(Ppmimage *image, unsigned char r, unsigned char g, unsigned char b);
extern void ppm6SaveImage(Ppmimage *image, const char *filename);
extern void ppm6Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val);
extern void ppm6CleanupImage(Ppmimage *image);

#endif


