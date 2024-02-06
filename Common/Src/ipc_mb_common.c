#include "ipc_mb_common.h"

// todo change the magic numbers
// todo add the sectoin to the name
#define MB_STORAGE_VARIABLES_DEF(mem) \
    /* Message buffers */ \
    MessageBufferHandle_t \
        mem##ControlMessageBuffers[DIRECTION_NUM] \
        __attribute__((section(".shared_"#mem))); \
    MessageBufferHandle_t \
        mem##DataMessageBuffers[DIRECTION_NUM] \
        __attribute__((section(".shared_"#mem))); \
    /* The variable used to hold the stream buffer structure.*/ \
    StaticStreamBuffer_t \
        mem##ControlStreamBufferStruct[DIRECTION_NUM] \
        __attribute__((section(".shared_"#mem))); \
    StaticStreamBuffer_t \
        mem##DataStreamBufferStruct[DIRECTION_NUM] \
        __attribute__((section(".shared_"#mem))); \
    /* Used to dimension the array used to hold the streams.*/ \
    /* Defines the memory that will hold the streams within the stream buffer.*/ \
    uint8_t  \
        mem##ControlStorageBuffer[DIRECTION_NUM][ mbaCONTROL_MESSAGE_BUFFER_SIZE ] \
        __attribute__((section(".shared_"#mem))); \
    uint8_t \
        mem##DataStorageBuffer[DIRECTION_NUM][ mbaTASK_MESSAGE_BUFFER_SIZE ] \
        __attribute__((section(".shared_"#mem))); 

// Definition of the variables
MB_STORAGE_VARIABLES_DEF(D1)
MB_STORAGE_VARIABLES_DEF(D2)
MB_STORAGE_VARIABLES_DEF(D3)

MessageBufferHandle_t* mb_gpCurrentControlMB = D1ControlMessageBuffers;
MessageBufferHandle_t* mb_gpCurrentDataMB = D1DataMessageBuffers;


void mb_generateInterruptIPC_messageBuffer(void* updatedMessageBuffer){
    MessageBufferHandle_t xUpdatedBuffer 
        = (MessageBufferHandle_t)updatedMessageBuffer;
    
    if( xUpdatedBuffer != mb_gpCurrentControlMB[CONTROL_SEND_IDX] )
    {
        /* Use xControlMessageBuffer to pass the handle of the message buffer
        written to by core 1 to the interrupt handler about to be generated in
        core 2. */
        xMessageBufferSend( mb_gpCurrentControlMB[CONTROL_SEND_IDX], &xUpdatedBuffer,
                            sizeof( xUpdatedBuffer ), mbaDONT_BLOCK );
        
        /* This is where the interrupt would be generated. */
        generateIT_IPC(MB_SEND_EXTI_LINE);
    }
}

void mb_interruptHandlerIPC_messageBuffer( void ) {
    MessageBufferHandle_t xUpdatedMessageBuffer;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* xControlMessageBuffer contains the handle of the message buffer that
    contains data. */
    if( xMessageBufferReceiveFromISR(
                mb_gpCurrentControlMB[CONTROL_RECV_IDX], &xUpdatedMessageBuffer,
                sizeof( xUpdatedMessageBuffer ), &xHigherPriorityTaskWoken)
            == sizeof( xUpdatedMessageBuffer ) ) {
        /* API function notifying any task waiting for the messagebuffer*/
        xMessageBufferSendCompletedFromISR(xUpdatedMessageBuffer,
                                           &xHigherPriorityTaskWoken );
    }
    /* Scheduling with normal FreeRTOS semantics */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void mb_setUsedMemory(params_mem mem) {
    switch (mem) {
    case MEM_D1:
        mb_gpCurrentControlMB = D1ControlMessageBuffers;
        mb_gpCurrentDataMB = D1DataMessageBuffers;
        break;
    case MEM_D2:
        mb_gpCurrentControlMB = D2ControlMessageBuffers;
        mb_gpCurrentDataMB = D2DataMessageBuffers;
        break;
    case MEM_D3:
        mb_gpCurrentControlMB = D3ControlMessageBuffers;
        mb_gpCurrentDataMB = D3DataMessageBuffers;
        break;
    default:
        assert(false);
    }
}
