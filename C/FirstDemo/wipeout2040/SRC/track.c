#include "track.h"
#include "inline_n.h"
#include "texture.h"

#define BYTES_PER_VERTEX    16
#define BYTES_PER_FACE      20
#define BYTES_PER_SECTION   156

void LoadTrackVertices(Track *track, char *filename)
{
    u_long i;
    u_long b;
    u_long length;
    u_char *bytes;

    bytes = (u_char*) FileRead(filename, &length);

    if (bytes == NULL)
    {
        printf("Error reading %s from the CD.\n", filename);
        return;
    }

    b = 0;

    track->numvertices = length / BYTES_PER_VERTEX;
    track->vertices = (VECTOR*) malloc(track->numvertices * sizeof(VECTOR));

    for (i = 0; i < track->numvertices; i++)
    {
        track->vertices[i].vx = GetLongBE(bytes, &b);
        track->vertices[i].vy = GetLongBE(bytes, &b);
        track->vertices[i].vz = GetLongBE(bytes, &b);
        track->vertices[i].pad = GetLongBE(bytes, &b);
    }

    free(bytes);
}

void LoadTrackFaces(Track *track, char *filename, u_short texturestart)
{
    u_long i;
    u_long b;
    u_long length;
    u_char *bytes;
    Texture *texture;

    bytes = (u_char*) FileRead(filename, &length);

    if (bytes == NULL)
    {
        printf("Error reading %s from the CD.\n", filename);
        return;
    }

    b = 0;

    track->numfaces = length / BYTES_PER_FACE;
    track->faces = (Face*) malloc(track->numfaces * sizeof(Face));

    for (i = 0; i < track->numfaces; i++)
    {
        Face *face = &track->faces[i];
        face->indices[0] = GetShortBE(bytes, &b);
        face->indices[1] = GetShortBE(bytes, &b);
        face->indices[2] = GetShortBE(bytes, &b);
        face->indices[3] = GetShortBE(bytes, &b);

        face->normal.vx = GetShortBE(bytes, &b);
        face->normal.vy = GetShortBE(bytes, &b);
        face->normal.vz = GetShortBE(bytes, &b);

        face->texture = GetChar(bytes, &b);

        face->flags = GetChar(bytes, &b);

        face->color.r = GetChar(bytes, &b);
        face->color.g = GetChar(bytes, &b);
        face->color.b = GetChar(bytes, &b);
        face->color.cd = GetChar(bytes, &b);

        face->texture += texturestart;
        texture = GetFromTextureStore(face->texture);
        face->tpage = texture->tpage;
        face->clut = texture->clut;

        if (face->flags & FACE_FLIP_TEXTURE)
        {
            face->u0 = texture->u1;
            face->v0 = texture->v1;
            face->u1 = texture->u0;
            face->v1 = texture->v0;
            face->u2 = texture->u3;
            face->v2 = texture->v3;
            face->u3 = texture->u2;
            face->v3 = texture->v2;
        }
        else
        {
            face->u0 = texture->u0;
            face->v0 = texture->v0;
            face->u1 = texture->u1;
            face->v1 = texture->v1;
            face->u2 = texture->u2;
            face->v2 = texture->v2;
            face->u3 = texture->u3;
            face->v3 = texture->v3;
        }
    }
    free(bytes);
}

void LoadTrackSections(Track *track, char *filename)
{
    u_long i;
    u_long b;
    u_long length;
    u_char *bytes;

    bytes = (u_char*) FileRead(filename, &length);

    if (bytes == NULL)
    {
        printf("Error reading %s from the CD.\n", filename);
        return;
    }

    b = 0;

    track->numsections = length / BYTES_PER_SECTION;
    track->sections = (Section*) malloc(track->numsections * sizeof(Section));

    for (i = 0; i < track->numsections; i++)
    {
        b += 4;

        track->sections[i].prev = track->sections + GetLongBE(bytes, &b);
        track->sections[i].next = track->sections + GetLongBE(bytes, &b);

        track->sections[i].center.vx = GetLongBE(bytes, &b);
        track->sections[i].center.vy = GetLongBE(bytes, &b);
        track->sections[i].center.vz = GetLongBE(bytes, &b);

        b += 118;

        track->sections[i].facestart = GetShortBE(bytes, &b);
        track->sections[i].numfaces = GetShortBE(bytes, &b);

        b += 4;

        track->sections[i].flags = GetShortBE(bytes, &b);
        track->sections[i].id = GetShortBE(bytes, &b);
        track->sections[i].id = i;

        b += 2;
    }
    free(bytes);
}

void RenderTrackSection(Track *track, Section *section, Camera *camera)
{
    u_long i;
    short nclip;
    long otz;
    long p;
    long flg;
    POLY_FT4* poly;
    LINE_F2 *line0, *line1, *line2, *line3;
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR v3;

    CVECTOR facecolor;

    MATRIX worldmat;
    MATRIX viewmat;
    MATRIX transmat;

    VECTOR pos;
    VECTOR scale;
    SVECTOR rot;

    setVector(&pos, 0, 0, 0);
    setVector(&rot, 0, 0, 0);
    setVector(&scale, ONE, ONE, ONE);

    worldmat = (MATRIX){0};

    RotMatrix(&rot, &worldmat);
    TransMatrix(&worldmat, &pos);
    ScaleMatrix(&worldmat, &scale);

    CompMatrixLV(&camera->rotmat, &worldmat, &viewmat); // combine world and lookat transform

    SetRotMatrix(&viewmat);
    SetTransMatrix(&viewmat);

    for (i = 0; i < section->numfaces; i++)
    {
        Face *face = track->faces + section->facestart + i;

        v0.vx = (short)(track->vertices[face->indices[1]].vx - camera->position.vx);
        v0.vy = (short)(track->vertices[face->indices[1]].vy - camera->position.vy);
        v0.vz = (short)(track->vertices[face->indices[1]].vz - camera->position.vz);

        v1.vx = (short)(track->vertices[face->indices[0]].vx - camera->position.vx);
        v1.vy = (short)(track->vertices[face->indices[0]].vy - camera->position.vy);
        v1.vz = (short)(track->vertices[face->indices[0]].vz - camera->position.vz);

        v2.vx = (short)(track->vertices[face->indices[2]].vx - camera->position.vx);
        v2.vy = (short)(track->vertices[face->indices[2]].vy - camera->position.vy);
        v2.vz = (short)(track->vertices[face->indices[2]].vz - camera->position.vz);

        v3.vx = (short)(track->vertices[face->indices[3]].vx - camera->position.vx);
        v3.vy = (short)(track->vertices[face->indices[3]].vy - camera->position.vy);
        v3.vz = (short)(track->vertices[face->indices[3]].vz - camera->position.vz);

        poly = (POLY_FT4*) GetNextPrim();
        gte_ldv0(&v0);
        gte_ldv1(&v1);
        gte_ldv2(&v2);
        gte_rtpt();
        gte_nclip();
        gte_stopz(&nclip);
        if (nclip < 0) {
          continue;
        }
        gte_stsxy0(&poly->x0);
        gte_ldv0(&v3);
        gte_rtps();
        gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
        gte_avsz4();
        gte_stotz(&otz);
        if (otz > 0 && otz < OT_LEN) {
            setPolyFT4(poly);

            setRGB0(poly, face->color.r, face->color.g, face->color.b);
            poly->tpage = face->tpage;
            poly->clut = face->clut;
            setUV4(poly, face->u0, face->v0, face->u1, face->v1, face->u2, face->v2, face->u3, face->v3);

            addPrim(GetOTAt(GetCurrBuff(), otz), poly);
            IncrementNextPrim(sizeof(POLY_FT4));
        }
    }
}

long Clamp16Bits(long value)
{
    if (value > +32767) return +32767;
    if (value < -32767) return -32767;
    return value;
}

void RenderTrack(Track *track, Camera *camera)
{
    Section *currsection;
    VECTOR d;
    u_long distmagsq;
    u_long distmag;

    currsection = track->sections;

    do {
        d.vx = Clamp16Bits(currsection->center.vx - camera->position.vx);
        d.vy = Clamp16Bits(currsection->center.vy - camera->position.vy);
        d.vz = Clamp16Bits(currsection->center.vz - camera->position.vz);
        distmagsq = (d.vx * d.vx) + (d.vy * d.vy) + (d.vz * d.vz);
        distmag = SquareRoot12(distmagsq);

        if (distmag < 950000)
        {
            RenderTrackSection(track, currsection, camera);
        }

        currsection = currsection->next;
    } while (currsection != track->sections);
}

