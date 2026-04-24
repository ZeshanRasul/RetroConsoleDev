#include <stdlib.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <LIBGPU.H>
#include "globals.h"
#include "joypad.h"
#include "display.h"



#define NUM_VERTICES 8
#define NUM_FACES 6

typedef struct Cube {
    SVECTOR rotation;
    VECTOR position;
    VECTOR scale;
    VECTOR vel;
    VECTOR acc;
    SVECTOR vertices[8];
    short faces[24];
    MATRIX world;
} Cube;

typedef struct Floor {
    SVECTOR rotation;
    VECTOR position;
    VECTOR scale;
    SVECTOR vertices[4];
    short faces[6];
    MATRIX world;
} Floor;

SVECTOR vertices[] = {
    { -128, -128, -128 },
    { 128, -128, -128},
    { 128, -128, 128},
    { -128, -128, 128},
    { -128, 128, -128},
    { 128, 128, -128},
    { 128, 128, 128},
    { -128, 128, 128}
};

short faces[] = {
    3, 2,
    0, 1,
    0, 1,
    4, 5,
    4, 5,
    7, 6,
    1, 2,
    5, 6,
    2, 3,
    6, 7,
    3, 0,
    7, 4
};

SVECTOR triVertices[] = {
    0, 300, -128,
    400, 300, -128,
    0, 100, 120
};

POLY_G4 *poly;
POLY_F3 *tri;

Cube cube = {
    {0, 0, 0},
    {0, -400, 1800},
    {ONE, ONE, ONE},
    {0, 0, 0},
    {0, 1, 0},
    {
        { -128, -128, -128 },
        { 128, -128, -128},
        { 128, -128, 128},
        { -128, -128, 128},
        { -128, 128, -128},
        { 128, 128, -128},
        { 128, 128, 128},
        { -128, 128, 128}
    },
    {
        3, 2,
        0, 1,
        0, 1,
        4, 5,
        4, 5,
        7, 6,
        1, 2,
        5, 6,
        2, 3,
        6, 7,
        3, 0,
        7, 4
    },
    {0}
};

Floor floor = {
    {0, 0, 0},
    {0, 450, 1800},
    {ONE, ONE, ONE},
    {
        {-900, 0, -900},
        {-900, 0, 900},
        {900, 0, -900},
        {900, 0, 900}
    },
    {
        0, 1, 2,
        1, 3, 2
    },
    {0}
};

void Setup(void)
{
    ScreenInit();

    JoyPadInit();

    ResetNextPrim(GetCurrentBuffer());
}

void Update(void)
{
    int i = 0;
    int j = 0;
    int nclip;
    long otz = 0;
    long p;
    long flg;

    EmptyOT(GetCurrentBuffer());

    JoyPadUpdate();

    if (JoyPadCheck(_PAD(0, PADLleft)))
    {
        cube.rotation.vy += 20;
    }

    if (JoyPadCheck(_PAD(0, PADLright)))
    {
        cube.rotation.vy -= 20;
    }

    RotMatrix(&floor.rotation, &floor.world);
    TransMatrix(&floor.world, &floor.position);
    ScaleMatrix(&floor.world, &floor.scale);

    SetRotMatrix(&floor.world);
    SetTransMatrix(&floor.world);

    for (j = 0; j < 2 * 3; j += 3)
    {
        tri = (POLY_F3*) GetNextPrim();
        setPolyF3(tri);
        setRGB0(tri, 255, 0, 0);

        nclip = RotAverageNclip3(&floor.vertices[floor.faces[j]], &floor.vertices[floor.faces[j + 1]], &floor.vertices[floor.faces[j + 2]], (long*) &tri->x0, (long*) &tri->x1, (long*) &tri->x2, &p, &otz, &flg);

        if (nclip < 0)
        {
            continue;
        }

        if ((otz > 0) && (otz < OT_LENGTH))
        {
            addPrim(GetOTAt(GetCurrentBuffer(), otz), tri);
            IncrementNextPrim(sizeof(POLY_F3));
        }
    }

    cube.vel.vx += cube.acc.vx;
    cube.vel.vy += cube.acc.vy;
    cube.vel.vz += cube.acc.vz;

    cube.position.vx += cube.vel.vx;
    cube.position.vy += cube.vel.vy;
    cube.position.vz += cube.vel.vz;

    if (cube.position.vy + 150 > floor.position.vy)
    {
         cube.vel.vy *= -1;
    }

    RotMatrix(&cube.rotation, &cube.world);
    TransMatrix(&cube.world, &cube.position);
    ScaleMatrix(&cube.world, &cube.scale);

    SetRotMatrix(&cube.world);
    SetTransMatrix(&cube.world);

    for (i = 0; i < 6 * 4; i += 4)
    {
        poly = (POLY_G4*) GetNextPrim();
        setPolyG4(poly);
        setRGB0(poly, 255, 0, 255);
        setRGB1(poly, 255, 255, 0);
        setRGB2(poly, 0, 255, 255);
        setRGB3(poly, 0, 255, 0);

        nclip = RotAverageNclip4(&cube.vertices[cube.faces[i + 0]], &cube.vertices[cube.faces[i + 1]], &cube.vertices[cube.faces[i + 2]], &cube.vertices[cube.faces[i+3]], (long*) &poly->x0, (long*) &poly->x1, (long*) &poly->x2, (long*) &poly->x3, &p, &otz, &flg);
        if (nclip < 0)
        {
            continue;
        }

        if ((otz > 0) && (otz < OT_LENGTH))
        {
            addPrim(GetOTAt(GetCurrentBuffer(), otz), poly);
            IncrementNextPrim(sizeof(POLY_G4));
        }
    }
}

void Render(void)
{
    DisplayFrame();
}

int main(void)
{
    Setup();

    while (1)
    {
        Update();
        Render();
    }

    return 0;
}
