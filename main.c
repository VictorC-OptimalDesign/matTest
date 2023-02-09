// === INCLUDES ================================================================

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imu.h"


// === MAIN ====================================================================

int main(int argc, char* argv[])
{
    printf("\n%s(%d,[", __func__, argc);
    for (int i = 0; i < argc; ++i)
        printf("%s,", argv[i]);
    printf("])\n");

    debugData_t debugData;
    //setDataPointers(0x0574, 0x574);
    setDataPointers(0x0a15, 0x0a15);
    saveBufferToFlash(debugData);
}
