/*
 * WLink状态处理
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLInclude.h"
#include "WLState.h"

WL_UART_State *WL_UART_Create(UART_HandleTypeDef *);
WL_UART_State *WL_USART_Create(USART_HandleTypeDef *);

void WL_UART_Destory(WL_UART_State *);
void (*WL_USART_Destory)(WL_UART_State *) = WL_UART_Destory;

WL_BOOLEAN WL_UART_SetTimeOut(WL_UART_State *, WL_TIMETICK);
WL_BOOLEAN (*WL_USART_SetTimeOut)
(WL_UART_State *, WL_TIMETICK) = WL_UART_SetTimeOut;
