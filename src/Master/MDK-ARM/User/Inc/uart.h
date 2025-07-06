#ifndef __uart_h__
#define __uart_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef struct 
{
    UART_TypeDef* Instance;
    UART_InitTypeDef Init;
} UART_HandleTypeDef;


void MAX_UART2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __uart_h__ */
