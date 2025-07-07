#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "xmodem.h"
#include "common.h"
#include "light.h"
// ----------------- Flash 参数配置 ------------------
// #define def_FLASH_BASE ((uint32_t)0x08000000)
// #define def_FLASH_PAGESIZE ((uint32_t)2048) // 页面大小（你要根据芯片实际配置）
// #define def_FLASH_PAGECOUNT ((uint32_t)256) // 总页数
// #define def_USERAPP_START (def_FLASH_BASE + def_FLASH_PAGESIZE * 20)
// #define def_USERAPP_BOTTOM (def_FLASH_BASE + def_FLASH_PAGESIZE * (def_FLASH_PAGECOUNT - 2))

// ----------------- XMODEM 控制码 -------------------
#define XMODEM_NUL 0x00
#define XMODEM_SOH 0x01
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_EOF 0x1A

#define XMODEM_WAIT_CHAR XMODEM_NAK

#define dST_WAIT_START 0x00
#define dST_BLOCK_OK 0x01
#define dST_BLOCK_FAIL 0x02
#define dST_OK 0x03

// ----------------- XMODEM 接收数据结构 ---------------
typedef struct
{
    uint8_t SOH;
    uint8_t BlockNo;
    uint8_t nBlockNo;
    uint8_t Xdata[128];
    uint8_t CheckSum;
} XMODEM_Frame_t;

static XMODEM_Frame_t strXMODEM;
static uint8_t gXM_BlockCount = 1;
static uint8_t gXM_STATUS = dST_WAIT_START;
static uint32_t gFlash_User_Address = 0;
static uint32_t gFlash_User_Count = 0;
static uint32_t gFlash_Count = 0;

static uint8_t Block_512[512] = {0};

// ======= 用户实现接口 ==========
extern void uart_send_byte(uint8_t byte);
extern int uart_receive_bytes(uint8_t *buf, uint16_t len, uint32_t timeout_ms);
extern void SysTickDelay(uint32_t ms);


// ======= 工具函数 ==============
static void uUART_PutChar(uint8_t Data)
{
    uart_send_byte(Data);
}

static uint8_t get_data(uint8_t *ptr, uint16_t len, uint32_t timeout_ms)
{
    return uart_receive_bytes(ptr, len, timeout_ms) ;
}

static uint8_t uCheckSum(const uint8_t *ptr, uint8_t len)
{
    uint8_t sum = 0;
    while (len--)
        sum += *ptr++;
    return sum;
}

// ========== XMODEM 主处理函数 ===========
void uXMODEM_Process(void)
{
    uint16_t i;
    uint8_t checksum;

    while (1)
    {
        printf("--> Bootloader ready. Waiting for [d] key to enter upgrade mode.\r\n");

        uint8_t rev_data = 0;
        if (get_data(&rev_data, 1, 1000) == 1)
        {
            if (rev_data == 'd' || rev_data == 'D')
            {
                break;
            }
        }
    }
    printf("wait .. \n");


    if (gXM_STATUS != dST_WAIT_START)
    {
        printf("--> Timeout. Jumping to user app.\r\n");
        gXM_STATUS = dST_OK;
        return;
    }

    // 初始化参数
    gXM_BlockCount = 1;
//    gFlash_User_Address = def_USERAPP_START;

    // 主循环：接收数据包
    while (gXM_STATUS != dST_OK)
    {
        // 发送 NAK 提示继续发送
        if (gXM_STATUS == dST_WAIT_START)
        {
            uUART_PutChar(XMODEM_WAIT_CHAR);
        }

        i = get_data((uint8_t *)&strXMODEM, sizeof(XMODEM_Frame_t), 1000);
        if (i == 0)
            continue;

        switch (strXMODEM.SOH)
        {
        case XMODEM_SOH:
            if (i >= sizeof(XMODEM_Frame_t))
            {
                gXM_STATUS = dST_BLOCK_OK;
            }
            else
            {
                gXM_STATUS = dST_BLOCK_FAIL;
                uUART_PutChar(XMODEM_NAK);
            }
            break;

        case XMODEM_EOT:
            uUART_PutChar(XMODEM_ACK);
            gXM_STATUS = dST_OK;
            printf("--> Upgrade finished.\r\n");
            break;

        case XMODEM_CAN:
            uUART_PutChar(XMODEM_ACK);
            gXM_STATUS = dST_OK;
            printf("--> Upgrade cancelled by sender.\r\n");
            break;

        default:
            uUART_PutChar(XMODEM_NAK);
            gXM_STATUS = dST_BLOCK_FAIL;
            break;
        }

        if (gXM_STATUS == dST_BLOCK_OK)
        {
            // 校验编号与反码
            if ((uint8_t)(strXMODEM.BlockNo + strXMODEM.nBlockNo) != 0xFF)
            {
                uUART_PutChar(XMODEM_NAK);
                continue;
            }

            // 处理重传包
            if (strXMODEM.BlockNo == (uint8_t)(gXM_BlockCount - 1))
            {
                uUART_PutChar(XMODEM_ACK); // 已处理，直接 ACK
                continue;
            }

            // 检查是否正确的包号
            if (strXMODEM.BlockNo != gXM_BlockCount)
            {
                uUART_PutChar(XMODEM_NAK);
                continue;
            }

            // 校验 CheckSum
            checksum = uCheckSum(strXMODEM.Xdata, 128);
            if (checksum != strXMODEM.CheckSum)
            {
                uUART_PutChar(XMODEM_NAK);
                continue;
            }
            // 校验通过，写入 Flash
            memcpy(Block_512+ (gFlash_User_Count%4)*128, strXMODEM.Xdata, 128);
            gFlash_User_Address += 128;
            gFlash_User_Count++;
            set_light_brightness(1000*(gFlash_User_Count%2), 1000*(gFlash_User_Count%2), 1000*(gFlash_User_Count%2));
            if (gFlash_User_Count%4 == 0)
            {
                gFlash_Count++;
                uint32_t target_addr = FLASH_START_ADDR + (gFlash_Count-1)*512;

                FLASH_UnlockPages(target_addr, target_addr);
                FLASH_ErasePages(target_addr, target_addr);
                FLASH_LockAllPages();

                SysTickDelay(1);

                FLASH_UnlockPages(target_addr, target_addr);
                FLASH_WriteBytes(target_addr, Block_512, 512);
                FLASH_LockAllPages();
            }
            // 包计数+1，ACK应答
            gXM_BlockCount++;
            uUART_PutChar(XMODEM_ACK);
        }
    }
    // 收尾：最后不足 512 字节的数据写入 Flash
    if (gFlash_Count % 4 != 0)
    {
        gFlash_Count++;
        uint32_t target_addr = FLASH_START_ADDR + (gFlash_Count-1)*512;
        FLASH_UnlockPages(target_addr, target_addr);
        FLASH_ErasePages(target_addr, target_addr);
        FLASH_WriteBytes(target_addr, Block_512, 512);
        FLASH_LockAllPages();
    }
    set_light_brightness(0, 0, 0);
}
