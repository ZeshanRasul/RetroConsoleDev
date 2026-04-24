#ifndef GLOBALS_H
#define GLOBALS_H
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

#define OT_LEN 8192
#define PRIMBUFF_LEN 131072

void EmptyOT(u_short currBuff);

void SetOTAt(u_short currBuff, u_int i, u_long value);
u_long *GetOTAt(u_short currBuff, u_int i);

void IncrementNextPrim(u_int size);
void SetNextPrim(char *value);
void ResetNextPrim(u_short currBuff);
char *GetNextPrim(void);

#endif
