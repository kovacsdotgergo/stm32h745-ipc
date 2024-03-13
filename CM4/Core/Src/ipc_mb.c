#include "ipc_mb.h"

#define ASSERT_MB_INITIALIZED(mem, purpose, index) \
    do { assert(mem##purpose##MessageBuffers[(index)] != NULL); } while(0)

void initIPC_MessageBuffers(void) {
    /* Int config for message buffer*/
    HAL_EXTI_EdgeConfig(MB2TO1_INT_EXTI_LINE, EXTI_RISING_EDGE);

    /* SW interrupt for message buffer */
    HAL_NVIC_SetPriority(MB1TO2_INT_EXTI_IRQ, 14U, 0U);
    HAL_NVIC_EnableIRQ(MB1TO2_INT_EXTI_IRQ);
    
    /* m7 core initializes the message buffers */
    ASSERT_MB_INITIALIZED(D1, Control, CONTROL_SEND_IDX);
    ASSERT_MB_INITIALIZED(D1, Control, CONTROL_RECV_IDX);
    ASSERT_MB_INITIALIZED(D2, Control, CONTROL_SEND_IDX);
    ASSERT_MB_INITIALIZED(D2, Control, CONTROL_RECV_IDX);
    ASSERT_MB_INITIALIZED(D3, Control, CONTROL_SEND_IDX);
    ASSERT_MB_INITIALIZED(D3, Control, CONTROL_RECV_IDX);
    
    ASSERT_MB_INITIALIZED(D1, Data, CONTROL_SEND_IDX);
    ASSERT_MB_INITIALIZED(D1, Data, CONTROL_RECV_IDX);
    ASSERT_MB_INITIALIZED(D2, Data, CONTROL_SEND_IDX);
    ASSERT_MB_INITIALIZED(D2, Data, CONTROL_RECV_IDX);
    ASSERT_MB_INITIALIZED(D3, Data, CONTROL_SEND_IDX);
    ASSERT_MB_INITIALIZED(D3, Data, CONTROL_RECV_IDX);

    mb_setUsedMemory(CURRENT_MEM_INIT);
}
