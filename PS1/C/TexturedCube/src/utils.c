#include "utils.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <libcd.h>

char *FileRead(char *filename, u_long *length)
{
    CdlFILE filepos;
    int numsectors;
    char *buffer;

    buffer = NULL;

    if (CdSearchFile(&filepos, filename) == NULL)
    {
        printf("%s file not found in the CD.\n", filename);
    }
    else
    {
        printf("Found %s in the CD.\n", filename);
        numsectors = (filepos.size + 2047) / 2048;
        buffer = (char*) malloc(2048 * numsectors);

        if(!buffer)
        {
            printf("Error allocating %d sectors!\n", numsectors);
        }

        CdControl(CdlSetloc, (u_char*) &filepos.pos, 0);
        CdRead(numsectors, (u_long*) buffer, CdlModeSpeed);
        CdReadSync(0, 0);
    }

    *length = filepos.size;

    return buffer;
}

long GetLongLE(char *bytes, u_long *b)
{
    u_long value = 0;
    value |= bytes[(*b)++] << 0;
    value |= bytes[(*b)++] << 8;
    value |= bytes[(*b)++] << 16;
    value |= bytes[(*b)++] << 24;
    return (long) value;
}

long GetLongBE(char *bytes, u_long *b)
{
    u_long value = 0;
    value |= bytes[(*b)++] << 24;
    value |= bytes[(*b)++] << 16;
    value |= bytes[(*b)++] << 8;
    value |= bytes[(*b)++] << 0;
    return (long) value;
}

short GetShortLE(char *bytes, u_long *b)
{
    unsigned short value = 0;
    value |= bytes[(*b)++] << 0;
    value |= bytes[(*b)++] << 8;
    return (short) value;
}

short GetShortBE(char *bytes, u_long *b)
{
    unsigned short value = 0;
    value |= bytes[(*b)++] << 8;
    value |= bytes[(*b)++] << 0;
    return (short) value;
}

short GetChar(char *bytes, u_long *b)
{
    return bytes[(*b)++];
}

void LoadModel(char *filename, Object *object)
{
    char *bytes;
    u_long length;
    u_long b;
    u_long i;

    bytes = FileRead(filename, &length);
    printf("%d bytes were read from MODEL\n", length);

    b = 0;

    object->numVerts = GetShortBE(bytes, &b);
    object->vertices = malloc(object->numVerts * sizeof(SVECTOR));

    for (i = 0; i < object->numVerts; i++)
    {
        object->vertices[i].vx = GetShortBE(bytes, &b);
        object->vertices[i].vy = GetShortBE(bytes, &b);
        object->vertices[i].vz = GetShortBE(bytes, &b);
    }

    object->numFaces = GetShortBE(bytes, &b) * 4;  // 4 indices per quad
    object->faces = malloc(object->numFaces * sizeof(short));

    for (i = 0; i < object->numFaces; i++)
    {
        object->faces[i] = GetShortBE(bytes, &b);
        printf("Face %d = %d\n", i, object->faces[i]);
    }

    object->numColors = GetChar(bytes, &b);
    object->colors = malloc(object->numColors * sizeof(CVECTOR));
    printf("NumColors = %d\n", object->numColors);

    for (i = 0; i < object->numColors; i++)
    {
        object->colors[i].r = GetChar(bytes, &b);
        object->colors[i].g = GetChar(bytes, &b);
        object->colors[i].b = GetChar(bytes, &b);
        object->colors[i].cd = GetChar(bytes, &b);
        printf("Color %d: r = %d, g = %d, b = %d\n", i, object->colors[i].r, object->colors[i].g, object->colors[i].b);
    }

    free(bytes);
}

TIM_IMAGE LoadTexture(char *filename)
{
    u_long *bytes;
    u_long length;
    u_long b;
    u_long i;
    TIM_IMAGE tim;


    bytes = (u_long*) FileRead(filename, &length);
    printf("%d bytes were read from Texture\n", length);

    OpenTIM(bytes);
    ReadTIM(&tim);

    LoadImage(tim.prect, tim.paddr);
    DrawSync(0);

    if (tim.mode & 0x8)
    {
        LoadImage(tim.crect, tim.caddr);
        DrawSync(0);
    }

    free(bytes);

    return tim;
}
