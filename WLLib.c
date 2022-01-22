/*
 * WLink状态处理
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLLib.h"
#include "WLMap.h"

#define WL_UART_DEFAULTTIMEOUT 1000 //默认发送超时

static WL_Map *WL_StateList = WL_NULL;
static WL_Map *WL_ReadBuffers = WL_NULL;
static WL_Map *WL_WriteBuffers = WL_NULL;

static void __WL_UART_Timeout(xTimerHandle pxTimer)
{
    WL_UINT32 id = pvTimerGetTimerID(pxTimer);
    WL_UART_State **state = WL_Map_Find(WL_StateList, &id);
    if (state == WL_NULL)
    {
        return;
    }

    (*state)->timer = WL_NULL; //定时器已自动销毁，此时timer悬空，应置为空

    switch ((*state)->sendState)
    {
    case WL_UART_SendStateConnected0:
    {
        (*state)->sendState = WL_UART_SendStateEmpty;
        break;
    }
    case WL_UART_SendStateBusy:
    {
        (*state)->sendState = WL_UART_SendStateConnected1;
        break;
    }
    }

    if ((*state)->readErrorFunc != WL_NULL)
    {
        (*state)->readErrorFunc(*state);
    }
}

void __WL_UART_ResetTimer(WL_UART_State *state)
{
    if (state->timer != WL_NULL)
    {
        xTimerDelete(state->timer, state->timeout);
        state->timer = WL_NULL;
    }
    state->timer = xTimerCreate("", state->timeout, pdFALSE, (void *)(state->id), __WL_UART_Timeout);
}

void _WL_UART_ReadCallback(UART_HandleTypeDef *huart)
{
    if (WL_ReadBuffers == WL_NULL)
    {
        return;
    }
    void **buffer = WL_Map_Find(WL_ReadBuffers, &(huart->Instance));
    if (buffer == WL_NULL)
    {
        return;
    }

    if (*((WL_UINT32 *)(*buffer)) != WL_UART_MagicNumber || *((WL_UINT32 *)((*buffer) + WL_UART_BufferSize - 32)) != WL_UART_MagicNumber)
    {
        return;
    } //头尾校验
    switch (*((WL_UINT32 *)((*buffer) + 32)))
    {
    case WL_UART_Head1:
    {
        WL_UINT32 peerID = *((WL_UINT32 *)((*buffer) + 96)); //获取接口ID
        for (WL_UINT32 i = 0; i < WL_StateList->size; i++)
        {
            WL_UART_State **current = WL_StateList->values + (i * (WL_StateList->valueSize));
            if ((*current)->id == peerID)
            {
                if ((*current)->sendState == WL_UART_SendStateEmpty)
                {
                    __WL_UART_ResetTimer((*current));
                    (*current)->peerID = *((WL_UINT32 *)((*buffer) + 64)); //远程接口匹配
                    void *writeBuff = *((void **)WL_Map_Find(WL_WriteBuffers, &(huart->Instance)));
                    memcpy(writeBuff, (*buffer), WL_UART_BufferSize);
                    memcpy(writeBuff + 64, *((WL_UINT32 *)((*buffer) + 96)), 32); // peerID->id
                    memcpy(writeBuff + 96, *((WL_UINT32 *)((*buffer) + 64)), 32); // id->peerID
                    *((WL_UINT32 *)(writeBuff + 32)) = WL_UART_Head2;             //握手返回操作头
                    if (HAL_UART_Transmit(huart, writeBuff, WL_UART_BufferSize, (*current)->timeout) != HAL_OK)
                    {
                        break;
                    } //发送
                    (*current)->sendState = WL_UART_SendStateConnected1;
                    HAL_UART_Receive_IT((*current)->handle, *((void **)WL_Map_Find(WL_ReadBuffers, &(huart->Instance))), WL_UART_BufferSize);
                } //判断接口状态合适
                break;
            }
        } //查找对应接口
        break;
    }
    case WL_UART_Head2:
    {
        WL_UINT32 peerID = *((WL_UINT32 *)((*buffer) + 96)); //获取接口ID
        for (WL_UINT32 i = 0; i < WL_StateList->size; i++)
        {
            WL_UART_State **current = WL_StateList->values + (i * (WL_StateList->valueSize));
            if ((*current)->id == peerID)
            {
                if ((*current)->sendState == WL_UART_SendStateConnected0)
                {
                    __WL_UART_ResetTimer((*current));
                    if ((*current)->peerID == *((WL_UINT32 *)((*buffer) + 64)))
                    {
                        (*current)->sendState = WL_UART_SendStateConnected1;
                        HAL_UART_Receive_IT((*current)->handle, *((void **)WL_Map_Find(WL_ReadBuffers, &(huart->Instance))), WL_UART_BufferSize);
                    } //远程接口匹配
                    else
                    {
                        (*current)->sendState = WL_UART_SendStateEmpty;
                    } //远程接口不匹配
                }     //判断接口状态合适
                break;
            }
        } //查找对应接口
        break;
    }
    case WL_UART_Head3:
    {
        break;
    }
    case WL_UART_Head4:
    {
        break;
    }
    } //功能判断
}

WL_UART_State *WL_UART_Create(UART_HandleTypeDef *uHandleType, WL_UINT32 id, WL_UINT32 peerID, WL_UART_HookFunction readFunc, WL_UART_HookFunction readErrorFunc)
{
    if (uHandleType == WL_NULL)
    {
        return WL_NULL;
    }

    if (WL_StateList == WL_NULL)
    {
        WL_StateList = WL_Map_Create(sizeof(WL_UINT32), sizeof(WL_UART_State *));
        WL_ReadBuffers = WL_Map_Create(sizeof(USART_TypeDef *), sizeof(void *));
        WL_WriteBuffers = WL_Map_Create(sizeof(USART_TypeDef *), sizeof(void *));
    } //如果map不存在则建立

    if (WL_Map_Find(WL_StateList, &id) != WL_NULL)
    {
        return WL_NULL;
    } // id防重

    WL_UART_State *state = pvPortMalloc(sizeof(WL_UART_State)); //分配内存
    state->handle = uHandleType;
    state->sendState = WL_UART_SendStateEmpty;
    state->timeout = WL_UART_DEFAULTTIMEOUT;
    state->id = id;
    state->peerID = peerID;
    state->timer = WL_NULL;
    state->readFunc = readFunc;
    state->readErrorFunc = readErrorFunc;
    state->writeFinishFunc = WL_NULL;
    state->writeErrorFunc = WL_NULL;

    if (WL_Map_Find(WL_ReadBuffers, &(state->handle->Instance)) == WL_NULL)
    {
        WL_Map_Insert(WL_ReadBuffers, &(state->handle->Instance), pvPortMalloc(WL_UART_BufferSize));
    }
    if (WL_Map_Find(WL_WriteBuffers, &(state->handle->Instance)) == WL_NULL)
    {
        WL_Map_Insert(WL_WriteBuffers, &(state->handle->Instance), pvPortMalloc(WL_UART_BufferSize));
    } //若无串口缓存，则建立

    WL_Map_Insert(WL_StateList, &(state->id), &state); // state存入map

    HAL_UART_Receive_IT(state->handle, *((void **)WL_Map_Find(WL_ReadBuffers, &(state->handle->Instance))), WL_UART_BufferSize);//等待连接

    return state;
}

void WL_UART_Destory(WL_UART_State *state)
{
    if (state == WL_NULL)
    {
        return;
    }
    if (state->timer != WL_NULL)
    {
        xTimerDelete(state->timer, state->timeout);
        state->timer = WL_NULL;
    } //终止计时器

    WL_Map_Erase(WL_StateList, &(state->id)); //从map中移除

    DMA_Stream_TypeDef *readPort = state->handle->Instance;
    DMA_Stream_TypeDef *writePort = state->handle->Instance; //获取串口地址

    WL_BOOLEAN haveRead = WL_FALSE;
    WL_BOOLEAN haveWrite = WL_FALSE;
    for (WL_UINT32 i = 0; i < WL_StateList->size; i++)
    {
        WL_UART_State *current = WL_StateList->values + (i * (WL_StateList->valueSize));
        if (current->handle->Instance == readPort)
        {
            haveRead = WL_TRUE;
        }
        if (current->handle->Instance == writePort)
        {
            haveWrite = WL_TRUE;
        }
    } //判断是否有串口占用
    if (!haveRead)
    {
        vPortFree(*((void **)WL_Map_Find(WL_ReadBuffers, &readPort)));
        WL_Map_Erase(WL_ReadBuffers, &readPort);
    }
    if (!haveWrite)
    {
        vPortFree(*((void **)WL_Map_Find(WL_WriteBuffers, &writePort)));
        WL_Map_Erase(WL_WriteBuffers, &writePort);
    } //若无占用清理串口缓存

    vPortFree(state);

    if (WL_Map_Size(WL_StateList) == 0)
    {
        WL_Map_Destory(WL_ReadBuffers);
        WL_ReadBuffers = WL_NULL;
        WL_Map_Destory(WL_WriteBuffers);
        WL_WriteBuffers = WL_NULL;
        WL_Map_Destory(WL_StateList);
        WL_StateList = WL_NULL;
    } //若map为空则销毁
}

WL_BOOLEAN WL_UART_Connect(WL_UART_State *state)
{
    if (state == WL_NULL)
    {
        return WL_FALSE;
    }
    if (state->sendState != WL_UART_SendStateEmpty)
    {
        return WL_FALSE;
    }

    __WL_UART_ResetTimer(state);
    void **buffer = WL_Map_Find(WL_ReadBuffers, &(state->handle->Instance)); //获取写缓冲区
    if (buffer == WL_NULL)
    {
        return WL_FALSE;
    }

    *((WL_UINT32 *)(*buffer)) = WL_UART_MagicNumber;                             //包头
    *((WL_UINT32 *)((*buffer) + WL_UART_BufferSize - 32)) = WL_UART_MagicNumber; //包尾
    *((WL_UINT32 *)((*buffer) + 32)) = WL_UART_Head1;                            //握手请求操作头
    *((WL_UINT32 *)((*buffer) + 64)) = state->id;
    *((WL_UINT32 *)((*buffer) + 96)) = state->peerID; //包构建

    state->sendState = WL_UART_SendStateConnected0;

    HAL_UART_Receive_IT(state->handle, *((void **)WL_Map_Find(WL_ReadBuffers, &(state->handle->Instance))), WL_UART_BufferSize);

    if (HAL_UART_Transmit(state->handle, (*buffer), WL_UART_BufferSize, state->timeout) != HAL_OK)
    {
        state->sendState = WL_UART_SendStateEmpty;
        return WL_FALSE;
    } //发送

    xTimerStart(state->timer, state->timeout);

    while (state->sendState == WL_UART_SendStateConnected0)
    {
    }; //等待

    if (state->sendState == WL_UART_SendStateConnected1)
    {
        return WL_TRUE;
    }
    return WL_FALSE;
}

WL_BOOLEAN WL_UART_SetTimeOut(WL_UART_State *state, WL_TIMETICK timeout)
{
    if (state == WL_NULL)
    {
        return WL_FALSE;
    }
    if (state->sendState != WL_UART_SendStateEmpty)
    {
        return WL_FALSE;
    }
    state->timeout = timeout;
    return WL_TRUE;
}

WL_BOOLEAN WL_UART_Write(WL_UART_State *state, void *data, WL_SIZE_T size, WL_UART_HookFunction finishFunc, WL_UART_HookFunction errorFunc)
{
    if (state == WL_NULL)
    {
        return WL_FALSE;
    }
    if (data == WL_NULL)
    {
        return WL_FALSE;
    }
    if (state->sendState != WL_UART_SendStateEmpty)
    {
        return WL_FALSE;
    }
}
