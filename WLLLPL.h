#pragma once
/*
 * WLink底层端口库
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLLLP.h"

WL_UART_LLP *WL_UART_LLPCreate(UART_HandleTypeDef *);
void WL_UART_LLPDestory(WL_UART_LLP *);

void WL_UART_LLPRead(WL_UART_LLP *, void *, WL_SIZE_T, WL_UART_LLPHookFunction);
void WL_UART_LLPWrite(WL_UART_LLP *, void *, WL_SIZE_T);

void _WL_UART_LLPEvent();                        //推动发送
void _WL_UART_LLPCallBack(UART_HandleTypeDef *); //读取回调

void _WL_UART_LLPEventTask(void *);
