//cs371
//program: ppm.cpp
//author:  Gordon Griesel
//date:    Fall 2013
//Read and write PPM files.
//   P1 = 2-color
//   P3 = Plain format (ASCII)
//   P6 = Packed format (binary)
//Files can be used to store images for viewing or texture maps.
/*

sample image file is below:
------------------------------------------
P1
#5x10 the number 1
5 10
0 0 0 0 0
0 0 1 0 0
0 1 1 0 0
0 0 1 0 0
0 0 1 0 0
0 0 1 0 0
0 0 1 0 0
0 0 1 0 0
0 1 1 1 0
0 0 0 0 0
------------------------------------------

sample image file is below:
------------------------------------------
P3
#4x2 yellow image
4 2
255
255 255 0 255 255 0 255 255 0 255 255 0
255 255 0 255 255 0 255 255 0 255 255 0
------------------------------------------

sample image file is below:
------------------------------------------
P6
#4x2 dim-yellow image
4 4
255
zzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzA
------------------------------------------
*/
//
//format inventor: Jef Poskanzer
//When writing, we will follow his suggested limit of 70-character lines.
//When reading, we allow for much longer lines.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ppm.h"

Ppmimage *ppm1GetImage(const char *filename)
{
	int i, j, width, height, size, ntries;
	char ts[4096];
	unsigned int color;
	unsigned char c;
	unsigned char *p;
	FILE *fpi;
	Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
	if (!image) {
		printf("ERROR: out of memory\n");
		exit(EXIT_FAILURE);
	}
	fpi = fopen(filename, "r");
	if (!fpi) {
		printf("ERROR: cannot open file **%s** for reading.\n", filename);
		exit(EXIT_FAILURE);
	}
	image->data=NULL;
	char *x = fgets(ts, 4, fpi);
	if (x){}
	if (strncmp(ts, "P1" ,2) != 0) {
		printf("ERROR: File is not ppm format.\n");
		exit(EXIT_FAILURE);
	}
	//comments?
	while(1) {
		c = fgetc(fpi);
		if (c != '#')
			break;
		//read until newline
		ntries=0;
		while(1) {
			//to avoid infinite loop...
			if (++ntries > 10000) {
				printf("ERROR: too many blank lines in **%s**\n", filename);
				exit(EXIT_FAILURE);
			}
			c = fgetc(fpi);
			if (c == '\n')
				break;
		}
	}
	ungetc(c, fpi);
	int r = fscanf(fpi, "%u %u", &width, &height);
	if (r){}
	//
	size = width * height * 3;
	image->data = (unsigned char *)malloc(size);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}
	image->width = width;
	image->height = height;
	p = (unsigned char *)image->data;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			r = fscanf(fpi, "%u",&color);
			*p = color;
			p++;
		}
	}
	fclose(fpi);
	return image;
}

void ppm1CleanupImage(Ppmimage *image)
{
	if (image != NULL) {
		if (image->data != NULL)
			free(image->data);
		free(image);
	}
}

Ppmimage *ppm1CreateImage(int width, int height)
{
	//printf("ppm1_create_image()...\n");
	int size = width * height;
	Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
	if (!image) {
		printf("ERROR: no memory for image.\n");
		exit(EXIT_FAILURE);
	}
	image->data = (unsigned char *)malloc(size);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}
	image->width = width;
	image->height = height;
	return image;
}  

void ppm1SaveImage(Ppmimage *image, const char *filename)
{
	int i,j;
	unsigned char *p = (unsigned char *)image->data;
	FILE *fpo = fopen(filename, "w");
	if (!fpo) {
		printf("ERROR: cannot open file **%s** for writing.\n", filename);
		exit(EXIT_FAILURE);
	}
	fprintf(fpo, "P1\n%d %d\n", image->width, image->height);
	for (i=0; i<image->height; i++) {
		for (j=0; j<image->width; j++) {
			fprintf(fpo, "%u ",*p);
			p++;
		}
		fprintf(fpo, "\n");
	}
	fclose(fpo);
}  

void ppm1ClearImage(Ppmimage *image, unsigned char color)
{
	int i;
	int pix = image->width * image->height;
	unsigned char *data = (unsigned char *)image->data;
	for (i=0; i<pix; i++) {
		*data++ = color;
	}
}

void ppm1Setpixel(Ppmimage *image, int x, int y, unsigned char val) {
	unsigned char *data = (unsigned char *)image->data + (y * image->width + x);
	*data = val;
}



Ppmimage *ppm3GetImage(const char *filename)
{
	int i, j, width, height, size, maxval, ntries;
	char ts[4096];
	unsigned int rgb[3];
	unsigned char c;
	unsigned char *p;
	FILE *fpi;
	Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
	if (!image) {
		printf("ERROR: out of memory\n");
		exit(EXIT_FAILURE);
	}
	fpi = fopen(filename, "r");
	if (!fpi) {
		printf("ERROR: cannot open file **%s** for reading.\n", filename);
		exit(EXIT_FAILURE);
	}
	image->data=NULL;
	char *x = fgets(ts, 4, fpi);
	if (x){}//to make compiler happy
	if (strncmp(ts, "P3" ,2) != 0) {
		printf("ERROR: File is not ppm ASCII format.\n");
		exit(EXIT_FAILURE);
	}
	//comments?
	while(1) {
		c = fgetc(fpi);
		if (c != '#')
			break;
		//read until newline
		ntries=0;
		while(1) {
			//to avoid infinite loop...
			if (++ntries > 10000) {
				printf("ERROR: too many blank lines in **%s**\n", filename);
				exit(EXIT_FAILURE);
			}
			c = fgetc(fpi);
			if (c == '\n')
				break;
		}
	}
	ungetc(c, fpi);
	int r = fscanf(fpi, "%u%u%u", &width, &height, &maxval);
	if (r){}
	//
	size = width * height * 3;
	image->data = (unsigned char *)malloc(size);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}
	image->width = width;
	image->height = height;
	p = (unsigned char *)image->data;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			r = fscanf(fpi, "%u%u%u",&rgb[0],&rgb[1],&rgb[2]);
			*p = rgb[0]; p++;
			*p = rgb[1]; p++;
			*p = rgb[2]; p++;
		}
	}
	fclose(fpi);
	return image;
}

void ppm3CleanupImage(Ppmimage *image)
{
	if (image != NULL) {
		if (image->data != NULL)
			free(image->data);
		free(image);
	}
}

Ppmimage *ppm3CreateImage(int width, int height)
{
	//printf("ppm3_create_image()...\n");
	int size = width * height * 3;
	Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
	if (!image) {
		printf("ERROR: no memory for image.\n");
		exit(EXIT_FAILURE);
	}
	image->data = (unsigned char *)malloc(size);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}
	image->width = width;
	image->height = height;
	return image;
}  

void ppm3SaveImage(Ppmimage *image, const char *filename)
{
	int i,j;
	//int size = image->width * image->height * 3;
	unsigned char *p = (unsigned char *)image->data;
	FILE *fpo = fopen(filename, "w");
	if (!fpo) {
		printf("ERROR: cannot open file **%s** for writing.\n", filename);
		exit(EXIT_FAILURE);
	}
	fprintf(fpo, "P3\n%d %d\n%d\n", image->width, image->height, 255);
	for (i=0; i<image->height; i++) {
		for (j=0; j<image->width; j++) {
			fprintf(fpo, "%u %u %u ",*p,*(p+1),*(p+2));
			p += 3;
		}
		fprintf(fpo, "\n");
	}
	fclose(fpo);
}  

void ppm3ClearImage(Ppmimage *image, unsigned char red, unsigned char green, unsigned char blue)
{
	int i;
	int pix = image->width * image->height;
	unsigned char *data = (unsigned char *)image->data;
	for (i=0; i<pix; i++) {
		*data++ = red;
		*data++ = green;
		*data++ = blue;
	}
}

void ppm3Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val) {
	unsigned char *data = (unsigned char *)image->data + ((y * image->width * 3) + (x * 3) + channel);
	*data = val;
}

//
//
//
//The routines below are for P6 format files.
//They store the colors in binary, rather than digital.
//Some characters will be unprintable in this format.
//Sample image file is below:
//------------------------------------------
//P6
//#4x2 red image
//4 2
//255
//a..a..a..a..a..a..a..a..
//------------------------------------------
//
//
//

Ppmimage *ppm6GetImage(const char *filename)
{
	int i, j, width, height, size, maxval, ntries;
	char ts[4096];
	//unsigned int rgb[3];
	unsigned char c;
	unsigned char *p;
	FILE *fpi;
	Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
	if (!image) {
		printf("ERROR: out of memory\n");
		exit(EXIT_FAILURE);
	}
	fpi = fopen(filename, "r");
	if (!fpi) {
		printf("ERROR: cannot open file **%s** for reading.\n", filename);
		exit(EXIT_FAILURE);
	}
	image->data=NULL;
	char *x = fgets(ts, 4, fpi);
	if (x){}
	if (strncmp(ts, "P6" ,2) != 0) {
		printf("ERROR: File is not ppm RAW format.\n");
		exit(EXIT_FAILURE);
	}
	//comments?
	while(1) {
		c = fgetc(fpi);
		if (c != '#')
			break;
		//read until newline
		ntries=0;
		while(1) {
			//to avoid infinite loop...
			if (++ntries > 10000) {
				printf("ERROR: too many blank lines in **%s**\n", filename);
				exit(EXIT_FAILURE);
			}
			c = fgetc(fpi);
			if (c == '\n')
				break;
		}
	}
	ungetc(c, fpi);
	int r = fscanf(fpi, "%u%u%u", &width, &height, &maxval);
	if (r){}
	//
	//get past any newline or carrage-return
	ntries=0;
	while(1) {
		//to avoid infinite loop...
		if (++ntries > 10000) {
			printf("ERROR: too many blank lines in **%s**\n", filename);
			exit(EXIT_FAILURE);
		}
		c = fgetc(fpi);
		if (c != 10 && c != 13) {
			ungetc(c, fpi);
			break;
		}
	}
	//
	size = width * height * 3;
	image->data = (unsigned char *)malloc(size);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}
	image->width = width;
	image->height = height;
	p = (unsigned char *)image->data;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			*p = fgetc(fpi); p++;
			*p = fgetc(fpi); p++;
			*p = fgetc(fpi); p++;
		}
	}
	fclose(fpi);
	return image;
}

void ppm6CleanupImage(Ppmimage *image)
{
	if (image != NULL) {
		if (image->data != NULL)
			free(image->data);
		free(image);
	}
}

Ppmimage *ppm6CreateImage(int width, int height)
{
	//printf("ppm6_create_image()...\n");
	int size = width * height * 3;
	Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
	if (!image) {
		printf("ERROR: no memory for image.\n");
		exit(EXIT_FAILURE);
	}
	image->data = (unsigned char *)malloc(size);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}
	image->width = width;
	image->height = height;
	return image;
}  

void ppm6SaveImage(Ppmimage *image, const char *filename)
{
	int i,j;
	//int size = image->width * image->height * 3;
	unsigned char *p = (unsigned char *)image->data;
	FILE *fpo = fopen(filename, "w");
	if (!fpo) {
		printf("ERROR: cannot open file **%s** for writing.\n", filename);
		exit(EXIT_FAILURE);
	}
	fprintf(fpo, "P6\n%d %d\n%d\n", image->width, image->height, 255);
	for (i=0; i<image->height; i++) {
		for (j=0; j<image->width; j++) {
			fputc(*p, fpo); p++;
			fputc(*p, fpo); p++;
			fputc(*p, fpo); p++;
		}
	}
	fclose(fpo);
}

void ppm6ClearImage(Ppmimage *image, unsigned char red, unsigned char green, unsigned char blue)
{
	int i;
	int pix = image->width * image->height;
	unsigned char *data = (unsigned char *)image->data;
	for (i=0; i<pix; i++) {
		*data = red; data++;
		*data = green; data++;
		*data = blue; data++;
	}
}

void ppm6Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val) {
	unsigned char *data = (unsigned char *)image->data + ((y * image->width * 3) + (x * 3) + channel);
	*data = val;
}

