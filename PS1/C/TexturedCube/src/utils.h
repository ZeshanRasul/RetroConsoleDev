#ifndef UTILS_H
#define UTILS_H

#include "object.h"
#include <libgpu.h>

char *FileRead(char *filename, u_long *length);

long GetLongLE(char *bytes, u_long *b);
long GetLongBE(char *bytes, u_long *b);

short GetShortLE(char *bytes, u_long *b);
short GetShortBE(char *bytes, u_long *b);

short GetChar(char *bytes, u_long *b);

void LoadModel(char *filename, Object *object);
TIM_IMAGE LoadTexture(char *filename);

#endif
