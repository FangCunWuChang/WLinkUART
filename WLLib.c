/*
 * WLink状态处理
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLLib.h"
#include "WLMap.h"

#define WL_UART_DEFAULTTIMEOUT 1000 //默认发送超时

static WL_Map *WL_StateList = WL_NULL;
static WL_UINT64 WL_StateID = 0;

WL_UART_State *WL_UART_Create(UART_HandleTypeDef *uInitType)
{
    if (uInitType == WL_NULL)
    {
        return WL_NULL;
    }
    if (uInitType->Init.Mode != DMA_NORMAL)
    {
        return WL_NULL;
    } //强制要求串口使用DMA模式

    if (WL_StateList == WL_NULL)
    {
        WL_StateList = WL_Map_Create(sizeof(WL_UINT64), sizeof(WL_UART_State *));
    } //如果map不存在则建立

    WL_UART_State *state = pvPortMalloc(sizeof(WL_UART_State)); //分配内存
    state->id = (WL_StateID++);                                 //设置索引并迭代
    state->type = WL_UART_StateUART;                            //设置类型
    state->handle.uartHandle = uInitType;
    state->timeout = WL_UART_DEFAULTTIMEOUT;

    WL_Map_Insert(&WL_StateList, &(state->id), &state); // state存入map

    return state;
}

WL_UART_State *WL_USART_Create(USART_HandleTypeDef *uInitType)
{
    if (uInitType == WL_NULL)
    {
        return WL_NULL;
    }
    if (uInitType->Init.Mode != DMA_NORMAL)
    {
        return WL_NULL;
    } //强制要求串口使用DMA模式

    if (WL_StateList == WL_NULL)
    {
        WL_StateList = WL_Map_Create(sizeof(WL_UINT64), sizeof(WL_UART_State *));
    } //如果map不存在则建立

    WL_UART_State *state = pvPortMalloc(sizeof(WL_UART_State)); //分配内存
    state->id = (WL_StateID++);                                 //设置索引并迭代
    state->type = WL_UART_StateUSART;                           //设置类型
    state->handle.usartHandle = uInitType;
    state->timeout = WL_UART_DEFAULTTIMEOUT;

    WL_Map_Insert(&WL_StateList, &(state->id), &state); // state存入map

    return state;
}

void WL_UART_Destory(WL_UART_State *state)
{
    if (state == WL_NULL)
    {
        return;
    }

    WL_Map_Erase(&WL_StateList, &(state->id)); //从map中移除

    pvPortFree(state);

    if (WL_Map_Size(&WL_StateList) == 0)
    {
        WL_Map_Destory(&WL_StateList);
        WL_StateList = WL_NULL;
    } //若map为空则销毁
}

WL_BOOLEAN WL_UART_SetTimeOut(WL_UART_State *state, WL_TIMETICK timeout)
{
    if (state == WL_NULL)
    {
        return WL_FALSE;
    }
    if (state->sendState == WL_UART_SendStateBusy)
    {
        return WL_FALSE;
    }
    state->timeout = timeout;
    return WL_TRUE;
}
