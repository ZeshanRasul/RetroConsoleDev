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
#define OT_LENGTH 16

typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
} DoubleBuffer;

DoubleBuffer screen;
u_short currentBuffer;

POLY_F3 *triangle;
TILE    *tile;
POLY_G4 *quadG;

u_long ot[2][OT_LENGTH];

char primBuffer[2][2048];
char *nextPrim;

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
    ClearOTagR(ot[currentBuffer], OT_LENGTH);

    tile = (TILE*) nextPrim;
    setTile(tile);
    setXY0(tile, 32, 32);
    setWH(tile, 64, 64);
    setRGB0(tile, 185, 13, 249);
    addPrim(ot[currentBuffer], tile);

    nextPrim += sizeof(TILE);

    triangle = (POLY_F3*) nextPrim;
    setPolyF3(triangle);
    setXY3(triangle, 64, 100, 200, 150, 50, 220);
    setRGB0(triangle, 2, 225, 30);
    addPrim(ot[currentBuffer], triangle);

    nextPrim += sizeof(POLY_F3);

    quadG = (POLY_G4*) nextPrim;
    setPolyG4(quadG);
    setXY4(quadG, 150, 150, 250, 150,  150, 50, 250, 50);
    setRGB0(quadG, 0, 255, 0);
    setRGB1(quadG, 0, 0, 255);
    setRGB2(quadG, 133, 0, 45);
    setRGB3(quadG, 255, 0, 0);
    addPrim(ot[currentBuffer], quadG);

    nextPrim += sizeof(POLY_G4);
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
