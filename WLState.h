#pragma once
/*
 * WLink状态
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLInclude.h"
#include "WLBase.h"
#include "WLLLP.h"

typedef enum
{
    WL_UART_SendStateEmpty = 0,
    WL_UART_SendStateConnected0 = 1,
    WL_UART_SendStateConnected1 = 2,
    WL_UART_SendStateConnected2 = 3,
    WL_UART_SendStateBusy = 4
} WL_UART_SendState;

typedef void (*WL_UART_HookFunction)(void *);

typedef struct
{
    WL_UINT32 id;                         //索引
    WL_UART_LLP *llp;                     //底层串口
    WL_TIMETICK timeout;                  //传输超时
    WL_UART_SendState sendState;          //发送状态
    WL_BOOLEAN isReading;                 //正在接收
    xTimerHandle timer;                   //传输定时器
    WL_UART_HookFunction readFunc;        //串口输入回调
    WL_UART_HookFunction readErrorFunc;   //串口输入错误回调
    WL_UART_HookFunction writeErrorFunc;  //串口输出错误回调
    WL_UART_HookFunction writeFinishFunc; //串口队列输出结束回调
    WL_UINT32 peerID;                     //远程id

} WL_UART_State; // WLink状态
