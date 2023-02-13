// === INCLUDES ================================================================

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exception.h"
#include "imu.h"


// === TYPE DEFINES ============================================================




// === PRIVATE FUNCTIONS =======================================================

static imuArgs_t processMovingAverageArgs(int argc, char* argv[])
{
#if false
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
            int value = strtol(argv[i], (char**)NULL, 10);
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

static void printImuArgs(imuArgs_t const args)
{
    printf("%u, %u, %u, ",
        args.head, args.tail, args.rollover
        );
}

static void printImuSaveResult( imuSaveResult_t const result)
{
    printf("%u, %u, 0x%08x, 0x%08x, %u, %u, %u, %u, %u, %u, ",
        result.head, result.tail, result.head * 7, result.tail * 7, result.size, result.firstSize, result.endSize,
        result.blockSizes.a, result.blockSizes.f, result.blockSizes.b
        );
}

static void testImu(int argc, char* argv[])
{
    if (argc > 1)
    {
        imuArgs_t args = processMovingAverageArgs(argc, argv);
        printImuArgs(args);
        setDataPointers(args);
        debugData_t debugData;
        saveBufferToFlash(debugData);
        imuSaveResult_t result = getImuSaveResult();
        printImuSaveResult(result);
        printf("\n");
    }
    else
    {
        printf("\n%s(%d,[", __func__, argc);
        for (int i = 0; i < argc; ++i)
            printf("%s,", argv[i]);
        printf("])\n");
        
        printHeader();
        //uint16_t bufferSize = getImuSaveBufferSize();
        uint16_t bufferSize = 10u;
        for (uint16_t tail = 0; tail < bufferSize; ++tail)
        {
            for (uint16_t head = 0; head < bufferSize; ++head)
            {
                imuArgs_t args =
                {
                    .rollover = tail >= head,
                    .head = head,
                    .tail = tail,
                };
                printImuArgs(args);
                setDataPointers(args);
                debugData_t debugData;
                saveBufferToFlash(debugData);
                imuSaveResult_t result = getImuSaveResult();
                printImuSaveResult(result);
                printf("\n");
            }
        }
    }
}

static void testException(void)
{
    exceptionTest1();
    exceptionTest2();
}


// === MAIN ====================================================================

int main(int argc, char* argv[])
{
    testImu(argc, argv);
}
