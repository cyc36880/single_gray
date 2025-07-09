/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : ZIShen
 * @Date         : 2025-07-07 17:10:09
 * @LastEditTime : 2025-07-09 10:35:38
 * @Description  : 
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "control.h"
#include "i2c.h"
#include "light.h"
#include "sys_config_and_flash.h"
#include "stdio.h"
#include "register.h"
#include "gray.h"
#include "binary.h"
#include "color.h"

typedef void (*key_func_cb)(void);

typedef enum
{
    KEY_IDLE = 0,
    KEY_ON_CLICK,
    KEY_ON_LONG,
} KEY_STATE;

typedef enum
{
    MACHINE_IDLE = 0,
    MACHINE_GRAY_IDENTIFY,
    MACHINE_BINARY_IDENTIFY,
    MACHINE_GRAY_STUDY,
    MACHINE_BINARY_STUDY,

    MACHINE_COLOR_IDENTIFY,
    MACHINE_COLOR_CLEAR_STUDY,
    MACHINE_COLOR_RED_STUDY,
    MACHINE_COLOR_GREEN_STUDY,
    MACHINE_COLOR_BLUE_STUDY,
    MACHINE_COLOR_YELLOW_STUDY,
    MACHINE_COLOR_CYAN_STUDY,
    MACHINE_COLOR_PUPRLE_STUDY,
    MACHINE_COLOR_BLACK_STUDY,
    MACHINE_COLOR_WHITE_STUDY,
} MACHINE_STATE;


typedef struct
{
    KEY_STATE state;
    key_func_cb on_click;
    key_func_cb on_long;
    uint32_t last_press_time;
    uint8_t press_flag : 1;
    uint8_t tarigger_flag : 1;
} KET_Typedef;

static void key_event_handling(void);
static void other_addr_light_color(void);

static const uint16_t other_addr_color[][3] = {
    {1000, 0, 0},
    {0, 1000, 0},
    {0, 0, 1000},
    {0, 1000, 1000},
    {1000, 0, 1000},
    {1000, 1000, 0},
};

static const uint8_t iic_command_map[][2] = {
    {0x00, MACHINE_IDLE},
    {0x01, MACHINE_GRAY_IDENTIFY},
    {0x02, MACHINE_BINARY_IDENTIFY},
    {0x03, MACHINE_GRAY_STUDY},
    {0x04, MACHINE_BINARY_STUDY},

    {0x05, MACHINE_COLOR_IDENTIFY},
    {0x06, MACHINE_COLOR_CLEAR_STUDY},
    {0x07, MACHINE_COLOR_RED_STUDY},
    {0x08, MACHINE_COLOR_GREEN_STUDY},
    {0x09, MACHINE_COLOR_BLUE_STUDY},
    {0x0A, MACHINE_COLOR_YELLOW_STUDY},
    {0x0B, MACHINE_COLOR_CYAN_STUDY},
    {0x0C, MACHINE_COLOR_PUPRLE_STUDY},
    {0x0D, MACHINE_COLOR_BLACK_STUDY},
    {0x0E, MACHINE_COLOR_WHITE_STUDY},
};

static uint8_t iic_read_reg_idle_val[2] = {0, 1};
static uint8_t iic_read_reg_study_val[2] = {0, 2};

static KET_Typedef key_info = {0};
static MACHINE_STATE machine_state = MACHINE_IDLE;


static void key_on_click_cb(void)
{
    machine_state = MACHINE_GRAY_STUDY;
}

static void key_on_long_cb(void)
{
    MAX_I2C_addr_other_changle();
    other_addr_light_color();
}


void setup(void)
{
    other_addr_light_color();
    key_info.on_click = key_on_click_cb;
    key_info.on_long = key_on_long_cb;
    Grayscale_Init();
    // color_init();
    color2_init();

    iic_read_reg.reg = iic_read_reg_idle_val;
    iic_read_reg.size = 2;
}

void loop(void)
{   
    key_event_handling();

    if (MACHINE_IDLE == machine_state) // 空闲
    {
        set_rgb_brightness(0, 0, 0);
        iic_read_reg.reg = iic_read_reg_idle_val;
        iic_read_reg.size = 2;
    }
    else if (MACHINE_GRAY_IDENTIFY == machine_state) // 灰度识别
    {
        gary_identify();
        iic_read_reg.reg = grayVal;
        iic_read_reg.size = 1;
    }
    else if (MACHINE_BINARY_IDENTIFY == machine_state) // 二值识别
    {
        binary_identify();
        iic_read_reg.reg = binaryVal;
        iic_read_reg.size = 1;
    }
    else if (MACHINE_COLOR_IDENTIFY == machine_state) // 颜色识别
    {
        color2_identify();
        iic_read_reg.reg = color2Val;
        iic_read_reg.size = 1;
        // color_identify();
        // iic_read_reg.reg = colorVal;
        // iic_read_reg.size = 1;
    }
    else if (MACHINE_GRAY_STUDY == machine_state) // 灰度学习
    {
        iic_read_reg.reg = iic_read_reg_study_val;
        iic_read_reg.size = 2;
        gary_study();
        machine_state = MACHINE_GRAY_IDENTIFY;
    }
    else if (MACHINE_BINARY_STUDY == machine_state) // 二值学习
    {
        iic_read_reg.reg = iic_read_reg_study_val;
        iic_read_reg.size = 2;
        binary_study();
        machine_state = MACHINE_BINARY_IDENTIFY;
    }
    else if (MACHINE_COLOR_CLEAR_STUDY == machine_state) // 清除颜色学习
    {
        iic_read_reg.reg = iic_read_reg_idle_val;
        iic_read_reg.size = 2;
        // clear_color_study();
        clear_color2_study();
        machine_state = MACHINE_IDLE;
    }
    // 颜色学习
    else if (MACHINE_COLOR_RED_STUDY<=machine_state && machine_state<=MACHINE_COLOR_WHITE_STUDY)
    {
        // iic_read_reg.reg = iic_read_reg_study_val;
        // iic_read_reg.size = 2;
        // color_study(machine_state - MACHINE_COLOR_RED_STUDY);
        // machine_state = MACHINE_COLOR_IDENTIFY;
        iic_read_reg.reg = iic_read_reg_study_val;
        iic_read_reg.size = 2;
        color2_study(machine_state - MACHINE_COLOR_RED_STUDY);
        machine_state = MACHINE_COLOR_IDENTIFY;
    }

    // IIC 写入命令处理
    if (1 == iic_write_reg.changle_flag)
    {
        iic_write_reg.changle_flag = 0;
        uint8_t find_flag=0, pos=0;
        for (uint8_t i = 0; i < sizeof(iic_command_map) / sizeof(iic_command_map[0]); i++)
        {
            if (iic_write_reg.reg[0] == iic_command_map[i][0])
            {
                find_flag = 1;
                pos = i;
                break;
            }
        }
        if (find_flag)
        {
            machine_state =  iic_command_map[pos][1];
        }
    }
}









static void other_addr_light_color(void)
{
    uint8_t other_addr_pos = 0;
    const uint16_t *other_addr_color_ptr;
    other_addr_pos = MAX_I2C_addr_get_other();
    other_addr_color_ptr = other_addr_color[other_addr_pos];
    set_light_brightness(other_addr_color_ptr[0], other_addr_color_ptr[1], other_addr_color_ptr[2]);
}



// ======== KEY ========
static void key_event_handling(void)
{
    uint8_t key_press = GPIO_ReadPin(KEY_GPIO_Port, KEY_PIN);

    // 按键按下
    if (key_press == 0)
    {
        if (key_info.press_flag == 0)
        {
            key_info.press_flag = 1;
            key_info.last_press_time = GetTick();
            key_info.state = KEY_IDLE;
            key_info.tarigger_flag = 0;
        }
        else if (key_info.state == KEY_IDLE && (GetTick() - key_info.last_press_time > 1000))
        {
            key_info.state = KEY_ON_LONG;
            if (!key_info.tarigger_flag)
            {
                key_info.tarigger_flag = 1;
                if (key_info.on_long)
                    key_info.on_long();
            }
        }
    }
    // 按键抬起
    else
    {
        if (key_info.press_flag == 1)
        {
            uint32_t press_duration = GetTick() - key_info.last_press_time;
            if (press_duration >= 30 && press_duration <= 1000)
            {
                key_info.state = KEY_ON_CLICK;
                if (!key_info.tarigger_flag)
                {
                    key_info.tarigger_flag = 1;
                    if (key_info.on_click)
                        key_info.on_click();
                }
            }
        }
        // 重置状态
        key_info.press_flag = 0;
        key_info.state = KEY_IDLE;
        key_info.tarigger_flag = 0;
    }
}
