#include "ipc_mb.h"

#define MB_CREATE_STATIC(mem, purpose, index) \
    mem##purpose##MessageBuffers[(index)] \
        = xMessageBufferCreateStatic( \
            mbaCONTROL_MESSAGE_BUFFER_SIZE, \
            &mem##purpose##StorageBuffer[(index)][0], \
            &mem##purpose##StreamBufferStruct[(index)]); \
    configASSERT(mem##purpose##MessageBuffers[(index)] != NULL);

void createIPCMessageBuffers(void) {
    MB_CREATE_STATIC(D1, Control, CONTROL_SEND_IDX)
    MB_CREATE_STATIC(D1, Control, CONTROL_RECV_IDX)
    MB_CREATE_STATIC(D2, Control, CONTROL_SEND_IDX)
    MB_CREATE_STATIC(D2, Control, CONTROL_RECV_IDX)
    MB_CREATE_STATIC(D3, Control, CONTROL_SEND_IDX)
    MB_CREATE_STATIC(D3, Control, CONTROL_RECV_IDX)
    
    MB_CREATE_STATIC(D1, Data, CONTROL_SEND_IDX)
    MB_CREATE_STATIC(D1, Data, CONTROL_RECV_IDX)
    MB_CREATE_STATIC(D2, Data, CONTROL_SEND_IDX)
    MB_CREATE_STATIC(D2, Data, CONTROL_RECV_IDX)
    MB_CREATE_STATIC(D3, Data, CONTROL_SEND_IDX)
    MB_CREATE_STATIC(D3, Data, CONTROL_RECV_IDX)

}

void initIPC_MessageBuffers(void) {
    /* AIEC Common configuration: make CPU1 and CPU2 SWI line0
    sensitive to rising edge : Configured only once */
    HAL_EXTI_EdgeConfig(MB1TO2_INT_EXTI_LINE, EXTI_RISING_EDGE);
    /* SW interrupt for message buffer */
    HAL_NVIC_SetPriority(MB2TO1_INT_EXTI_IRQ, 0xFU, 1U);
    HAL_NVIC_EnableIRQ(MB2TO1_INT_EXTI_IRQ);
}
