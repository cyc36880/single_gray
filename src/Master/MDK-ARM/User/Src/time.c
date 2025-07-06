#include "time.h"

static void HAL_TIME_GPIO_Init(TIME_HandleTypeDef* timeHandle);


TIME_HandleTypeDef gtime;
TIME_HandleTypeDef atime;

void MAX_ATIME_Init(void)
{
    ATIM_InitTypeDef ATIM_InitStruct = {DISABLE};
    ATIM_OCInitTypeDef ATIM_OCInitStruct = {0};

    __SYSCTRL_ATIM_CLK_ENABLE();

    ATIM_InitStruct.BufferState = DISABLE;
    ATIM_InitStruct.CounterAlignedMode = ATIM_COUNT_ALIGN_MODE_EDGE;
    ATIM_InitStruct.CounterDirection = ATIM_COUNTING_UP;
    ATIM_InitStruct.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
    ATIM_InitStruct.Prescaler = 4-1;    
    ATIM_InitStruct.ReloadValue = 1000-1; 
    ATIM_InitStruct.RepetitionCounter = 0;
    ATIM_Init(&ATIM_InitStruct);

    ATIM_OCInitStruct.BufferState = DISABLE;
    ATIM_OCInitStruct.OCComplement = DISABLE;
    ATIM_OCInitStruct.OCFastMode = DISABLE;
    ATIM_OCInitStruct.OCInterruptState = DISABLE;
    ATIM_OCInitStruct.OCMode = ATIM_OCMODE_PWM1;
    ATIM_OCInitStruct.OCPolarity = ATIM_OCPOLARITY_NONINVERT;
    ATIM_OCInitStruct.OCNPolarity = ATIM_OCPOLARITY_NONINVERT;
    ATIM_OC1Init(&ATIM_OCInitStruct);
    ATIM_OC2Init(&ATIM_OCInitStruct);

    ATIM_SetCompare1(1000);
    ATIM_SetCompare2(1000);
    ATIM_CH1Config(ENABLE);
    ATIM_CH2Config(ENABLE);
    ATIM_CtrlPWMOutputs(ENABLE);
    ATIM_Cmd(ENABLE);
   
    HAL_TIME_GPIO_Init(&atime);
}


void MAX_GTIME_Init(void)
{
    GTIM_InitTypeDef GTIM_InitStruct = {0};
    GTIM_OCModeCfgTypeDef GTIM_OCModeCfgStruct = {DISABLE,DISABLE,0};
    __SYSCTRL_GTIM1_CLK_ENABLE();

    GTIM_InitStruct.AlignMode = GTIM_ALIGN_MODE_EDGE;
    GTIM_InitStruct.ARRBuffState = GTIM_ARR_BUFF_EN;
    GTIM_InitStruct.Direction = GTIM_DIRECTION_UP;
    GTIM_InitStruct.EventOption = GTIM_EVENT_NORMAL;
    GTIM_InitStruct.Prescaler = 4 - 1;
    GTIM_InitStruct.PulseMode = GTIM_PULSE_MODE_DIS;
    GTIM_InitStruct.ReloadValue = 1000 -1;
    GTIM_InitStruct.UpdateOption = GTIM_UPDATE_DIS;
    GTIM_TimeBaseInit(CW_GTIM1, &GTIM_InitStruct);

    GTIM_OCModeCfgStruct.FastMode = DISABLE;
    GTIM_OCModeCfgStruct.OCMode = GTIM_OC_MODE_PWM1;
    GTIM_OCModeCfgStruct.OCPolarity = GTIM_OC_POLAR_NONINVERT;
    GTIM_OCModeCfgStruct.PreloadState = DISABLE;
    GTIM_OC4ModeCfg(CW_GTIM1, &GTIM_OCModeCfgStruct);
    GTIM_OC3ModeCfg(CW_GTIM1, &GTIM_OCModeCfgStruct);
    GTIM_OC2ModeCfg(CW_GTIM1, &GTIM_OCModeCfgStruct);
    GTIM_OC1ModeCfg(CW_GTIM1, &GTIM_OCModeCfgStruct);


    GTIM_SetCompare4(CW_GTIM1, 1000);
    GTIM_SetCompare3(CW_GTIM1, 1000);
    GTIM_SetCompare2(CW_GTIM1, 1000);
    GTIM_SetCompare1(CW_GTIM1, 1000);

    GTIM_OC4Cmd(CW_GTIM1, ENABLE);
    GTIM_OC3Cmd(CW_GTIM1, ENABLE);
    GTIM_OC2Cmd(CW_GTIM1, ENABLE);
    GTIM_OC1Cmd(CW_GTIM1, ENABLE);

    GTIM_Cmd(CW_GTIM1, ENABLE);

    gtime.xtime_Instance = (void *)CW_GTIM1;
    

    HAL_TIME_GPIO_Init(&gtime);
}



static void HAL_TIME_GPIO_Init(TIME_HandleTypeDef* timeHandle)
{
    if (timeHandle == &gtime)
    {
        __SYSCTRL_GPIOB_CLK_ENABLE();
        
        PB03_DIGTAL_ENABLE();
        PB03_DIR_OUTPUT();
        PB03_PUSHPULL_ENABLE();
        PB03_AFx_GTIM1CH4();

        PB04_DIGTAL_ENABLE();
        PB04_DIR_OUTPUT();
        PB04_PUSHPULL_ENABLE();
        PB04_AFx_GTIM1CH3();

        PB05_DIGTAL_ENABLE();
        PB05_DIR_OUTPUT();
        PB05_PUSHPULL_ENABLE();
        PB05_AFx_GTIM1CH2();

        PB06_DIGTAL_ENABLE();
        PB06_DIR_OUTPUT();
        PB06_PUSHPULL_ENABLE();
        PB06_AFx_GTIM1CH1();
    }
    else if (timeHandle == &atime)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        PB00_AFx_ATIMCH1();
        PB01_AFx_ATIMCH2();
        GPIO_InitStruct.IT = GPIO_IT_NONE;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pins = GPIO_PIN_0;
        GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pins = GPIO_PIN_1;
        GPIO_Init(CW_GPIOB, &GPIO_InitStruct);
    }
}



