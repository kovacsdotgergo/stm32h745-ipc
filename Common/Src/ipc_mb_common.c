#include "ipc_mb_common.h"

#ifdef CORE_CM4
    #define CONTROL_RECV_IDX MB1TO2_IDX
    #define CONTROL_SEND_IDX MB2TO1_IDX

    #define MB_SEND_EXTI_LINE MB2TO1_INT_EXTI_LINE
#elif defined CORE_CM7
    #define CONTROL_RECV_IDX MB2TO1_IDX
    #define CONTROL_SEND_IDX MB1TO2_IDX

    #define MB_SEND_EXTI_LINE MB1TO2_INT_EXTI_LINE
#else
#error Neither core is defined
#endif

/* Message buffers */
volatile MessageBufferHandle_t xControlMessageBuffer[2] __attribute__((section(".shared"))); 
volatile MessageBufferHandle_t xDataMessageBuffers[2] __attribute__((section(".shared"))); 
/* The variable used to hold the stream buffer structure.*/
volatile StaticStreamBuffer_t xStreamBufferStruct[4] __attribute__((section(".shared"))); 
/* Used to dimension the array used to hold the streams.*/
/* Defines the memory that will actually hold the streams within the stream buffer.*/
volatile uint8_t ucStorageBuffer_ctrl[2][ mbaCONTROL_MESSAGE_BUFFER_SIZE ] __attribute__((section(".shared"))); 
volatile uint8_t ucStorageBuffer[2][ mbaTASK_MESSAGE_BUFFER_SIZE ] __attribute__((section(".shared"))); 


void generateInterruptIPC_messageBuffer(void* updatedMessageBuffer){
  MessageBufferHandle_t xUpdatedBuffer = ( MessageBufferHandle_t ) updatedMessageBuffer;
  
  if( xUpdatedBuffer != xControlMessageBuffer[CONTROL_SEND_IDX] )
  {
    /* Use xControlMessageBuffer to pass the handle of the message buffer
    written to by core 1 to the interrupt handler about to be generated in
    core 2. */
    xMessageBufferSend( xControlMessageBuffer[CONTROL_SEND_IDX], &xUpdatedBuffer,
                        sizeof( xUpdatedBuffer ), mbaDONT_BLOCK );
    
    /* This is where the interrupt would be generated. */
    generateIT_IPC(MB_SEND_EXTI_LINE);
  }
}

void interruptHandlerIPC_messageBuffer( void ) {
    MessageBufferHandle_t xUpdatedMessageBuffer;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* xControlMessageBuffer contains the handle of the message buffer that
    contains data. */
    if( xMessageBufferReceiveFromISR(
                xControlMessageBuffer[CONTROL_RECV_IDX], &xUpdatedMessageBuffer,
                sizeof( xUpdatedMessageBuffer ), &xHigherPriorityTaskWoken)
            == sizeof( xUpdatedMessageBuffer ) ) {
        /* API function notifying any task waiting for the messagebuffer*/
        xMessageBufferSendCompletedFromISR( xUpdatedMessageBuffer, &xHigherPriorityTaskWoken );
    }
    /* Scheduling with normal FreeRTOS semantics */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}