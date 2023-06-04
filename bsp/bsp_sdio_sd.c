#include "bsp_sdio_sd.h"

/**
 * @brief 讀取SD卡信息
 */
void bsp_SD_INFO(void)
{
    if (HAL_SD_GetCardState(&hsd) != (HAL_SD_CARD_DISCONNECTED | HAL_SD_CARD_ERROR))
    {

        HAL_SD_CardStatusTypeDef sdcard_status = {0};
        HAL_SD_CardCIDTypeDef sdcard_cid = {0};

        /* SD卡基本信息 */
        bsp_Log_Info(" SD card information!");
        bsp_Log_Info(" CardCapacity  : %llu", (unsigned long long)hsd.SdCard.BlockSize * hsd.SdCard.BlockNbr); // 顯示容量
        bsp_Log_Info(" CardBlockSize : %d", hsd.SdCard.BlockSize);                                             // 塊大小
        bsp_Log_Info(" CardBlockNbr  : %d", hsd.SdCard.BlockNbr);                                              // 塊數量
        bsp_Log_Info(" LogBlockNbr   : %d", hsd.SdCard.LogBlockNbr);                                           // 邏輯塊數量
        bsp_Log_Info(" LogBlockSize  : %d", hsd.SdCard.LogBlockSize);                                          // 邏輯塊大小
        bsp_Log_Info(" RCA           : %d", hsd.SdCard.RelCardAdd);                                            // 卡相對地址
        bsp_Log_Info(" CardType      : %d", hsd.SdCard.CardType);                                              // 卡類型
        bsp_Log_Info(" Class         : %d", hsd.SdCard.Class);
        bsp_Log_Info(" CardVersion   : %d", hsd.SdCard.CardVersion);

        HAL_SD_GetCardStatus(&hsd, &sdcard_status);
        bsp_Log_Info(" DataBusWidth  : %d", sdcard_status.DataBusWidth);
        bsp_Log_Info(" SpeedClass    : %d", sdcard_status.SpeedClass);
        bsp_Log_Info(" SecuredMode   : %d", sdcard_status.SecuredMode);
        bsp_Log_Info(" PAS           : %d", sdcard_status.ProtectedAreaSize);
        bsp_Log_Info(" PM            : %d", sdcard_status.PerformanceMove);
        bsp_Log_Info(" AUS           : %d", sdcard_status.AllocationUnitSize);
        bsp_Log_Info(" EraseSize     : %d", sdcard_status.EraseSize);
        bsp_Log_Info(" EraseTimeout  : %d", sdcard_status.EraseTimeout);
        bsp_Log_Info(" EraseOffset   : %d", sdcard_status.EraseOffset);

        /* 讀取並打印SD卡的CID信息 */
        HAL_SD_GetCardCID(&hsd, &sdcard_cid);
        bsp_Log_Info(" ManufacturerID: %d", sdcard_cid.ManufacturerID);
    }
    else
    {
        bsp_Log_Error("SD card fail!");
    }
}
