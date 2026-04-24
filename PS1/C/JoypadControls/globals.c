#include "sys/types.h"
#include "globals.h"

static u_long ot[2][OT_LENGTH];
static char primBuffer[2][PRIMBUFF_LEN];
static char *nextPrim;

void EmptyOT(u_short currBuff)
{
    ClearOTagR(ot[currBuff], OT_LENGTH);
}

void SetOTAt(u_short currBuff, u_int i, u_long value)
{
    ot[currBuff][i] = value;
}

u_long *GetOTAt(u_short currBuff, u_int i)
{
    return &ot[currBuff][i];
}

void IncrementNextPrim(u_int size)
{
    nextPrim += size;
}

void SetNextPrim(char *value)
{
    nextPrim = value;
}

void ResetNextPrim(u_short currBuff)
{
    nextPrim = primBuffer[currBuff];
}

char *GetNextPrim(void)
{
    return nextPrim;
}
