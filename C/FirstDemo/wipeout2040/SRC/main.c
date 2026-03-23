#include <stdlib.h>
#include <LIBETC.H>
#include "joypad.h"
#include "display.h"
#include "camera.h"
#include "globals.h"
#include "utils.h"
#include "object.h"
#include "ship.h"
#include "texture.h"
#include "track.h"
#include <stdlib.h>

Camera camera;

Track track;

Object *ships;

Ship ship;

u_short shipindex = 0;

Object *sceneobjs;
Object *sceneobj;

u_short objectcount;

u_short angle;

int holdingright = 0;
int holdingleft = 0;

void Setup(void)
{
    VECTOR startpos;
    u_short shipstarttexture;
    u_short scenestarttexture;
    u_short trackstarttexture;

    ScreenInit();

    CdInit();

    JoyPadInit();

    ResetNextPrim(GetCurrBuff());

    shipstarttexture = GetTextureCount();
    LoadTextureCMP("\\ALLSH.CMP;1", NULL);

    scenestarttexture = GetTextureCount();
    LoadTextureCMP("\\TRACK02\\SCENE.CMP;1", NULL);

    ships = LoadObjectPRM("\\ALLSH.PRM;1", shipstarttexture);

    trackstarttexture = GetTextureCount();
    LoadTextureCMP("\\TRACK02\\LIBRARY.CMP;1", "\\TRACK02\\LIBRARY.TTF;1");

    LoadTrackVertices(&track, "\\TRACK02\\TRACK.TRV;1");
    LoadTrackFaces(&track, "\\TRACK02\\TRACK.TRF;1", trackstarttexture);
    LoadTrackSections(&track, "\\TRACK02\\TRACK.TRS;1");

    printf("NUM TRACK VERTICES: %d\n", track.numvertices);
    printf("NUM TRACK FACES: %d\n", track.numfaces);
    printf("NUM TRACK SECTIONS: %d\n", track.numsections);

    ship.object = GetObjectByIndex(ships, shipindex);

    setVector(&startpos, 32599, -347, -45310);

    ShipInit(&ship, &track, &startpos);

    setVector(&camera.position, ship.object->position.vx, ship.object->position.vy - 200, ship.object->position.vz - 800);
    camera.lookat = (MATRIX){0};
    camera.rotmat = (MATRIX){0};
}

void Update(void)
{
    u_short i;

    EmptyOT(GetCurrBuff());

    JoyPadUpdate();

    if (JoyPadCheck(PAD1_LEFT))
    {
        ship.object->rotation.vy -= 10;
    }

    if (JoyPadCheck(PAD1_RIGHT))
    {
        ship.object->rotation.vy += 10;
    }

    if (JoyPadCheck(PAD1_UP))
    {
        ship.object->rotation.vx -= 10;
    }

    if (JoyPadCheck(PAD1_DOWN))
    {
        ship.object->rotation.vx += 10;
    }

    if (JoyPadCheck(PAD1_CROSS))
    {
        ship.thrustmag += 10;
    }
    else if (ship.thrustmag > 0)
    {
        ship.thrustmag -= 10;
    }

    if (ship.thrustmag > ship.thrustmax)
    {
        ship.thrustmag = ship.thrustmax;
    }

    if (ship.thrustmag < 0)
    {
        ship.thrustmag = 0;
    }

    ShipUpdate(&ship);

    camera.position.vx = ship.object->position.vx;
    camera.position.vy = ship.object->position.vy - 500;
    camera.position.vz = ship.object->position.vz - 800;

    LookAt(&camera, &camera.position, &ship.object->position, &(VECTOR){0, -ONE, 0});

    RenderTrack(&track, &camera);
    RenderObject(ship.object, &camera);
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
