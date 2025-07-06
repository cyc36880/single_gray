#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "time.h"
#include "stdio.h"
#include "control.h"

#include "light.h"

static void SystemClock_Config(void);

int main(void)
{
    SystemClock_Config();
    MX_GPIO_Init();
    MAX_UART1_Init();
    // MAX_ATIME_Init(); // 有问题？
    MAX_GTIME_Init();
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
    SYSCTRL_HSI_Enable(SYSCTRL_HSIOSC_DIV6);
    SYSCTRL_HSE_Enable(SYSCTRL_HSE_MODE_OSC, 32000000, SYSCTRL_HSE_DRIVER_LEVEL5, 0); // 开外部高速时钟，实际频率需要根据实际晶体频率重新配置
    SYSCTRL_SysClk_Switch(SYSCTRL_SYSCLKSRC_HSE);
   InitTick(32000000);
   SYSCTRL_SystemCoreClockUpdate(32000000);

   REGBITS_SET(CW_SYSCTRL->AHBEN, (0x5A5A0000 | bv1));
   REGBITS_SET(CW_SYSCTRL->AHBEN, (0x5A5A0000 | bv5));
}
