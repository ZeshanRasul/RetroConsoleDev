#ifndef TRACK_H
#define TRACK_H

#include "globals.h"
#include "camera.h"
#include <stddef.h>

#define FACE_TRACK_BASE         1
#define FACE_WEAPON_GRID_LEFT   2
#define FACE_FLIP_TEXTURE       4
#define FACE_WEAPON_GRID_RIGHT  8
#define FACE_START_GRID         16
#define FACE_SPEED_UP           32

typedef struct Face {
    short indices[4];
    char flags;
    SVECTOR normal;
    CVECTOR color;
    char texture;
    short clut;
    short tpage;
    short u0, v0;
    short u1, v1;
    short u2, v2;
    short u3, v3;
} Face;

typedef struct Section {
    short id;
    short flags;

    struct Section *prev;
    struct Section *next;

    VECTOR center;
    SVECTOR normal;

    short numfaces;
    short facestart;
} Section;

typedef struct Track {
    long numvertices;
    VECTOR *vertices;

    long numfaces;
    Face *faces;

    long numsections;
    Section *sections;
} Track;

void LoadTrackVertices(Track *track, char *filename);
void LoadTrackSections(Track *track, char *filename);
void LoadTrackFaces(Track *track, char *filename, u_short texturestart);

void RenderTrack(Track *track, Camera *camera);

#endif
