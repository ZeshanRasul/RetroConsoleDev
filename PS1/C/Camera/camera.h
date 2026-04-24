#ifndef CAMERA_H
#define CAMERA_H

#include "globals.h"

typedef struct Camera {
    VECTOR position;
    SVECTOR rotation;
    MATRIX lookat;
} Camera;

void VectorCross(VECTOR *a, VECTOR *b, VECTOR *out);

void LookAt(Camera *camera, VECTOR *eye, VECTOR *target, VECTOR *up);



#endif
