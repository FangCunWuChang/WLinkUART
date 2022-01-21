/*
 * WLink状态处理
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLInclude.h"
#include "WLState.h"

#define WL_UART_FrameMax 512                       //最大数据帧长
#define WL_UART_BufferSize (WL_UART_FrameMax + 24) //缓存大小
#define WL_UART_MagicNumber 0x66b66b6b             //校验数

#define WL_UART_Head1 0x11111111
#define WL_UART_Head2 0x22222222
#define WL_UART_Head3 0x33333333
#define WL_UART_Head4 0x44444444 //操作头

WL_UART_State *WL_UART_Create(UART_HandleTypeDef *, WL_UINT32, WL_UINT32, WL_UART_HookFunction, WL_UART_HookFunction);

void WL_UART_Destory(WL_UART_State *);

WL_BOOLEAN WL_UART_Connect(WL_UART_State *);

WL_BOOLEAN WL_UART_SetTimeOut(WL_UART_State *, WL_TIMETICK);

WL_BOOLEAN WL_UART_Write(WL_UART_State *, void *, WL_SIZE_T, WL_UART_HookFunction, WL_UART_HookFunction);

void _WL_UART_ReadCallback(UART_HandleTypeDef *);