#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "time.h"
#include "stdio.h"
#include "control.h"
#include "xmodem.h"
#include "light.h"

static void SystemClock_Config(void);

typedef  void (*pFunction)(void);

static void jump(uint32_t APPLICATION_ADDRESS)
{
    pFunction JumpToApplication;
    uint32_t JumpAddress;
    __disable_irq();    // 关中断
    
    SCB->VTOR = APPLICATION_ADDRESS; //设置中断向量偏移地址
    /* execute the new program */
    JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
    /* Jump to user application */
    JumpToApplication = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
    JumpToApplication();
}


int main(void)
{
    MX_GPIO_Init();
    
    if (GPIO_ReadPin(KEY_GPIO_Port, KEY_PIN) == GPIO_Pin_RESET)
    {
        SystemClock_Config();
        MAX_UART1_Init();
        // MAX_ATIME_Init(); // 有问题？
        MAX_GTIME_Init();
        setup();
    }
    jump(FLASH_START_ADDR);
    
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
