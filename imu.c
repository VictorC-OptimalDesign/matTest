// === INCLUDES ================================================================

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imu.h"


// === DEFINES =================================================================

#define NRF_LOG_INFO(f, ...)

typedef uint32_t nrfx_err_t;


// === PRIVATE FUNCTIONS =======================================================

#if false
static void NRF_LOG_INFO(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    printf(format, args);
    printf("\n");
    va_end(args);
}
#endif

// === clock.c =================================================================

uint32_t ulGetClockSecondTick( void) { return 0u; }


// === datastore.h =============================================================

#define FLASH_ERASE_BLOCK_SIZE (4*1024) // this is the smallest erase size

typedef enum {
    MODE_NA = -1,
    MODE_LEARNING = 3,
    MODE_TRAINING = 0,
    MODE_HUNTING = 2,
    MODE_TOURNAMENT = 1,
    MODE_LEVELING = 4
} deviceMode_t;



// === datastore.c =============================================================

#define START_ADDR_OF_META (FLASH_ERASE_BLOCK_SIZE) // 1st block is reserved/used for testing
#define SIZE_OF_META ( 256 ) // This is the flash page size
#define NUMBER_OF_META_PER_BLOCK (FLASH_ERASE_BLOCK_SIZE / SIZE_OF_META)
#define MAX_NUMBER_OF_META (NUMBER_OF_META_PER_BLOCK*2000*1024) // allocating 2000K for META STORAGE
#define START_ADDR_OF_SENSOR ((MAX_NUMBER_OF_META /16) + START_ADDR_OF_META)
#define END_OF_FLASH (64*1024*1024) // 512Meg


/*******************************************************************************

    PURPOSE: returns the current shot data pointer

    INPUTS: none

    OUTPUTS: shot count

    RETURN CODE: none

*******************************************************************************/
static __attribute__ ((aligned (4))) uint32_t ramFlashShotDataPointer = START_ADDR_OF_SENSOR;
uint32_t getFlashShotDataPointer ( void ) {
#if false
    static bool inited = false;
    if (inited == false) {
        flash_record_read( FDS_FILE_MATHEWS, FDS_RECORD_SHOT_DATA_POINTER,
            &ramFlashShotDataPointer, sizeof(ramFlashShotDataPointer), &ramFlashShotDataPointer);
        inited = true;
    }
#endif
    NRF_LOG_INFO("get flash raw shot data pointer = %08X", ramFlashShotDataPointer);
    return(ramFlashShotDataPointer);
}


uint32_t getShotCount ( void ) { return 0u; }
void setShotCount( uint32_t shot ) { ; }
uint32_t getDisplayShot ( void ) { return 0u; }
void setDisplayShot( uint32_t shot ) { ; }
void setShotMetaData( uint32_t shotCount, uint32_t startTime, uint32_t stopTime,
                      uint8_t mode, uint32_t serialNumber, debugData_t debugData,
                      uint32_t dataSize, uint32_t dataOffset) { ; }
void setFlashShotDataPointer( uint32_t value ) { ; }
deviceMode_t getMode ( void ) { return MODE_LEARNING; }
uint32_t getBowSerial( void ) { return 0u; }


// === qspi_flash.c ============================================================

uint32_t flash_init(void)
{
    return 0u;
}


void flash_off(void)
{

}


nrfx_err_t qflash_erase_blocking(uint32_t addr)
{
    return 0u;
}


nrfx_err_t qflash_write_blocking(void *buffer, size_t len, uint32_t addr)
{
    return 0u;
}



// === imu.c ===================================================================

#define FIFO_WATERMARK   510
#define FIFO_MAX_SIZE   512

#define MAX_RAW_SHOT_BUFFER (FIFO_WATERMARK*6)

uint8_t __attribute__ ((aligned (4))) rawShotBuffer[MAX_RAW_SHOT_BUFFER][7];
uint16_t rawShotBufferHeadPoint = 0;
uint16_t rawShotBufferTailPoint = 0;
bool     rawShotBufferEmptyFlag = true;
bool rawShotBufferRollingFlag = false;

uint32_t startTime = 0;

static imuArgs_t g_args =
{
    .rollover = false,
    .head = 0u,
    .tail = 0u
};

static imuSaveResult_t g_result =
{
    .head = 0u,
    .tail = 0u,
    .size = 0u,
    .firstSize = 0u,
    .endSize = 0u,
    .blockSizes =
    {
        .a = 0u,
        .f = 0u,
        .b = 0u
    }
};


void setBufferEmpty( void )
{
    rawShotBufferHeadPoint = 0;
    rawShotBufferTailPoint = 0;
    rawShotBufferEmptyFlag = true;
    rawShotBufferRollingFlag = false;
}


uint16_t bufferUsed( uint16_t head, uint16_t tail) {
    uint16_t size = sizeof(rawShotBuffer);

    if ((head == tail) && (rawShotBufferEmptyFlag == true)) {
        size = 0;
    } else if (head > tail) {
        size = (head - tail)*7;
    } else {
        size -= (tail - head) * 7;
    }
    return (size);
}


/*******************************************************************************

    PURPOSE: save the shot butter to flash

    INPUTS: none

    OUTPUTS: none

    RETURN CODE:

    NOTE: we are treating the rawShotBuffer as (int8_t *)

    TODO: Roll over of flash storage pointer

*******************************************************************************/

#define SINGLE_FLASH_BLOCK_SIZE 256
#define DOUBLE_FLASH_BLOCK_SIZE (SINGLE_FLASH_BLOCK_SIZE * 2)
void saveBufferToFlash( debugData_t debugData ) {

    uint8_t __attribute__ ((aligned (4))) blockBuffer[DOUBLE_FLASH_BLOCK_SIZE];
    uint16_t partF = 0;
    uint16_t tempBufferSize = SINGLE_FLASH_BLOCK_SIZE;

    // drop data to make 32bit aligned
    while (((uint32_t) &rawShotBuffer[rawShotBufferTailPoint][0] % 4) != 0) {
        NRF_LOG_INFO("dump 7 bytes to 32bit align tail was = %08X", &rawShotBuffer[rawShotBufferTailPoint][0]);
        rawShotBufferTailPoint += 1;
        partF += 7;
    }
    NRF_LOG_INFO("tail now = %08X", &rawShotBuffer[rawShotBufferTailPoint][0]);

    uint16_t size = bufferUsed( rawShotBufferHeadPoint, rawShotBufferTailPoint);
    NRF_LOG_INFO("Save Buffer head=%04X tail=%04X size=%04X",
        rawShotBufferHeadPoint, rawShotBufferTailPoint, size);

    // shift "5 bit imu tag" down 3 bits for mobile
    for (uint16_t i=0;i<MAX_RAW_SHOT_BUFFER;i++) {
        rawShotBuffer[i][0] = rawShotBuffer[i][0] >>3;
    }

    flash_init();
    uint32_t startFlashBufferPointer = getFlashShotDataPointer();

    // erase needed flash space
    for (uint32_t block = 0; block < size; block += 4 * 1024) {
        qflash_erase_blocking(startFlashBufferPointer+block);
        NRF_LOG_INFO("Flash erase addr=%08X", startFlashBufferPointer+ block);
    }

    // write
    if ((rawShotBufferTailPoint < rawShotBufferHeadPoint)||(rawShotBufferTailPoint == 0)) {
        qflash_write_blocking(&rawShotBuffer[rawShotBufferTailPoint], size, startFlashBufferPointer);
        NRF_LOG_INFO("Flash write addr=%08X len=%04X", startFlashBufferPointer, size);
    } else {

        uint16_t firstSize = size - (rawShotBufferTailPoint*7);
        uint16_t partA = firstSize % SINGLE_FLASH_BLOCK_SIZE;
        uint16_t partB = 0;
        uint16_t endSize = size - firstSize;
        if (partA != 0) {
            if ( SINGLE_FLASH_BLOCK_SIZE < (partA + partF)) {
                tempBufferSize = DOUBLE_FLASH_BLOCK_SIZE;
                NRF_LOG_INFO("double temp buffer!");
            }
            partB = tempBufferSize - partA - partF;
            NRF_LOG_INFO("Make partB %d", partB);
            firstSize = firstSize - partA;
            endSize -= partB;
            memcpy( blockBuffer, ((uint8_t *) &rawShotBuffer[rawShotBufferTailPoint])+firstSize, partA );
            memset( &blockBuffer[partA], 0xff, partF);
            memcpy( &blockBuffer[partA+partF], rawShotBuffer, partB);
        }
        NRF_LOG_INFO("split Flash 1 write faddr=%08X baddr=%08X len=%04X",
            startFlashBufferPointer, &rawShotBuffer[rawShotBufferTailPoint], firstSize);
        qflash_write_blocking(&rawShotBuffer[rawShotBufferTailPoint], firstSize, startFlashBufferPointer);
        if (partA != 0) {
            NRF_LOG_INFO("split Flash 2 write faddr=%08X baddrA=%08X lenA=%04X baddrB=%08X lenB=%04X",
                startFlashBufferPointer+firstSize, ((int8_t *) &rawShotBuffer[rawShotBufferTailPoint])+firstSize, partA, rawShotBuffer, partB);
            qflash_write_blocking(blockBuffer, tempBufferSize, startFlashBufferPointer+firstSize);
        }
        NRF_LOG_INFO("split Flash 3 write faddr=%08X baddr=%08X len=%04X",
            startFlashBufferPointer+firstSize+256, rawShotBuffer + partB, endSize);
        qflash_write_blocking(((uint8_t *) rawShotBuffer)+partB, endSize, startFlashBufferPointer+firstSize+256);
        g_result.firstSize = firstSize;
        g_result.endSize = endSize;
        g_result.blockSizes.a = partA;
        g_result.blockSizes.f = partF;
        g_result.blockSizes.b = partB;
    }
    flash_off();
    g_result.head = rawShotBufferHeadPoint;
    g_result.tail = rawShotBufferTailPoint;
    g_result.size = size;

    // update shot count, display count, and meta data
    uint32_t shotCount = getShotCount()+1;
    setShotCount(shotCount);
    setDisplayShot(getDisplayShot()+1);

    setShotMetaData( shotCount, startTime, ulGetClockSecondTick(),
                  getMode(), getBowSerial(), debugData,
                  size, startFlashBufferPointer);

    // adjust pointer to next flash block border
    startFlashBufferPointer = startFlashBufferPointer + size +
        (FLASH_ERASE_BLOCK_SIZE - ((startFlashBufferPointer + size) % FLASH_ERASE_BLOCK_SIZE));
    setFlashShotDataPointer(startFlashBufferPointer);

    setBufferEmpty();
}


void updateRingBufferPointers( uint16_t bytesWritten ) {

    if ((rawShotBufferRollingFlag == true)&&(rawShotBufferEmptyFlag != true)) {
        rawShotBufferHeadPoint+= bytesWritten;
        rawShotBufferTailPoint = rawShotBufferHeadPoint; // buffer full rotating....
    } else {
        rawShotBufferHeadPoint+= bytesWritten;
        rawShotBufferEmptyFlag = false; // buffer filling
        if (rawShotBufferHeadPoint >= MAX_RAW_SHOT_BUFFER) { // switched to rolling
            rawShotBufferRollingFlag = true;
            rawShotBufferTailPoint = rawShotBufferHeadPoint; // buffer full rotating....
        }
    }

    if (rawShotBufferHeadPoint >= MAX_RAW_SHOT_BUFFER) {
        rawShotBufferHeadPoint = rawShotBufferHeadPoint%MAX_RAW_SHOT_BUFFER;
        rawShotBufferRollingFlag = true;
    }

    if (rawShotBufferTailPoint >= MAX_RAW_SHOT_BUFFER) {
        rawShotBufferTailPoint = rawShotBufferTailPoint%MAX_RAW_SHOT_BUFFER;
        rawShotBufferRollingFlag = true;
    }
}

void newUpdateRingBufferPointers( uint16_t bytesWritten ) {
    rawShotBufferEmptyFlag = false;
    rawShotBufferHeadPoint += bytesWritten;
    if (rawShotBufferHeadPoint >= MAX_RAW_SHOT_BUFFER)
    {
        rawShotBufferRollingFlag = true;
        rawShotBufferHeadPoint = rawShotBufferHeadPoint % MAX_RAW_SHOT_BUFFER;
    }

    if (rawShotBufferRollingFlag)
        rawShotBufferTailPoint = rawShotBufferHeadPoint;
}


void setDataPointers(imuArgs_t const args)
{
    g_args = args;
    g_args.rollover = args.tail >= args.head;

    rawShotBufferRollingFlag = g_args.rollover;
    rawShotBufferHeadPoint = g_args.head;
    rawShotBufferTailPoint = g_args.tail;
    rawShotBufferEmptyFlag = false;

    g_result.head = g_args.head;
    g_result.tail = g_args.tail;
    g_result.size = 0u;
    g_result.firstSize = 0u;
    g_result.endSize = 0u;
    g_result.blockSizes.a = 0u;
    g_result.blockSizes.f = 0u;
    g_result.blockSizes.b = 0u;
}


imuSaveResult_t getImuSaveResult(void)
{
    return g_result;
}


uint16_t getImuSaveBufferSize(void)
{
    return sizeof(rawShotBuffer);
}


uint16_t getImuQueueSize(void)
{
    return MAX_RAW_SHOT_BUFFER;
}