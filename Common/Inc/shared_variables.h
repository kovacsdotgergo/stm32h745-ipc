#ifndef MESSAGE_BUFFER_AMP_H
#define MESSAGE_BUFFER_AMP_H

#include "FreeRTOS.h"
#include "message_buffer.h"
#include "stdint.h"
#include "time_meas.h"

/* Enough for 4 8 byte strings, plus the additional 4 bytes per message
overhead of message buffers. */
#define mbaTASK_MESSAGE_BUFFER_SIZE ( 16384 )
#define mbaCONTROL_MESSAGE_BUFFER_SIZE ( 24 )

#define MB1TO2_IDX 0
#define MB2TO1_IDX 1

/* A block time of 0 simply means, don't block. */
#define mbaDONT_BLOCK 0

/* Shared variables used for storing the configurable measurement
    parameters */
/* TODO numemas and datasize not shared for now, remove */
extern volatile uint32_t shNumMeas; 
extern volatile uint32_t shDataSize; 
extern volatile measDirection shDirection; 

/* Shared variables for time measurement */
extern volatile uint32_t shStartTime;
extern volatile uint32_t shEndTime;

/* Message buffers*/
extern volatile MessageBufferHandle_t xControlMessageBuffer[2];
extern volatile MessageBufferHandle_t xDataMessageBuffers[2];

extern volatile StaticStreamBuffer_t xStreamBufferStruct[4];

extern volatile uint8_t ucStorageBuffer_ctrl[2][ mbaCONTROL_MESSAGE_BUFFER_SIZE ];
extern volatile uint8_t ucStorageBuffer[2][ mbaTASK_MESSAGE_BUFFER_SIZE ];

#endif /* MESSAGE_BUFFER_AMP_H */
