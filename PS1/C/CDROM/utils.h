#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

char *FileRead(char *filename, u_long* length);

long GetLongLE(char *bytes, u_long *b);
long GetLongBE(char *bytes, u_long *b);

short GetShortLE(char *bytes, u_long *b);
short GetShortBE(char *bytes, u_long *b);

short GetChar(char *bytes, u_long *b);

#endif
