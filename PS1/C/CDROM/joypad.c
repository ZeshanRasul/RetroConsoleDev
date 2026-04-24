#include "joypad.h"
#include "sys/types.h"

static u_long padState;


int JoyPadCheck(int p)
{
    return padState & p;
}

void JoyPadInit(void)
{
    PadInit(0);
}

void JoyPadReset(void)
{
    padState = 0;
}

void JoyPadUpdate(void)
{
    u_long pad = PadRead(0);
    padState = pad;
}
