#include "MessageBufferAMP.h"

#if defined ( __ICCARM__ )
#pragma location = 0x38000000
MessageBufferHandle_t xControlMessageBuffer;
#pragma location = 0x38000004
MessageBufferHandle_t xDataMessageBuffers;
/* The variable used to hold the stream buffer structure.*/
#pragma location = 0x38000050
StaticStreamBuffer_t xStreamBufferStruct_ctrl;
#pragma location = 0x380000A0
StaticStreamBuffer_t xStreamBufferStruct;
/* Used to dimension the array used to hold the streams.*/
/* Defines the memory that will actually hold the streams within the stream buffer.*/
#pragma location = 0x38000100
static uint8_t ucStorageBuffer_ctrl[ mbaCONTROL_MESSAGE_BUFFER_SIZE ];
#pragma location = 0x38000200
static uint8_t ucStorageBuffer[ mbaTASK_MESSAGE_BUFFER_SIZE ];
#elif defined ( __CC_ARM )
MessageBufferHandle_t xControlMessageBuffer __attribute__((at(0x38000000)));
MessageBufferHandle_t xDataMessageBuffers __attribute__((at(0x38000004)));
/* The variable used to hold the stream buffer structure.*/
StaticStreamBuffer_t xStreamBufferStruct_ctrl __attribute__((at(0x38000050)));
StaticStreamBuffer_t xStreamBufferStruct __attribute__((at(0x380000A0)));
/* Used to dimension the array used to hold the streams.*/
/* Defines the memory that will actually hold the streams within the stream buffer.*/
static uint8_t ucStorageBuffer_ctrl[ mbaCONTROL_MESSAGE_BUFFER_SIZE ]__attribute__((at(0x38000100)));
static uint8_t ucStorageBuffer[ mbaTASK_MESSAGE_BUFFER_SIZE ]__attribute__((at(0x38000200)));
#elif defined ( __GNUC__ )
MessageBufferHandle_t xControlMessageBuffer[2] __attribute__((section(".RAM_D3_Z1")));
MessageBufferHandle_t xDataMessageBuffers[2] __attribute__ ((section (".RAM_D3_Z2")));
/* The variable used to hold the stream buffer structure.*/
StaticStreamBuffer_t xStreamBufferStruct[4] __attribute__ ((section (".RAM_D3_Z3")));
/* Used to dimension the array used to hold the streams.*/
/* Defines the memory that will actually hold the streams within the stream buffer.*/
uint8_t ucStorageBuffer_ctrl[2][ mbaCONTROL_MESSAGE_BUFFER_SIZE ] __attribute__ ((section (".RAM_D3_Z4")));
uint8_t ucStorageBuffer[2][ mbaTASK_MESSAGE_BUFFER_SIZE ] __attribute__ ((section (".RAM_D3_Z5")));


#endif