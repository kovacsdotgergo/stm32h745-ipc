#ifndef MESSAGE_BUFFER_AMP_H
#define MESSAGE_BUFFER_AMP_H

#include "FreeRTOS.h"
#include "message_buffer.h"
#include "stdint.h"
#include "meas_control_common.h"

/* Enough for 4 8 byte strings, plus the additional 4 bytes per message
overhead of message buffers. */
#define mbaTASK_MESSAGE_BUFFER_SIZE ( 16384 )
#define mbaCONTROL_MESSAGE_BUFFER_SIZE ( 24 )

/* Maximum size for the messages */
#define MAX_DATA_SIZE (16376)

#define MB2TO1_INT_EXTI_IRQ EXTI4_IRQn
#define MB2TO1_INT_EXTI_LINE EXTI_LINE4
#define MB2TO1_GPIO_PIN GPIO_PIN_4

#define MB1TO2_IDX 0
#define MB2TO1_IDX 1

/* A block time of 0 simply means, don't block. */
#define mbaDONT_BLOCK 0

/* Message buffers*/
extern volatile MessageBufferHandle_t xControlMessageBuffer[2];
extern volatile MessageBufferHandle_t xDataMessageBuffers[2];

extern volatile StaticStreamBuffer_t xStreamBufferStruct[4];

extern volatile uint8_t ucStorageBuffer_ctrl[2][ mbaCONTROL_MESSAGE_BUFFER_SIZE ];
extern volatile uint8_t ucStorageBuffer[2][ mbaTASK_MESSAGE_BUFFER_SIZE ];

#endif /* MESSAGE_BUFFER_AMP_H */
