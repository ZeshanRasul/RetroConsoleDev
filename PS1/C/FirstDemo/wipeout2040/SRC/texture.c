#include "texture.h"
#include "globals.h"
#include "lzss.h"
#include <stdlib.h>

static Texture *texturestore[MAX_TEXTURES];
static u_long texturecount = 0;

static u_short textx = 320;
static u_short texty = 0;
static u_short clutx = 320;
static u_short cluty = 256;

Texture *GetFromTextureStore(u_int i) {
  return texturestore[i];
}

u_short GetTextureCount(void) {
  return texturecount;
}

void LoadTextureCMP(char *filenamecmp, char *filenamettf)
{
    u_char *bytes;
    u_long length;
    u_long b;
    u_long i;
    u_short numtextures;
    long *textureoffsets;
    u_long totaltexturesizes = 0;
    static void *timsbaseaddr;
    static Texture *texture;

    bytes = (u_char*) FileRead(filenamecmp, &length);

    if (bytes == NULL)
    {
        printf("Error reading %s from the CD\n", filenamecmp);
        return;
    }
    else
    {
        printf("%d bytes were read from %s\n", length, filenamecmp);
    }

    b = 0;

    numtextures = GetLongLE(bytes, &b);


    textureoffsets = (long*) malloc(numtextures * sizeof(long));

    for (i = 0; i < numtextures; i++)
    {
        u_long timsize;

        textureoffsets[i] = totaltexturesizes;
        timsize = GetLongLE(bytes, &b);

        totaltexturesizes += timsize;
    }

    timsbaseaddr = (char*) malloc(totaltexturesizes);

    for (i = 0; i < numtextures; i++)
    {
        textureoffsets[i] += (long) timsbaseaddr;
    }

    ExpandLZSSData(&bytes[b], timsbaseaddr);

    free(bytes);

    if (filenamettf == NULL)
    {
        for (i = 0; i < numtextures; i++)
        {
            texture = UploadTextureToVRAM(textureoffsets[i]);
            if (texture != NULL)
            {
                texturestore[texturecount++] = texture;
            }
            if (texturecount > MAX_TEXTURES)
            {
                printf("Texture store is full!\n");
            }
        }
    }

    if (filenamettf != NULL)
    {
        Tile *tiles;
        u_short numtiles;
        bytes = (u_char*) FileRead(filenamettf, &length);
        if (bytes == NULL)
        {
            printf("Error reading %s from the CD.\n", filenamettf);
            return;
        }
        numtiles = length / BYTES_PER_TILE;

        tiles = (Tile*) malloc(numtiles * sizeof(Tile));

        b = 0;

        for (i = 0; i < numtiles; i++)
        {
            b += 16 * 2; // Skip hi-res tile indices
            b += 4 * 2; // Skil mid-res tile indices

            tiles[i].tileindex = GetShortBE(bytes, &b);

            texture = UploadTextureToVRAM(textureoffsets[tiles[i].tileindex]);
            if (texture != NULL)
            {
                texturestore[texturecount++] = texture;
            }
            if (texturecount > MAX_TEXTURES)
            {
                printf("Texture store is full!\n");
            }
        }
        free(tiles);
        free(bytes);
    }

    free(timsbaseaddr);
}

Texture *UploadTextureToVRAM(long timptr) {
  Tim *tim;
  Texture *texture;
  u_short x, y;
  RECT rect;
  tim = (Tim*) timptr;
  switch (ClutType(tim)) {
    case CLUT_4BIT: {
      TimClut4 *tc4;
      tc4 = (TimClut4*) tim;
      texture = (Texture*) malloc(sizeof(Texture));
      texture->type = CLUT4;

      if (!tc4->textureX && !tc4->textureY)
      {
        tc4->textureX = textx;
        tc4->textureY = texty;
        tc4->clutX = clutx;
        tc4->clutY = cluty;
        tc4->clutW = 16;
        tc4->clutH = 1;

        clutx += 16;
        if (clutx >= 384)
        {
            clutx = 320;
            cluty += 1;
        }

        texty += 32;
        if (texty >= 256)
        {
            textx += 8;
            texty = 0;
        }
      }

      texture->textureX = tc4->textureX;
      texture->textureY = tc4->textureY;
      texture->textureW = tc4->textureW;
      texture->textureH = tc4->textureH;
      texture->clutX    = tc4->clutX;
      texture->clutY    = tc4->clutY;
      texture->clutW    = tc4->clutW;
      texture->clutH    = tc4->clutH;

      x = tc4->textureX - TextureHOffset(tc4->textureX);
      y = tc4->textureY - TextureVOffset(tc4->textureY);

      texture->u0 = (x << 2);
      texture->v0 = (y);
      texture->u1 = ((x + tc4->textureW) << 2) - 1;
      texture->v1 = (y);
      texture->u2 = (x << 2);
      texture->v2 = (y + tc4->textureH) - 1;
      texture->u3 = ((x + tc4->textureW) << 2) - 1;
      texture->v3 = (y + tc4->textureH) - 1;

      texture->tpage = TPAGE(CLUT_4BIT, TRANSLUCENT, texture->textureX, texture->textureY);
      texture->clut  = CLUT(texture->clutX >> 4, texture->clutY);

      // Load the CLUT rectangle to VRAM
      rect.x = tc4->clutX;
      rect.y = tc4->clutY;
      rect.w = tc4->clutW;
      rect.h = tc4->clutH;
      LoadImage(&rect, (u_long*)(&tc4->clut));
      DrawSync(0);

      // Load the Texture rectangle to VRAM
      rect.x = tc4->textureX;
      rect.y = tc4->textureY;
      rect.w = tc4->textureW;
      rect.h = tc4->textureH;
      LoadImage(&rect, (u_long*)(tc4 + 1));
      DrawSync(0);

      break;
    }
    case CLUT_8BIT: {
      TimClut8 *tc8;
      tc8 = (TimClut8*) tim;
      texture = (Texture*) malloc(sizeof(Texture));
      texture->type = CLUT8;
      texture->textureX = tc8->textureX;
      texture->textureY = tc8->textureY;
      texture->textureW = tc8->textureW;
      texture->textureH = tc8->textureH;
      texture->clutX    = tc8->clutX;
      texture->clutY    = tc8->clutY;
      texture->clutW    = tc8->clutW;
      texture->clutH    = tc8->clutH;

      x = tc8->textureX - TextureHOffset(tc8->textureX);
      y = tc8->textureY - TextureVOffset(tc8->textureY);

      texture->u0 = (x << 1);
      texture->v0 = (y);
      texture->u1 = ((x + tc8->textureW) << 1) - 1;
      texture->v1 = (y);
      texture->u2 = (x << 1);
      texture->v2 = (y + tc8->textureH) - 1;
      texture->u3 = ((x + tc8->textureW) << 1) - 1;
      texture->v3 = (y + tc8->textureH) - 1;

      texture->tpage = TPAGE(CLUT_8BIT, TRANSLUCENT, texture->textureX, texture->textureY);
      texture->clut  = CLUT(texture->clutX >> 4, texture->clutY);

      // Load the CLUT rectangle to VRAM
      rect.x = tc8->clutX;
      rect.y = tc8->clutY;
      rect.w = tc8->clutW;
      rect.h = tc8->clutH;
      LoadImage(&rect, (u_long*)(&tc8->clut));
      DrawSync(0);

      // Load the Texture rectangle to VRAM
      rect.x = tc8->textureX;
      rect.y = tc8->textureY;
      rect.w = tc8->textureW;
      rect.h = tc8->textureH;
      LoadImage(&rect, (u_long*)(tc8 + 1));
      DrawSync(0);

      break;
    }
  }
  return texture;
}
