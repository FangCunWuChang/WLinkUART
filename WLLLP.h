#pragma once
/*
 * WLink底层端口
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLInclude.h"
#include "WLBase.h"

typedef enum
{
    WL_UART_LLPReadEmpty = 0,
    WL_UART_LLPReadReady = 1,
    WL_UART_LLPReadBusy = 2
} WL_UART_LLPReadState;

typedef enum
{
    WL_UART_LLPWriteEmpty = 0,
    WL_UART_LLPWriteBusy = 1
} WL_UART_LLPWriteState;

typedef void (*WL_UART_LLPHookFunction)(void *);

typedef struct
{
    UART_HandleTypeDef *handle; //串口句柄
    WL_UINT8 buffer;            //帧缓存
    WL_BOOLEAN mutex;           //互斥锁
    WL_UINT32 count;            //引用计数

    WL_SIZE_T rSize;               //读入数据长度
    void *rBuff;                   //读入缓存
    WL_SIZE_T rMove;               //读入偏移
    WL_UART_LLPReadState rState;   //读入状态
    WL_UART_LLPHookFunction rHook; //读取回调

    WL_SIZE_T wSize;              //输出数据长度
    void *wBuff;                  //输出缓存
    WL_SIZE_T wMove;              //输出偏移
    WL_UART_LLPWriteState wState; //输出状态
} WL_UART_LLP;
