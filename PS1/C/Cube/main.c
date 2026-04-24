#include <stdlib.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <LIBGPU.H>

#define VIDEO_MODE 1
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 400
#define OT_LENGTH 256

#define NUM_VERTICES 8
#define NUM_FACES 6

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

typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
} DoubleBuffer;

DoubleBuffer screen;
u_short currentBuffer;

u_long ot[2][OT_LENGTH];

char primBuffer[2][2048];
char *nextPrim;

POLY_G4 *poly;

SVECTOR rotation = {0, 0, 0};
VECTOR translation = {0, 0, 900};
VECTOR scale = { ONE, ONE, ONE};

MATRIX world = {0};

void ScreenInit(void)
{
    ResetGraph(0);

    SetDefDispEnv(&screen.disp[0], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);
    SetDefDrawEnv(&screen.draw[0], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);

    SetDefDispEnv(&screen.disp[1], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);
    SetDefDrawEnv(&screen.draw[1], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);

    screen.draw[0].isbg = 1;
    screen.draw[1].isbg = 1;

    setRGB0(&screen.draw[0], 63, 0, 127);
    setRGB0(&screen.draw[1], 63, 0, 127);

    currentBuffer = 0;
    PutDispEnv(&screen.disp[currentBuffer]);
    PutDrawEnv(&screen.draw[currentBuffer]);

    InitGeom();
    SetGeomOffset(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    SetGeomScreen(SCREEN_Z);

    SetDispMask(1);
}

void DisplayFrame(Void)
{
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[currentBuffer]);
    PutDrawEnv(&screen.draw[currentBuffer]);

    DrawOTag(ot[currentBuffer] + OT_LENGTH - 1);

    currentBuffer = !currentBuffer;

    nextPrim = primBuffer[currentBuffer];
}

void Setup(void)
{
    ScreenInit();

    nextPrim = primBuffer[currentBuffer];
}

void Update(void)
{
    int i = 0;
    int nclip;
    long otz = 0;
    long p;
    long flg;

    ClearOTagR(ot[currentBuffer], OT_LENGTH);

    RotMatrix(&rotation, &world);
    TransMatrix(&world, &translation);
    ScaleMatrix(&world, &scale);

    SetRotMatrix(&world);
    SetTransMatrix(&world);

    for (i = 0; i < NUM_FACES * 4; i+= 4)
    {
        poly = (POLY_G4*) nextPrim;
        setPolyG4(poly);
        setRGB0(poly, 255, 0, 255);
        setRGB1(poly, 255, 255, 0);
        setRGB2(poly, 0, 255, 255);
        setRGB3(poly, 0, 255, 0);

        nclip = RotAverageNclip4(&vertices[faces[i + 0]], &vertices[faces[i + 1]], &vertices[faces[i + 2]], &vertices[faces[i+3]], (long*) &poly->x0, (long*) &poly->x1, (long*) &poly->x2, (long*) &poly->x3, &p, &otz, &flg);
        if (nclip < 0)
        {
            continue;
        }

        if ((otz > 0) && (otz < OT_LENGTH))
        {
            addPrim(ot[currentBuffer][otz], poly);
            nextPrim += sizeof(POLY_G4);
        }
    }

    rotation.vx += 6;
    rotation.vy += 8;
    rotation.vz += 12;
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
