#ifndef __IMU_H
#define __IMU_H

// === INCLUDES ================================================================

#include <stdbool.h>
#include <stdint.h>


// === datastore.h =============================================================

typedef uint32_t debugData_t[16];

typedef struct imuArgs_t
{
    bool valid;
    bool rollover;
    uint16_t head;
    uint16_t tail;
} imuArgs_t;

typedef struct imuBlockSizes_t
{
    uint16_t a;
    uint16_t f;
    uint16_t b;
} imuBlockSizes_t;

typedef struct imuSaveResult_t
{
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint16_t firstSize;
    uint16_t endSize;
    imuBlockSizes_t blockSizes;
} imuSaveResult_t;


// === FUNCTION PROTOTYPES =====================================================

void saveBufferToFlash( debugData_t debugData );
void updateRingBufferPointers( uint16_t bytesWritten );
void newUpdateRingBufferPointers( uint16_t bytesWritten );
void setDataPointers(imuArgs_t const args);
imuSaveResult_t getImuSaveResult(void);
uint16_t getImuSaveBufferSize(void);
uint16_t getImuQueueSize(void);


#endif
