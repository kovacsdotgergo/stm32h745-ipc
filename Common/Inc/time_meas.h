#include <stdint.h>

// #pragma location=0x30000500
// uint32_t startTime;
// #pragma location=0x30000504
// uint32_t endTime;

// #elif defined ( __CC_ARM )  /* MDK ARM Compiler */

// __attribute__((at(0x30000000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
// __attribute__((at(0x30000200))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

// #elif defined ( __GNUC__ ) /* GNU Compiler */
uint32_t startTime __attribute__((section(".RAM_D3_Z3")));
uint32_t endTime __attribute__((section(".RAM_D3_Z3")));

// #endif

// uint32_t startTime, endTime;
const uint32_t runtimeOffset = 24;