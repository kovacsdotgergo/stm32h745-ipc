#ifndef MB_CONFIG_H
#define MB_CONFIG_H
/** @file Configurations parameters for the message buffers, solved circular dependency between ipc_mb_common and meas_control_common */

#define mbaTASK_MESSAGE_BUFFER_SIZE (16384)
#define mbaCONTROL_MESSAGE_BUFFER_SIZE (24)

/* Maximum size for the messages, somehow -4 wasn't enough */
#define MB_MAX_DATA_SIZE (mbaTASK_MESSAGE_BUFFER_SIZE - 8)

#endif // MB_CONFIG_H