#ifndef IPC_MB_H
#define IPC_MB_H

#include "FreeRTOS.h"
#include "task.h"

#include "ipc_mb_common.h"
#include "error_handler.h"

void initIPC_MessageBuffers(void);
void createIPCMessageBuffers(void);

#endif