#ifndef OBJECT_H
#define OBJECT_H

#include <sys/types.h>
#include <LIBGTE.H>

typedef struct Object {
    SVECTOR rotation;
    VECTOR position;
    VECTOR scale;

    MATRIX world;

    short numVerts;
    SVECTOR *vertices;

    short numFaces;
    short *faces;

    char numColors;
    CVECTOR *colors;
} Object;

#endif
