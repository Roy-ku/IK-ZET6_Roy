#ifndef _BSP_CPU_FLASH_H_
#define _BSP_CPU_FLASH_H_

#define CPU_FLASH_BASE_ADDR (uint32_t)(0x08008000)     /* APP開始地址 */
#define CPU_FLASH_END_ADDR (uint32_t)(FLASH_BANK1_END) /* FLASH結束地址 */

#define CPU_FLASH_SIZE (480 * 1024)      /* FLASH總容量 單位:Byte */
#define CPU_FLASH_SECTOR_SIZE (2 * 1024) /* 頁區大小，單位:Byte */

bsp_Status bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint32_t *_ucpDst, uint32_t _ulSize);
bsp_Status bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint32_t *_ucpSrc, uint32_t _ulSize);
bsp_Status bsp_EraseCpuFlash(uint32_t _ulFlashAddr);
void bsp_CpuFlash_Test(void);
#endif

/***************************** (END OF FILE) *********************************/
