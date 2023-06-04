#include "shell_port.h"

/*********************************************************************************************************/
void test(int a, float b, int c, float d)
{
    bsp_Log_Info("%d, %f, %d, %f ", a, b, c, d);
}
SHELL_EXPORT_CMD_AGENCY(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
                        test, test, test[a b c d],
                        p1, SHELL_PARAM_FLOAT(p2), p3, SHELL_PARAM_FLOAT(p4));

void test1(uint8_t i, char ch, char *str)
{
    bsp_Log_Info("input int: %d, char: %c, string: %s", i, ch, str);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), test1, test1, printstr[i ch str]);

void test2(char *str)
{
    const char *d = ",";
    char *p;
    uint8_t buff[8], len = 0;

    p = strtok(str, d);
    while (p != NULL)
    {
        // bsp_Log_Info("%s", p);
        if (len >= 8)
        {
            bsp_Log_Error("Parameter is too long...");
            break;
        }
        buff[len++] = atoi(p);

        p = strtok(NULL, d);
    }

    for (uint8_t i = 0; i < len; i++)
    {
        bsp_Log_Info("Data[%d] = %d", i, buff[i]);
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), test2, test2, strsplit[str]);
/*********************************************************************************************************/
/* 系統 */
void App_info(void)
{
    char info[128];
    char buffer[16];

    bsp_Get_App_Version(buffer);
    sprintf(info, "\r\nApp Version: %s\r\nBuild: %s %s\r\nAuthor: %s \r\nModel: %s", buffer, __DATE__, __TIME__, AUTHOR, EVALUATION_BOARD_TYPE);
    bsp_Log_Info("%s", info);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), App_info, App_info, App_info);

void App_Reboot(void)
{
    NVIC_SystemReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), App_Reboot, App_Reboot, App_Reboot);
/*********************************************************************************************************/
/* LED */
void LED(uint8_t _port, uint8_t _status)
{
    if (_port <= 4 && _port >= 1)
    {
        switch (_port)
        {
        case 1:
            if (_status == 0)
            {
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            }
            else if (_status == 1)
            {
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            }
            break;
        case 2:
            if (_status == 0)
            {
                HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
            }
            else if (_status == 1)
            {
                HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
            }
            break;
        case 3:
            if (_status == 0)
            {
                HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
            }
            else if (_status == 1)
            {
                HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
            }
            break;
        case 4:
            if (_status == 0)
            {
                HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
            }
            else if (_status == 1)
            {
                HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        bsp_Log_Error("parameter error !");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), LED, LED, LED Control[port status]);
/*********************************************************************************************************/
/* ADC  */
void GET_ADC_VR_Value(void)
{
    uint32_t _value = 0;
    bsp_Get_VR_Value(&_value);
    bsp_Log_Info("Src :%d, Voltage :%.2fV", _value, _value * 3.3 / 4096);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), GET_ADC_VR_Value, GET_ADC_VR_Value, GET_ADC_VR_Value);
/*********************************************************************************************************/
void GET_TEMPHUM(void)
{
    bsp_Log_Info("HUM:%.1f%%,TEMP:%.1fC", AHT10_Humidity, AHT10_Temperature);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), GET_TEMPHUM, GET_TEMPHUM, Get HumTemp);

void GET_ChipTEMP(void)
{
    bsp_Log_Info("ChipTEMP:%.1fC", ChipTemp);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), GET_ChipTEMP, GET_ChipTEMP, Get ChipTEMP);

void GET_Time(void)
{
    bsp_Log_Info("%02d/%02d/%02d %02d:%02d:%02d", 2000 + Get_Date.Year, Get_Date.Month, Get_Date.Date, Get_Time.Hours, Get_Time.Minutes, Get_Time.Seconds);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), GET_Time, GET_Time, Get Time);

void SET_TimeData(int year, int month, int date, int hours, int minutes, int seconds)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sDate.Year = year;
    sDate.Month = month;
    sDate.Date = date;

    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, (uint16_t)sDate.Year);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint16_t)sDate.Month);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint16_t)sDate.Date);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, (uint16_t)sDate.WeekDay);

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        bsp_Log_Info("SetDate error.");
    }

    sTime.Hours = hours;
    sTime.Minutes = minutes;
    sTime.Seconds = seconds;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        bsp_Log_Info("SetTime error.");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), SET_TimeData, SET_TimeData,
                 Set TimeData[year month date hours minutes seconds]);

/*********************************************************************************************************/
/* Modbus */
void Modbus_Param_01H(uint16_t _reg, uint16_t _num)
{
    if ((_reg >= 0x100) && (_reg <= 0x200))
    {
        if ((_num > 0) && (_num <= 64))
        {
            bsp_Log_Info("MODH_WriteParam_01H : %d", MODH_ReadParam_01H(_reg, _num));
        }
        else
        {
            bsp_Log_Error("num is out of range.(1~64)");
        }
    }
    else
    {
        bsp_Log_Error("reg is out of range.(0x100~0x200)");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), Modbus_Param_01H, Modbus_Param_01H, Modbus_Param_01H[reg num]);

void Modbus_Param_06H(uint16_t _reg, uint16_t _value)
{
    if ((_reg >= 0x3000) && (_reg <= 0x3100))
    {
        bsp_Log_Info("MODH_WriteParam_06H : %d", MODH_WriteParam_06H(_reg, _value));
    }
    else
    {
        bsp_Log_Error("reg is out of range.(0x3000~0x3100)");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), Modbus_Param_06H, Modbus_Param_06H, Modbus_Param_06H[reg value]);

/*********************************************************************************************************/
/* Canbus */
#ifdef HAL_I2C_MODULE_ENABLED

void Canbus_Std_ID(char *str)
{
    const char *d = ",";
    char *p;
    uint8_t buff[8], len = 0;

    p = strtok(str, d);
    while (p != NULL)
    {
        // bsp_Log_Info("%s", p);
        if (len >= 8)
        {
            bsp_Log_Error("Parameter is too long...");
            break;
        }
        buff[len++] = atoi(p);

        p = strtok(NULL, d);
    }

    bsp_CAN_SendPacket(buff, len);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), Canbus_Std_ID, Canbus_Std_ID, Canbus_Std_ID[str]);
#endif /* HAL_I2C_MODULE_ENABLED */
/*********************************************************************************************************/
/* PCF8574T */
extern IOExpand_DrvTypeDef *IODrv;
void EXIO_WritePin(uint8_t _pin, uint8_t _status)
{
    if (_pin <= 7 && _pin >= 0)
    {
        if (_status <= 1 && _status >= 0)
        {
            IODrv->WritePin(_pin, _status);
        }
        else
        {
            bsp_Log_Error("status is out of range.(0 or 1)");
        }
    }
    else
    {
        bsp_Log_Error("pin is out of range.(0 ~ 7)");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), EXIO_WritePin, EXIO_WritePin, EXIO_WritePin[pin status]);

void EXIO_ReadPin(uint8_t _pin)
{
    if (_pin <= 7 && _pin >= 0)
    {
        uint8_t _status = 0;
        _status = IODrv->ReadPin(_pin);
        bsp_Log_Info("P%d Current Status is (%x).", _pin, _status);
    }
    else
    {
        bsp_Log_Error("pin is out of range.(0 ~ 7)");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), EXIO_ReadPin, EXIO_ReadPin, EXIO_ReadPin[pin]);

void EXIO_TogglePin(uint8_t _pin)
{
    if (_pin <= 7 && _pin >= 0)
    {
        uint8_t _status = 0;
        _status = IODrv->TogglePin(_pin);
        if (_status != bsp_PASSED)
        {
            bsp_Log_Error("EXIO_TogglePin happen error.");
        }
    }
    else
    {
        bsp_Log_Error("pin is out of range.(0 ~ 7)");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), EXIO_TogglePin, EXIO_TogglePin, EXIO_TogglePin[pin]);

void EXIO_WriteByte(uint8_t value)
{
    if (value <= 0 && value >= 255)
    {
        IODrv->WriteByte(value);
    }
    else
    {
        bsp_Log_Error("value is out of range.(0 ~ 255)");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), EXIO_WriteByte, EXIO_WriteByte, EXIO_WriteByte[value]);

void EXIO_ReadByte(void)
{
    uint8_t _value = 0;
    uint16_t _len = 0;
    char bintemp[8] = {0};
    char finally[8] = {0};
    IODrv->ReadByte(&_value);
    _len = sizeof(_value) * 8;
    while ((_len--) > 0)
    {
        sprintf(bintemp, "%u", (_value >> _len) & 0x01);
        strcat(finally, bintemp);
    }
    bsp_Log_Info("Current Status is %s", finally);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), EXIO_ReadByte, EXIO_ReadByte, EXIO_ReadByte);
/*********************************************************************************************************/
/* USB_VCOM */
extern uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len);
void USB_VCOM_Packet(void)
{
    char Waveform1[10] = {0};
    size_t len = 0;
    ef_get_env_blob("waveform1", NULL, 0, &len);
    ef_get_env_blob("waveform1", Waveform1, len, NULL);
    CDC_Transmit_FS((uint8_t *)Waveform1, len);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), USB_VCOM_Packet, USB_VCOM_Packet, USB_VCOM_Packet);
/*********************************************END OF FILE*************************************************/
