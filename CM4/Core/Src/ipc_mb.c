#include "ipc_mb.h"

void initIPC_MessageBuffers(void) {
    /* Timer for time measurement */
    htim5.Instance = TIM5; // IMPORTANT to be able to read the timer! todo move somewhere else

    /* Int config for message buffer*/
    HAL_EXTI_EdgeConfig(MB2TO1_INT_EXTI_LINE, EXTI_RISING_EDGE);

    /* SW interrupt for message buffer */
    HAL_NVIC_SetPriority(MB1TO2_INT_EXTI_IRQ, 0xFU, 0U);
    HAL_NVIC_EnableIRQ(MB1TO2_INT_EXTI_IRQ);
    
    /* m7 core initializes the message buffers */
    assert(mb_gpCurrentDataMB[DATA_SEND_IDX] != NULL
           && mb_gpCurrentDataMB[DATA_RECV_IDX] != NULL
           && mb_gpCurrentControlMB[CONTROL_SEND_IDX] != NULL
           && mb_gpCurrentControlMB[CONTROL_RECV_IDX] != NULL);
}
