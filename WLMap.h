#pragma once
/*
 * WLink键值映射表
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLBase.h"

typedef struct
{
    WL_UINT32 size;      //当前元素数量
    void *keys;          //键
    void *values;        //值
    WL_SIZE_T keySize;   //单个键的大小
    WL_SIZE_T valueSize; //单个值的大小
} WL_Map;

WL_Map *WL_Map_Create(WL_SIZE_T, WL_SIZE_T);
void WL_Map_Destory(WL_Map *);

void WL_Map_Insert(WL_Map *, void *, void *); //插入
void *WL_Map_Find(WL_Map *, void *);          //查找
void WL_Map_Erase(WL_Map *, void *);          //移除

WL_UINT32 WL_Map_Size(WL_Map *);