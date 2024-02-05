#include "meas_control.h"

#include "FreeRTOS.h"
#include "stm32h7xx_hal.h"

void ctrl_initInterrupts(void) {
    /* SW interrupt for end of measurement */
    HAL_NVIC_SetPriority(END_MEAS_INT_EXTI_IRQ, 0xFU, 0U);
    HAL_NVIC_EnableIRQ(END_MEAS_INT_EXTI_IRQ);
    /* SW interrupt for start of measurement */
    HAL_EXTI_EdgeConfig(START_MEAS_INT_EXTI_LINE, EXTI_RISING_EDGE);
}

void ctrl_interruptHandlerIPC_endMeas( void ){
    /* Signaling to task with notification*/
    BaseType_t xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR( app_endMeasSemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    HAL_EXTI_D1_ClearFlag(END_MEAS_INT_EXTI_LINE);
}

void ctrl_generateInterruptIPC_startMeas(void){
    generateIT_IPC(START_MEAS_INT_EXTI_LINE);
}

/** 
 * @brief Returns the corresponding latency value 
 * @param[in] m4clk M4 core clock in [Hz]
 * @note assumes that m4 clock is the same as the AXI bus freq
*/
static uint32_t getVOS0FlashLatency(uint32_t m4clk) {
    // todo assert v0 supply
    uint32_t latency;
    if (m4clk <= VOS0_0WS_MAX_AXI_CLK) { // Hz
        latency = FLASH_LATENCY_0;
    }
    else if (m4clk <= VOS0_1WS_MAX_AXI_CLK) {
        latency = FLASH_LATENCY_1;
    }
    else if (m4clk <= VOS0_2WS_MAX_AXI_CLK) {
        latency = FLASH_LATENCY_2;
    }
    else if (m4clk <= VOS0_3WS_MAX_AXI_CLK) {
        latency = FLASH_LATENCY_3;
    }
    else if (m4clk <= VOS0_4WS_MAX_AXI_CLK) {
        latency = FLASH_LATENCY_4;
    }
    else {
        assert(false);
    }
    return latency;
}

typedef enum {
    CLK_M7_ERR,
    CLK_M4_ERR,
    CLK_OK,
} ClkErr;

/**
 * @brief Sets the clk divider values based on the desired core frequencies
 * @param[in] sysclk sys_ck frequency (input of the system clock 
 *  generation block)
 * @param[in] m7clk desired M7 core frequency
 * @param[in] m4clk desired M4 core frequency
 * @param[out] d1cpre D1CPRE value, can be ignored with NULL
 * @param[out] hpre HPRE value, can be ignored with NULL
 * @returns CLK_M7_ERR or CLK_M4_ERR if the desired frequencies cannot be 
 *  met, CLK_OK otherwise
*/
static ClkErr getClkDivs(uint32_t sysclk, uint32_t m7clk, uint32_t m4clk,
                         uint32_t* d1cpre, uint32_t* hpre) {
    // clk divider values from stm32h7xx_hal_rcc.h
    if (sysclk % m7clk != 0 || m7clk == 0) return CLK_M7_ERR;
    uint32_t m7div = sysclk / m7clk;
    if (m7clk % m4clk != 0 || m4clk == 0) return CLK_M4_ERR;
    uint32_t m4div = m7clk / m4clk;

    uint32_t d1cpreRet, hpreRet;
    switch (m7div) {
        case 1U: d1cpreRet = RCC_SYSCLK_DIV1; break;
        case 2U: d1cpreRet = RCC_SYSCLK_DIV2; break;
        case 4U: d1cpreRet = RCC_SYSCLK_DIV4; break;
        case 8U: d1cpreRet = RCC_SYSCLK_DIV8; break;
        default: return CLK_M7_ERR;
    }
    switch (m4div) {
        case 1U: hpreRet = RCC_HCLK_DIV1; break;
        case 2U: hpreRet = RCC_HCLK_DIV2; break;
        case 4U: hpreRet = RCC_HCLK_DIV4; break;
        case 8U: hpreRet = RCC_HCLK_DIV8; break;
        default: return CLK_M4_ERR;
    }

    if (d1cpre != NULL) *d1cpre = d1cpreRet;
    if (hpre != NULL) *hpre = hpreRet;
    return CLK_OK;
}

bool ctrl_setClks(uint32_t m7clk, uint32_t m4clk, const char** msg) {
    uint32_t sysclk = HAL_RCC_GetSysClockFreq(); // returns the freq before the d1cpre presclaer
    uint32_t d1cpre, hpre;
    // new clk divider and latency values based on the desired clk freq
    ClkErr status = getClkDivs(sysclk, m7clk, m4clk, &d1cpre, &hpre);
    if (status != CLK_OK) {
        if (status == CLK_M7_ERR) {
            if (msg != NULL) *msg = "Invalid m7 clk frequency\r\n";
        }
        else if (status == CLK_M4_ERR) {
            if (msg != NULL) *msg = "Invalid m4 clk frequency\r\n";
        }
        else {
            assert(false); // missing error value
        }
        return false;
    }
    uint32_t flatency = getVOS0FlashLatency(m4clk);

    // updating the config accordingly
    RCC_ClkInitTypeDef config;
    uint32_t ignoreLatency;
    HAL_RCC_GetClockConfig(&config, &ignoreLatency); // todo alternatively don't use the getter, and set the source as well, then set sysclk and hclk as clocktype

    config.SYSCLKDivider = d1cpre;
    config.AHBCLKDivider = hpre;

    HAL_StatusTypeDef ret 
        = HAL_RCC_ClockConfig(&config, flatency);
    assert(ret == HAL_OK);
    return true;
}

/**
 * @brief Returns the clk after the D1CPRE prescaler (M7 core clk freq)
 * @note code taken from stm32h7xx_hal_rcc.c line 1490 as there is no function
 *  returning the freq of this stage of the clk tree
*/
static uint32_t getM7ClkFreq(void) {
    return HAL_RCC_GetSysClockFreq() >> (D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos] & 0x1FU);
}

void ctrl_getClks(uint32_t* m7clk, uint32_t* m4clk) {
    if (m7clk != NULL) *m7clk = getM7ClkFreq(); // freq after the d1cpre prescaler
    if (m4clk != NULL) * m4clk = HAL_RCC_GetHCLKFreq(); // freq after the HPRE prescaler
}
