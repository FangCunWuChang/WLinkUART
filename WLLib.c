/*
 * WLink状态处理
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
*/

#include "WLLib.h"

#define WL_UART_DEFAULTTIMEOUT 1000//默认发送超时

WL_UART_State *WL_UART_Create(UART_HandleTypeDef *uInitType)
{
    if(uInitType==WL_NULL){
        return WL_NULL;
    }
    if(uInitType->Init.Mode!=DMA_NORMAL){
        return WL_NULL;
    }//强制要求串口使用DMA模式
    WL_UART_State *state = pvPortMalloc(sizeof(WL_UART_State));//分配内存
    state->type=WL_UART_StateUART;//设置类型
    state->handle.uartHandle=uInitType;
    state->timeout=WL_UART_DEFAULTTIMEOUT;
    return state;
}

WL_UART_State *WL_USART_Create(USART_HandleTypeDef *uInitType)
{
    if(uInitType==WL_NULL){
        return WL_NULL;
    }
    if(uInitType->Init.Mode!=DMA_NORMAL){
        return WL_NULL;
    }//强制要求串口使用DMA模式
    WL_UART_State *state = pvPortMalloc(sizeof(WL_UART_State));//分配内存
    state->type=WL_UART_StateUSART;//设置类型
    state->handle.usartHandle=uInitType;
    state->timeout=WL_UART_DEFAULTTIMEOUT;
    return state;
}

WL_BOOLEAN WL_UART_SetTimeOut(WL_UART_State* state,WL_TIMETICK timeout)
{
    if(state==WL_NULL){
        return WL_FALSE;
    }
    if(state->sendState==WL_UART_SendStateBusy){
        return WL_FALSE;
    }
    state->timeout=timeout;
    return WL_TRUE;
}
