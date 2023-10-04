#include "shared_variables.h"

/* Message buffers */
volatile MessageBufferHandle_t xControlMessageBuffer[2] __attribute__((section(".shared"))); 
volatile MessageBufferHandle_t xDataMessageBuffers[2] __attribute__((section(".shared"))); 
/* The variable used to hold the stream buffer structure.*/
volatile StaticStreamBuffer_t xStreamBufferStruct[4] __attribute__((section(".shared"))); 
/* Used to dimension the array used to hold the streams.*/
/* Defines the memory that will actually hold the streams within the stream buffer.*/
volatile uint8_t ucStorageBuffer_ctrl[2][ mbaCONTROL_MESSAGE_BUFFER_SIZE ] __attribute__((section(".shared"))); 
volatile uint8_t ucStorageBuffer[2][ mbaTASK_MESSAGE_BUFFER_SIZE ] __attribute__((section(".shared"))); 