#include <stdlib.h>
#include <LIBETC.H>
#include "joypad.h"
#include "display.h"
#include "camera.h"
#include "globals.h"
#include "utils.h"
#include "object.h"

Camera camera;
MATRIX view;
MATRIX world;

Object object;
TIM_IMAGE tim;
POLY_FT4 *polyft4;

void Setup(void)
{
    ScreenInit();

    CdInit();

    JoyPadInit();

    ResetNextPrim(GetCurrentBuffer());

    setVector(&camera.position, 500, -1000, -1200);
    camera.lookat = (MATRIX){0};

    setVector(&object.position, 0, 0, 0);
    setVector(&object.rotation, 0, 0, 0);
    setVector(&object.scale, ONE, ONE, ONE);

    LoadModel("\\MODEL.BIN;1", &object);

    tim = LoadTexture("\\BRICKS.TIM;1");
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

    if (JoyPadCheck(PAD1_LEFT))
    {
        camera.position.vx -= 50;
    }

    if (JoyPadCheck(PAD1_RIGHT))
    {
        camera.position.vx += 50;
    }

    if (JoyPadCheck(PAD1_UP))
    {
        camera.position.vy -= 50;
    }

    if (JoyPadCheck(PAD1_DOWN))
    {
        camera.position.vy += 50;
    }

    if (JoyPadCheck(PAD1_CROSS))
    {
        camera.position.vz += 50;
    }

    if (JoyPadCheck(PAD1_CIRCLE))
    {
        camera.position.vz -= 50;
    }

    LookAt(&camera, &camera.position, &object.position, &(VECTOR){0, -ONE, 0});

    RotMatrix(&object.rotation, &world);
    TransMatrix(&world, &object.position);
    ScaleMatrix(&world, &object.scale);

    CompMatrixLV(&camera.lookat, &world, &view);

    SetRotMatrix(&view);
    SetTransMatrix(&view);


    for (i = 0, j = 0; i < object.numFaces; i += 4, j++)
    {
        polyft4 = (POLY_FT4*) GetNextPrim();
        setPolyFT4(polyft4);
        setRGB0(polyft4, 128, 128, 128);

        polyft4->u0 = 0;
        polyft4->v0 = 0;

        polyft4->u1 = 63;
        polyft4->v1 = 0;

        polyft4->u2 = 0;
        polyft4->v2 = 63;

        polyft4->u3 = 63;
        polyft4->v3 = 63;

        polyft4->tpage = getTPage(tim.mode & 0x3, 0, tim.prect->x, tim.prect->y);
        polyft4->clut = getClut(tim.crect->x, tim.crect->y);

        nclip = RotAverageNclip4(&object.vertices[object.faces[i + 0]], &object.vertices[object.faces[i + 1]], &object.vertices[object.faces[i + 2]], &object.vertices[object.faces[i+3]], (long*) &polyft4->x0, (long*) &polyft4->x1, (long*) &polyft4->x2, (long*) &polyft4->x3, &p, &otz, &flg);

        if (nclip < 0)
        {
            continue;
        }

        if ((otz > 0) && (otz < OT_LENGTH))
        {
            addPrim(GetOTAt(GetCurrentBuffer(), otz), polyft4);
            IncrementNextPrim(sizeof(POLY_FT4));
        }
    }

    object.rotation.vy += 20;
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
