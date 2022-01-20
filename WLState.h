/*
 * WLink状态
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLInclude.h"
#include "WLBase.h"

typedef enum
{
    WL_UART_StateUART = 0,
    WL_UART_StateUSART = 1
} WL_UART_StateType;

typedef enum
{
    WL_UART_SendStateNormal = 0,
    WL_UART_SendStateBusy = 1
} WL_UART_SendState;

typedef union
{
    UART_HandleTypeDef *uartHandle;   // HAL UART串口句柄
    USART_HandleTypeDef *usartHandle; // HAL USART串口句柄
} WL_UART_Handle;

typedef struct
{
    WL_UINT64 id;                //索引
    WL_UART_StateType type;      //串口模式UART/UASRT
    WL_UART_Handle handle;       //串口句柄
    WL_TIMETICK timeout;         //发送超时
    WL_UART_SendState sendState; //发送状态
    TimerHandle_t *timer;        //定时器句柄
} WL_UART_State;                 // WLink状态