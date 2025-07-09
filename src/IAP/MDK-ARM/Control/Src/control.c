#include "control.h"
#include "light.h"


int uXMODEM_Process(void);

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
    enter_light_flash();
    uXMODEM_Process();
}

void loop(void)
{   
   
}






