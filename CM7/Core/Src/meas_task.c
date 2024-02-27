#include "meas_task.h"

static UART_HandleTypeDef* g_measUartHandle;
static bool g_startMeas = false;

static void clearStartMeas(void) {
    g_startMeas = false;
}

static void setStartMeas(void) {
    g_startMeas = true;
}

static bool getStartMeas(void) {
    return g_startMeas;
}

/**
 * @brief Prints the given number of characters from the string with 
 *  blocking wait
*/
static void printNUart(const char* msg, size_t len) {
    assert(msg != NULL);
    if (0 < len) {
        // transmit with max delay waits in a loop and only returns ok
        HAL_StatusTypeDef status 
            = HAL_UART_Transmit(g_measUartHandle, 
                                (const uint8_t*)msg, len,
                                HAL_MAX_DELAY);
        assert(status == HAL_OK);
    }
}

/**
 * @brief Print a null-terminated string with blocking wait
*/
static void printUart(const char* msg) {
    printNUart(msg, strlen(msg));
}

/**
 * @brief Read a character with blocking wait
*/
static char getcharUart(void) {
    uint8_t uartInput;
    HAL_StatusTypeDef receiveSuccess
        = HAL_UART_Receive(g_measUartHandle,
                           &uartInput, sizeof(uartInput),
                           HAL_MAX_DELAY);
    assert(receiveSuccess == HAL_OK); // max delay can only return with ok
    return (char)uartInput;
}

/** 
 * @brief Processes the UART control messages and returns when the
 *  measurement can be started
 * @param[inout] controlFuns parameters of the measurement to be performed
*/
static void processUartControl(const uart_controlIf* controlFuns) {
    clearStartMeas();

    uart_parseStatus uartControlStatus;
    do {
        uart_BufferStatus bufferStatus;
        uart_LineBuffer lineBuffer;
        uart_initLineBuffer(&lineBuffer);

        printNUart(uart_getPrompt(), PROMPT_STR_LEN);
        do {
            // Blocking wait for UART, processing by single characters
            uint8_t uartInput = getcharUart();
            
            const char* echo = NULL;
            bufferStatus = uart_addCharToBuffer(uartInput, &lineBuffer, &echo);
            
            if (echo != NULL) {
                printUart(echo);
            }

            if (bufferStatus == BUFFER_OVERFLOW) {
                printUart("\r\nInput buffer overflow\r\n");
                uart_clearLineBuffer(&lineBuffer);
            }
        } while (bufferStatus != BUFFER_DONE);
            const char* msg = NULL;
            uartControlStatus = uart_parseBuffer(&lineBuffer, controlFuns, &msg);
            uart_clearLineBuffer(&lineBuffer);

            switch (uartControlStatus)
            {
            case PARSE_COMMAND_ERR:
                printUart("Command parsing error\r\n");
                break;
            case PARSE_ARG_NUM_ERR:
                printUart("Argument number error while parsing\r\n");
                break;
            case PARSE_ARG_VAL_ERR:
                printUart("Argument value error while parsing\r\n");
                break;
            case PARSE_OK:
                break;
            default:
                assert(false); // missed enum value
                break;
            }

            if (msg != NULL) {
                printUart(msg);
            }

    } while (!getStartMeas());
}

void meastask_core1MeasurementTask( void *pvParameters ){
    g_measUartHandle = (UART_HandleTypeDef*)pvParameters;

    uart_controlIf controlFuns = {
        .setClks = ctrl_setClks, // functions setting the parameters
        .getClks = ctrl_getClks,
        .setDataSize = ctrl_setDataSize,
        .getDataSize = ctrl_getDataSize,
        .setRepeat = ctrl_setRepeat,
        .getRepeat = ctrl_getRepeat,
        .setDirection = ctrl_setDirection,
        .getDirection = ctrl_getDirection,
        .setMem = ctrl_setMemory,
        .getMem = ctrl_getMemory,
        .setEnabledCache = ctrl_setEnabledCache,
        .getEnabledCache = ctrl_getEnabledCache,
        .setStartMeas = setStartMeas, // function setting static global
    };

    SemaphoreHandle_t endMeasSemaphore = xSemaphoreCreateBinary();
    ctrl_setEndMeasSemaphore(endMeasSemaphore);
    
    printNUart(uart_getInitStr(), INIT_STR_LEN);
    
    while(1)
    {
        // Processing input until start of meas signaled
        processUartControl(&controlFuns);

        // Print header
        char uartOutBuf[128];
        size_t uartOutBufLen = sizeof(uartOutBuf) / sizeof(uartOutBuf[0]);
        csv_assembleResultHeader(uartOutBuf, uartOutBufLen);
        printUart(uartOutBuf);

        uint32_t clkM7, clkM4;
        controlFuns.getClks(&clkM7, &clkM4);
        csv_meas_config conf = {
            .timer = clkM4,
            .repeat = controlFuns.getRepeat(),
            .datasize = controlFuns.getDataSize(),
            .clkM7 = clkM7,
            .clkM4 = clkM4,
            .direction = params_measDirectionToStr(controlFuns.getDirection()),
            .mem = params_memToStr(controlFuns.getMem()),
            .cache = params_cacheToStr(controlFuns.getEnabledCache()),
        };

        for(uint32_t i = 0; i < controlFuns.getRepeat(); ++i){
            /* Signaling to the other core*/
            ctrl_generateInterruptIPC_startMeas(); // todo signalPartner func
            switch (controlFuns.getDirection())
            {
            case M7_SEND:
                meastask_measureCore1Sending(controlFuns.getDataSize(),
                                             endMeasSemaphore);
                break;
            case M7_RECEIVE:
                meastask_measureCore1Recieving();
                break;
            default:
                assert(false); // enum value error
                break;
            }

            // Print result of measurement
            uint32_t localOffset = time_measureOffset();
            csv_meas_result res = {
                .time = time_getRuntime(localOffset),
                .m7offset = localOffset,
                .m4offset = time_getSharedOffset(),
            };
            csv_assembleResultLine(uartOutBuf, uartOutBufLen, i, res, conf);
            printUart(uartOutBuf);
        }

        // Print footer
        csv_assembleResultFooter(uartOutBuf, uartOutBufLen);
        printUart(uartOutBuf);
    }
}

void meastask_measureCore1Sending(uint32_t dataSize, 
                                  SemaphoreHandle_t endMeasSemaphore){
    /* Assembling the message*/
    static char sendBuffer[MB_MAX_DATA_SIZE];
    static uint8_t nextValue = 0;
    for(uint32_t j = 0; j < dataSize; ++j){
        sendBuffer[j] = nextValue;
    }
    sprintf((char*)sendBuffer, "%lu", dataSize);
    // Delay to wait for other core to block on message
    vTaskDelay(1/portTICK_PERIOD_MS);
    /* Start of measurement and sending the data */
    time_setCheckpoint(TIME_START);
    xMessageBufferSend(mb_gpCurrentDataMB[DATA_SEND_IDX], 
                       (void*)sendBuffer,
                       dataSize,
                       mbaDONT_BLOCK );
    time_setCheckpoint(TIME_AFTER_SEND);
    ++nextValue;
    /* Waiting for the signal from the other core */
    xSemaphoreTake(endMeasSemaphore, portMAX_DELAY);
}

void meastask_measureCore1Recieving(void){
    static uint8_t nextValue = 0;
    uint32_t recievedBytes, sizeFromMessage;
    static uint8_t recieveBuffer[MB_MAX_DATA_SIZE];

    time_setCheckpoint(TIME_BEGIN_BLOCK);
    recievedBytes = xMessageBufferReceive(mb_gpCurrentDataMB[DATA_RECV_IDX],
                                          recieveBuffer,
                                          sizeof(recieveBuffer),
                                          portMAX_DELAY);
    time_setCheckpoint(TIME_END);

    /* Error checking, size and last element */
    sscanf((char*)recieveBuffer, "%lu", &sizeFromMessage);
    if(recievedBytes != sizeFromMessage ||
            (sizeFromMessage > 2 && recieveBuffer[recievedBytes - 1] != nextValue)){
        assert(false); // communication error
    }

    memset(recieveBuffer, 0x00, recievedBytes);
    ++nextValue;
}
