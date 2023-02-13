// === INCLUDES ================================================================

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imu.h"


// === TYPE DEFINES ============================================================




// === PRIVATE FUNCTIONS =======================================================

static imuArgs_t processMovingAverageArgs(int argc, char* argv[])
{
#if true
    // Hardcoded default args.
    imuArgs_t args =
    {
        .rollover = true,
        .head = 0x0a15,
        .tail = 0x0a15,
    };
#else
    imuArgs_t args =
    {
        .rollover = false,
        .head = 0u,
        .tail = 0u,
    };

    if ((argc > 1) && (argv != NULL))
    {
        for (int i = 1; i < argc; ++i)
        {
            int value = strol(argv[i], (char**)NULL, 10);
            switch (i)
            {
                case 1:
                    if (value > 0)
                    {
                        args.head = value;
                        args.tail = value;
                        args.rollover = true;
                    }
                    break;
                case 2:
                    if (value > 0)
                        args.tail = value;
                    break;
                case 3:
                    args.rollover = value != 0;
                    break;
                default:
                    ; // Do nothing.
            }
        }
    }
#endif

    return args;
}

static void printHeader(void)
{
    printf("headArg, tailArg, rolling, head, tail, pHead, pTail, size, firstSize, endSize, partA, partF, partB,\n");
}

static void printImuData(imuArgs_t const args, imuSaveResult_t const result)
{
    printf("%u, %u, %u, %u, %u, 0x%08x, 0x%08x, %u, %u, %u, %u, %u, %u,\n",
        args.head, args.tail, args.rollover,
        result.head, result.tail, result.head * 7, result.tail * 7, result.size, result.firstSize, result.endSize,
        result.blockSizes.a, result.blockSizes.f, result.blockSizes.b
        );
}


// === MAIN ====================================================================

int main(int argc, char* argv[])
{
    printf("\n%s(%d,[", __func__, argc);
    for (int i = 0; i < argc; ++i)
        printf("%s,", argv[i]);
    printf("])\n");

    if (argc > 1)
    {
        imuArgs_t args = processMovingAverageArgs(argc, argv);
        setDataPointers(args);
        debugData_t debugData;
        saveBufferToFlash(debugData);
        imuSaveResult_t result = getImuSaveResult();
        printHeader();
        printImuData(args, result);
    }
    else
    {
        printHeader();
        uint16_t bufferSize = getImuSaveBufferSize();
        for (uint16_t tail = 0; tail < bufferSize; ++tail)
        {
            for (uint16_t head = 0; head < bufferSize; ++head)
            {
                if (head > tail)
                    continue;
                imuArgs_t args =
                {
                    .rollover = tail >= head,
                    .head = head,
                    .tail = tail,
                };
                setDataPointers(args);
                debugData_t debugData;
                saveBufferToFlash(debugData);
                imuSaveResult_t result = getImuSaveResult();
                printImuData(args, result);
            }
        }

    }
    
}
