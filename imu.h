#ifndef __IMU_H
#define __IMU_H

// === INCLUDES ================================================================

#include <stdint.h>


// === datastore.h =============================================================

typedef uint32_t debugData_t[16];


// === FUNCTION PROTOTYPES =====================================================

void saveBufferToFlash( debugData_t debugData );
void updateRingBufferPointers( uint16_t bytesWritten );
void newUpdateRingBufferPointers( uint16_t bytesWritten );
void setDataPointers(uint16_t head, uint16_t tail);


#endif
