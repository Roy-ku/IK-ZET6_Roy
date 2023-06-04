/* 日誌輸出 */
#ifndef __BSP_LOG_H
#define __BSP_LOG_H

#include "stdio.h"
#include "string.h"

#define bsp_INFO_ON 1
#define bsp_ERROR_ON 1
#define bsp_DEBUG_ON 1

/* 取出檔名 */
#define filename(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x

#define bsp_Log_Info(fmt, ...)                                                                         \
    do                                                                                                 \
    {                                                                                                  \
        if (bsp_INFO_ON)                                                                               \
            bsp_Log_print("[INFO] [%s @%d] " fmt "\r\n", filename(__FILE__), __LINE__, ##__VA_ARGS__); \
    } while (0)

#define bsp_Log_Error(fmt, ...)                                                                         \
    do                                                                                                  \
    {                                                                                                   \
        if (bsp_ERROR_ON)                                                                               \
            bsp_Log_print("[ERROR] [%s @%d] " fmt "\r\n", filename(__FILE__), __LINE__, ##__VA_ARGS__); \
    } while (0)

#define bsp_Log_Debug(fmt, ...)                                                                         \
    do                                                                                                  \
    {                                                                                                   \
        if (bsp_DEBUG_ON)                                                                               \
            bsp_Log_print("[DEBUG] [%s @%d] " fmt "\r\n", filename(__FILE__), __LINE__, ##__VA_ARGS__); \
    } while (0)

#endif /* __BSP_LOG_H */
