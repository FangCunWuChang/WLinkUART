/*
 * WLink底层端口库
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLLLPL.h"
#include "WLMap.h"

static void __WL_UART_LLPLock(WL_UART_LLP *port)
{
    if (port == WL_NULL)
    {
        return;
    }

    //while (port->mutex)
       ;
    port->mutex = WL_TRUE;
}

static void __WL_UART_LLPUnlock(WL_UART_LLP *port)
{
    if (port == WL_NULL)
    {
        return;
    }

    port->mutex = WL_FALSE;
}

static WL_Map *WL_UART_LLPList = WL_NULL;
static WL_BOOLEAN WL_UART_LLPList_Mutex = WL_FALSE;

static void __WL_UART_LLPList_Mutex_Lock()
{
    //while (WL_UART_LLPList_Mutex);
    WL_UART_LLPList_Mutex = WL_TRUE;
}

static void __WL_UART_LLPList_Mutex_Unlock()
{
    WL_UART_LLPList_Mutex = WL_FALSE;
}

WL_UART_LLP *WL_UART_LLPCreate(UART_HandleTypeDef *huart)
{
    if (huart == WL_NULL)
    {
        return WL_NULL;
    }
    __WL_UART_LLPList_Mutex_Lock();
    if (WL_UART_LLPList == WL_NULL)
    {
        WL_UART_LLPList = WL_Map_Create(sizeof(USART_TypeDef *), sizeof(WL_UART_LLP *));
    }
    if (WL_Map_Find(WL_UART_LLPList, &(huart->Instance)) == WL_NULL)
    {
        WL_UART_LLP *llp = pvPortMalloc(sizeof(WL_UART_LLP));
        llp->handle = huart;
        llp->count = 0;
        llp->buffer = 0;
        llp->mutex = WL_FALSE;
        llp->rSize = 0;
        llp->rBuff = WL_NULL;
        llp->rMove = 0;
        llp->rState = WL_UART_LLPReadEmpty;
        llp->rHook = WL_NULL;
        llp->wSize = 0;
        llp->wBuff = WL_NULL;
        llp->wMove = 0;
        llp->wState = WL_UART_LLPWriteEmpty;

        WL_Map_Insert(WL_UART_LLPList, &(huart->Instance), &(llp));
        __WL_UART_LLPList_Mutex_Unlock();
        return llp;
    }
    else
    {
        WL_UART_LLP **llp = WL_Map_Find(WL_UART_LLPList, &(huart->Instance));
        (*llp)->count++;
        __WL_UART_LLPList_Mutex_Unlock();
        return (*llp);
    }
}

void WL_UART_LLPDestory(WL_UART_LLP *llp)
{
    if (llp == WL_NULL)
    {
        return;
    }


    if (llp->count > 0)
    {
        llp->count--;
        return;
    }

    __WL_UART_LLPList_Mutex_Lock();

    WL_Map_Erase(WL_UART_LLPList, &(llp->handle->Instance));

    vPortFree(llp);

    if (WL_Map_Size(WL_UART_LLPList) == 0)
    {
        WL_Map_Destory(WL_UART_LLPList);
        WL_UART_LLPList = WL_NULL;
    }

    __WL_UART_LLPList_Mutex_Unlock();
}

void WL_UART_LLPRead(WL_UART_LLP *llp, void *buffer, WL_SIZE_T size, WL_UART_LLPHookFunction hookFunc)
{
    if (llp == WL_NULL)
    {
        return;
    }
    __WL_UART_LLPLock(llp);
    if (size == 0)
    {
        llp->buffer = 0;
        llp->rSize = 0;
        llp->rBuff = WL_NULL;
        llp->rMove = 0;
        llp->rState = WL_UART_LLPReadEmpty;
        llp->rHook = WL_NULL;
    }
    else
    {
        llp->rState = WL_UART_LLPReadReady;
        llp->rBuff = buffer;
        llp->rSize = size;
        llp->rMove = 0;
        llp->rHook = hookFunc;

        HAL_UART_Receive_IT(llp->handle, &(llp->buffer), 1);
    }
    __WL_UART_LLPUnlock(llp);
}

void WL_UART_LLPWrite(WL_UART_LLP *llp, void *buffer, WL_SIZE_T size)
{
    if (llp == WL_NULL)
    {
        return;
    }

    __WL_UART_LLPLock(llp);
    if (size == 0)
    {
        llp->wSize = 0;
        llp->wBuff = WL_NULL;
        llp->wMove = 0;
        llp->wState = WL_UART_LLPWriteEmpty;
    }
    else
    {
        llp->wState = WL_UART_LLPWriteBusy;
        llp->wBuff = buffer;
        llp->wSize = size;
        llp->wMove = 0;
    }
    __WL_UART_LLPUnlock(llp);
}

void _WL_UART_LLPEvent()
{
    if (WL_UART_LLPList != WL_NULL)
    {
    	__WL_UART_LLPList_Mutex_Lock();
        for (WL_UINT32 i = 0; i < WL_UART_LLPList->size; i++)
        {
            WL_UART_LLP **llp = (WL_UART_LLP **)((WL_UINT32)WL_UART_LLPList->values + (i * WL_UART_LLPList->valueSize));
            __WL_UART_LLPLock((*llp));
            if ((*llp)->wState == WL_UART_LLPWriteBusy)
            {
                if (HAL_UART_Transmit_IT((*llp)->handle, (void *)((WL_UINT32)(*llp)->wBuff + (*llp)->wMove), 1) == HAL_OK)
                {
                    (*llp)->wMove++;
                    if ((*llp)->wMove == (*llp)->wSize)
                    {
                        (*llp)->wState = WL_UART_LLPWriteEmpty;
                        (*llp)->wBuff = WL_NULL;
                        (*llp)->wSize = 0;
                        (*llp)->wMove = 0;
                    }
                }
            }
            __WL_UART_LLPUnlock((*llp));
        }
        __WL_UART_LLPList_Mutex_Unlock();
    }
}

void _WL_UART_LLPCallBack(UART_HandleTypeDef *huart)
{
    if (huart == WL_NULL)
    {
        return;
    }

    __WL_UART_LLPList_Mutex_Lock();
    WL_UART_LLP **llp = WL_Map_Find(WL_UART_LLPList, &(huart->Instance));
    __WL_UART_LLPList_Mutex_Unlock();

    if (llp == WL_NULL)
    {
        return;
    }

    WL_UART_LLPHookFunction Func = WL_NULL;

    __WL_UART_LLPLock((*llp));
    if ((*llp)->rState == WL_UART_LLPReadReady || (*llp)->rState == WL_UART_LLPReadBusy)
    {
        (*llp)->rState = WL_UART_LLPReadBusy;
        memcpy((void *)((WL_UINT32)(*llp)->rBuff + (*llp)->rMove), &((*llp)->buffer), 1);
        (*llp)->rMove++;
        if ((*llp)->rMove == (*llp)->rSize)
        {
            Func = (*llp)->rHook;
            (*llp)->buffer = 0;
            (*llp)->rSize = 0;
            (*llp)->rBuff = WL_NULL;
            (*llp)->rMove = 0;
            (*llp)->rState = WL_UART_LLPReadEmpty;
            (*llp)->rHook = WL_NULL;
        }
        else
        {
            HAL_UART_Receive_IT(huart, &((*llp)->buffer), 1);
        }
    }
    __WL_UART_LLPUnlock((*llp));
    if (Func != WL_NULL)
    {
        Func((*llp));
    }
}

void _WL_UART_LLPEventTask(void *argument)
{
    UNUSED(argument);
    for (;;)
    {
        _WL_UART_LLPEvent();
    }
}
