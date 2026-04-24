#include <stdlib.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <LIBGPU.H>

#define VIDEO_MODE 1
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)

typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
} DoubleBuffer;

DoubleBuffer screen;
short currentBuffer;

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
    SetGeomScreen(SCREEN_CENTER_X);

    SetDispMask(1);
}

void DisplayFrame(Void)
{
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[currentBuffer]);
    PutDrawEnv(&screen.draw[currentBuffer]);

    currentBuffer = !currentBuffer;
}

void Setup(void)
{
    ScreenInit();
}

void Update(void)
{

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