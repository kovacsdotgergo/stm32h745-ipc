#include "ipc_mb.h"

#define PURPOSE_TO_SIZE(purpose) PURPOSE_TO_SIZE_IMPL(purpose)
#define PURPOSE_TO_SIZE_IMPL(purpose) SIZE_IF_##purpose
#define SIZE_IF_Control mbaCONTROL_MESSAGE_BUFFER_SIZE
#define SIZE_IF_Data mbaTASK_MESSAGE_BUFFER_SIZE

#define MB_CREATE_STATIC(mem, purpose, index) \
    do { \
    mem##purpose##MessageBuffers[(index)] \
        = xMessageBufferCreateStatic( \
            PURPOSE_TO_SIZE(purpose), \
            &mem##purpose##StorageBuffer[(index)][0], \
            &mem##purpose##StreamBufferStruct[(index)]); \
    configASSERT(mem##purpose##MessageBuffers[(index)] != NULL); \
    } while(0)

void createIPCMessageBuffers(void) {
    MB_CREATE_STATIC(D1, Control, CONTROL_SEND_IDX);
    MB_CREATE_STATIC(D1, Control, CONTROL_RECV_IDX);
    MB_CREATE_STATIC(D2, Control, CONTROL_SEND_IDX);
    MB_CREATE_STATIC(D2, Control, CONTROL_RECV_IDX);
    MB_CREATE_STATIC(D3, Control, CONTROL_SEND_IDX);
    MB_CREATE_STATIC(D3, Control, CONTROL_RECV_IDX);
    
    MB_CREATE_STATIC(D1, Data, CONTROL_SEND_IDX);
    MB_CREATE_STATIC(D1, Data, CONTROL_RECV_IDX);
    MB_CREATE_STATIC(D2, Data, CONTROL_SEND_IDX);
    MB_CREATE_STATIC(D2, Data, CONTROL_RECV_IDX);
    MB_CREATE_STATIC(D3, Data, CONTROL_SEND_IDX);
    MB_CREATE_STATIC(D3, Data, CONTROL_RECV_IDX);

    mb_setUsedMemory(CURRENT_MEM_INIT);
}

void initIPC_MessageBuffers(void) {
    /* AIEC Common configuration: make CPU1 and CPU2 SWI line0
    sensitive to rising edge : Configured only once */
    HAL_EXTI_EdgeConfig(MB1TO2_INT_EXTI_LINE, EXTI_RISING_EDGE);
    /* SW interrupt for message buffer */
    HAL_NVIC_SetPriority(MB2TO1_INT_EXTI_IRQ, 14U, 0U);
    HAL_NVIC_EnableIRQ(MB2TO1_INT_EXTI_IRQ);

    createIPCMessageBuffers();
}
