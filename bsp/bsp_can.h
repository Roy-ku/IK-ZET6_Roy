#ifndef __BSP_CAN_H_
#define __BSP_CAN_H_
#include "bsp.h"
#ifdef HAL_I2C_MODULE_ENABLED

void bsp_CAN_Init(void);
void bsp_CAN_Filter_Set(void);
bsp_Status bsp_CAN_SendPacket(uint8_t *_DataBuf, uint8_t _Len);
void bsp_CAN_Analyze(void);

extern CAN_RxHeaderTypeDef g_CanRxHeader;
extern uint8_t g_CanRxData[8];

#endif /* HAL_I2C_MODULE_ENABLED */
#endif /* __BSP_CAN_H_ */
/***************************** (END OF FILE) *********************************/
