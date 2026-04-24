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
POLY_F4 *poly;

void Setup(void)
{
    char *bytes;
    u_long length;
    u_long b;
    u_long i;

    ScreenInit();

    CdInit();

    JoyPadInit();

    ResetNextPrim(GetCurrentBuffer());

    setVector(&camera.position, 500, -1000, -1200);
    camera.lookat = (MATRIX){0};

    bytes = FileRead("\\MODEL.BIN;1", &length);
    printf("%d bytes were read from MODEL.BIN\n", length);

    b = 0;

    object.numVerts = GetShortBE(bytes, &b);
    object.vertices = malloc(object.numVerts * sizeof(SVECTOR));

    for (i = 0; i < object.numVerts; i++)
    {
        object.vertices[i].vx = GetShortBE(bytes, &b);
        object.vertices[i].vy = GetShortBE(bytes, &b);
        object.vertices[i].vz = GetShortBE(bytes, &b);
    }

    object.numFaces = GetShortBE(bytes, &b) * 4;  // 4 indices per quad
    object.faces = malloc(object.numFaces * sizeof(short));

    for (i = 0; i < object.numFaces; i++)
    {
        object.faces[i] = GetShortBE(bytes, &b);
        printf("Face %d = %d\n", i, object.faces[i]);
    }

    object.numColors = GetChar(bytes, &b);
    object.colors = malloc(object.numColors * sizeof(CVECTOR));
    printf("NumColors = %d\n", object.numColors);

    for (i = 0; i < object.numColors; i++)
    {
        object.colors[i].r = GetChar(bytes, &b);
        object.colors[i].g = GetChar(bytes, &b);
        object.colors[i].b = GetChar(bytes, &b);
        object.colors[i].cd = GetChar(bytes, &b);
        printf("Color %d: r = %d, g = %d, b = %d\n", i, object.colors[i].r, object.colors[i].g, object.colors[i].b);
    }

    setVector(&object.position, 0, 0, 0);
    setVector(&object.rotation, 0, 0, 0);
    setVector(&object.scale, ONE, ONE, ONE);

    free(bytes);

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
        poly = (POLY_F4*) GetNextPrim();
        setPolyF4(poly);
        setRGB0(poly, object.colors[j].r, object.colors[j].g, object.colors[j].b);

        nclip = RotAverageNclip4(&object.vertices[object.faces[i + 0]], &object.vertices[object.faces[i + 1]], &object.vertices[object.faces[i + 2]], &object.vertices[object.faces[i+3]], (long*) &poly->x0, (long*) &poly->x1, (long*) &poly->x2, (long*) &poly->x3, &p, &otz, &flg);

        if (nclip < 0)
        {
            continue;
        }

        if ((otz > 0) && (otz < OT_LENGTH))
        {
            addPrim(GetOTAt(GetCurrentBuffer(), otz), poly);
            IncrementNextPrim(sizeof(POLY_F4));
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
