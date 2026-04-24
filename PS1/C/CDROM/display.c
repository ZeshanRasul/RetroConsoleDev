#include "display.h"
#include "globals.h"
#include <sys/types.h>

static DoubleBuffer screen;
static u_short currentBuffer;

u_short GetCurrentBuffer(void)
{
    return currentBuffer;
};

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

void DisplayFrame(void)
{
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[currentBuffer]);
    PutDrawEnv(&screen.draw[currentBuffer]);

    DrawOTag(GetOTAt(currentBuffer, OT_LENGTH - 1));

    currentBuffer = !currentBuffer;

    ResetNextPrim(currentBuffer);
}
