#ifndef IPC_MB_COMMON_H
#define IPC_MB_COMMON_H

#include "FreeRTOS.h"
#include "message_buffer.h"

#include "stm32h7xx_hal.h"

#include <stdint.h>

#include "hw_control_common.h"
#include "shared_param_types.h"
#include "mb_config.h"

#define MB2TO1_INT_EXTI_IRQ EXTI4_IRQn
#define MB2TO1_INT_EXTI_LINE EXTI_LINE4
#define MB2TO1_GPIO_PIN GPIO_PIN_4

#define MB1TO2_INT_EXTI_IRQ EXTI0_IRQn
#define MB1TO2_INT_EXTI_LINE EXTI_LINE0
#define MB1TO2_GPIO_PIN GPIO_PIN_0

#ifdef CORE_CM4
    #define CONTROL_RECV_IDX M7_SEND
    #define CONTROL_SEND_IDX M7_RECEIVE
    #define DATA_RECV_IDX M7_SEND
    #define DATA_SEND_IDX M7_RECEIVE

    #define MB_SEND_EXTI_LINE MB2TO1_INT_EXTI_LINE
#elif defined CORE_CM7
    #define CONTROL_RECV_IDX M7_RECEIVE
    #define CONTROL_SEND_IDX M7_SEND
    #define DATA_RECV_IDX M7_RECEIVE
    #define DATA_SEND_IDX M7_SEND

    #define MB_SEND_EXTI_LINE MB1TO2_INT_EXTI_LINE
#else
#error Neither core is defined
#endif

/* A block time of 0 simply means, don't block. */
#define mbaDONT_BLOCK 0

#define MB_STORAGE_VARIABLES_DECL(mem) \
    extern MessageBufferHandle_t \
        mem##ControlMessageBuffers[DIRECTION_NUM]; \
    extern MessageBufferHandle_t \
        mem##DataMessageBuffers[DIRECTION_NUM]; \
    extern StaticStreamBuffer_t \
        mem##ControlStreamBufferStruct[DIRECTION_NUM]; \
    extern StaticStreamBuffer_t \
        mem##DataStreamBufferStruct[DIRECTION_NUM]; \
    extern uint8_t \
        mem##ControlStorageBuffer[DIRECTION_NUM][mbaCONTROL_MESSAGE_BUFFER_SIZE]; \
    extern uint8_t \
        mem##DataStorageBuffer[DIRECTION_NUM][mbaTASK_MESSAGE_BUFFER_SIZE]; \

// Variables used by the IPC MBs
MB_STORAGE_VARIABLES_DECL(D1)
MB_STORAGE_VARIABLES_DECL(D2)
MB_STORAGE_VARIABLES_DECL(D3)

// Pointers to the currently used MBs
extern MessageBufferHandle_t* mb_gpCurrentControlMB;
extern MessageBufferHandle_t* mb_gpCurrentDataMB;

/** @brief Triggers interrupt used for IPC message buffer communication */
void mb_generateInterruptIPC_messageBuffer(void* updatedMessageBuffer);
/** @brief Message Buffer functionality, receiving message */
void mb_interruptHandlerIPC_messageBuffer( void );
/** @brief Sets the pointer to the Message Buffer in the given memory */
void mb_setUsedMemory(params_mem mem);

#endif /* IPC_MB_COMMON_H */
