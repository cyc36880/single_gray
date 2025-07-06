#include "control.h"
#include "light.h"

typedef  void (*pFunction)(void);

int uXMODEM_Process(void);
static void jump(uint32_t APPLICATION_ADDRESS);

static void enter_light_flash(void)
{
    for (uint8_t i=0; i<3; i++)
    {
        set_light_brightness(1000, 1000, 1000);
        SysTickDelay(100);
        set_light_brightness(0, 0, 0);
        SysTickDelay(100);
    }
    set_light_brightness(1000, 1000, 1000);
}


void setup(void)
{
    if (GPIO_ReadPin(KEY_GPIO_Port, KEY_PIN) == GPIO_Pin_RESET)
    {
        enter_light_flash();
        uXMODEM_Process();
    }
    jump(0x2800);
}

void loop(void)
{   
   
}




static void jump(uint32_t APPLICATION_ADDRESS)
{
    pFunction JumpToApplication;
    uint32_t JumpAddress;

    // SCB->VTOR = APPLICATION_ADDRESS; //设置中断向量偏移地址
    /* execute the new program */
    JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
    /* Jump to user application */
    JumpToApplication = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
    JumpToApplication();
}


