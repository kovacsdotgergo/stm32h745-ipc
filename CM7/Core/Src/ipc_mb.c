#include "ipc_mb.h"

void createIPCMessageBuffers(void) {
    /* MBs used for m7->m4 communication */
    /* Create control message buffer */
    xControlMessageBuffer[MB1TO2_IDX] = xMessageBufferCreateStatic(
        mbaCONTROL_MESSAGE_BUFFER_SIZE, ucStorageBuffer_ctrl[MB1TO2_IDX], 
        &xStreamBufferStruct[MB1TO2_IDX*2]);  
    /* Create data message buffer */
    xDataMessageBuffers[MB1TO2_IDX] = xMessageBufferCreateStatic(
        mbaTASK_MESSAGE_BUFFER_SIZE, &ucStorageBuffer[MB1TO2_IDX][0],
        &xStreamBufferStruct[MB1TO2_IDX*2 + 1]);
    configASSERT( xDataMessageBuffers[MB1TO2_IDX] );
    configASSERT( xControlMessageBuffer[MB1TO2_IDX] );
    
    /* MBs used for m4->m7 communication */
    xControlMessageBuffer[MB2TO1_IDX] = xMessageBufferCreateStatic(
        mbaCONTROL_MESSAGE_BUFFER_SIZE, ucStorageBuffer_ctrl[MB2TO1_IDX], 
        &xStreamBufferStruct[MB2TO1_IDX*2]);  
    /* Create data message buffer */
    xDataMessageBuffers[MB2TO1_IDX] = xMessageBufferCreateStatic(
        mbaTASK_MESSAGE_BUFFER_SIZE, &ucStorageBuffer[MB2TO1_IDX][0],
        &xStreamBufferStruct[MB2TO1_IDX*2 + 1]);
    configASSERT( xDataMessageBuffers[MB2TO1_IDX] );
    configASSERT( xControlMessageBuffer[MB2TO1_IDX] );
}

void initIPC_MessageBuffers(void) {
    /* AIEC Common configuration: make CPU1 and CPU2 SWI line0
    sensitive to rising edge : Configured only once */
    HAL_EXTI_EdgeConfig(MB1TO2_INT_EXTI_LINE, EXTI_RISING_EDGE);
    /* SW interrupt for message buffer */
    HAL_NVIC_SetPriority(MB2TO1_INT_EXTI_IRQ, 0xFU, 1U);
    HAL_NVIC_EnableIRQ(MB2TO1_INT_EXTI_IRQ);
}
