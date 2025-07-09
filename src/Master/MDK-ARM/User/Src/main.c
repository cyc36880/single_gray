#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "time.h"
#include "adc.h"
#include "i2c.h"

#include "control.h"
#include "sys_config_and_flash.h"

static void SystemClock_Config(void);

int main(void)
{
    __enable_irq();
    SystemClock_Config();
    sys_config_info_init(); // 从flash中读取配置信息
    MX_GPIO_Init();
    MAX_UART2_Init();
    MAX_ATIME_Init();
    MAX_GTIME_Init();
    MAX_ADC_Init();
    MAX_I2C_Init();
    
    setup();
    while (1)
    {
        loop();
    }
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif

static void SystemClock_Config(void)
{
    uint32_t hsi_calibration, hsi_calibration_h, hsi_calibration_l;
    hsi_calibration_l = *(uint8_t *)(0x001007C0);
    hsi_calibration_h = *(uint8_t *)(0x001007C1);
    hsi_calibration = (hsi_calibration_h << 8) | hsi_calibration_l;
    CW_SYSCTRL->HSI_f.TRIM = hsi_calibration; // 校准内部48M频率
    SYSCTRL_HSI_Enable(SYSCTRL_HSIOSC_DIV2);
//    SYSCTRL_HSE_Enable(SYSCTRL_HSE_MODE_OSC, 32000000, SYSCTRL_HSE_DRIVER_LEVEL5, 0); // 开外部高速时钟，实际频率需要根据实际晶体频率重新配置
    SYSCTRL_SysClk_Switch(SYSCTRL_SYSCLKSRC_HSE);
    InitTick(24000000);
    SYSCTRL_SystemCoreClockUpdate(24000000);

    REGBITS_SET(CW_SYSCTRL->AHBEN, (0x5A5A0000 | bv1));
    REGBITS_SET(CW_SYSCTRL->AHBEN, (0x5A5A0000 | bv5));
    
    //    FLASH_SetLatency(FLASH_Latency_2);                   //频率大于24M需要配置FlashWait=2
    FLASH_SetLatency(FLASH_Latency_1);                    //频率小于24M需要配置FlashWait=1
}
