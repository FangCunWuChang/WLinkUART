/*
 * WLink键值映射表
 * (C)2022 WuChang & RM Team WDR.All Rights Reserved.
 */

#include "WLMap.h"
#include "WLInclude.h"

WL_Map *WL_Map_Create(WL_SIZE_T keySize, WL_SIZE_T valueSize)
{
    WL_Map *map = pvPortMalloc(sizeof(WL_Map));
    map->size = 0;
    map->keySize = keySize;
    map->valueSize = valueSize;
    map->keys = WL_NULL;
    map->values = WL_NULL;
    return map;
}

void WL_Map_Destory(WL_Map *map)
{
    if (map == WL_NULL)
    {
        return;
    }
    if (map->size > 0)
    {
        vPortFree(map->keys);
        vPortFree(map->values);
        map->keys = WL_NULL;
        map->values = WL_NULL;
    }
    vPortFree(map);
}

void WL_Map_Insert(WL_Map *map, void *key, void *value)
{
    if (map == WL_NULL)
    {
        return;
    }
    if (key == WL_NULL || value == WL_NULL)
    {
        return;
    }

    for (WL_UINT32 i = 0; i < map->size; i++)
    {
        if (memcmp(key, map->keys + (map->keySize) * i, map->keySize) == 0)
        {
            return;
        }
    } //判断重复

    void *newKeys = pvPortMalloc((map->keySize) * (map->size + 1));
    void *newValues = pvPortMalloc((map->valueSize) * (map->size + 1)); //新空间

    if (map->size > 0)
    {
        memcpy(newKeys, map->keys, (map->keySize) * (map->size));
        memcpy(newValues, map->values, (map->valueSize) * (map->size));
        vPortFree(map->keys);
        vPortFree(map->values);
        map->keys = WL_NULL;
        map->values = WL_NULL;
    } //如果原来有数据，则复制原来数据并释放原空间

    memcpy(newKeys + (map->keySize) * (map->size), key, map->keySize);
    memcpy(newValues + (map->valueSize) * (map->size), value, map->valueSize); //数据写入新空间

    map->keys = newKeys;
    map->values = newValues;
    map->size++; //将新空间记录
}

void *WL_Map_Find(WL_Map *map, void *key)
{
    if (map == WL_NULL)
    {
        return;
    }
    if (key == WL_NULL)
    {
        return;
    }

    for (WL_UINT32 i = 0; i < map->size; i++)
    {
        if (memcmp(key, map->keys + (map->keySize) * i, map->keySize) == 0)
        {
            return map->values + (map->valueSize) * i;
        }
    }
    return WL_NULL;
}

void WL_Map_Erase(WL_Map *map, void *key)
{
    if (map == WL_NULL)
    {
        return;
    }
    if (key == WL_NULL)
    {
        return;
    }

    WL_UINT32 index = map->size;
    for (WL_UINT32 i = 0; i < map->size; i++)
    {
        if (memcmp(key, map->keys + (map->keySize) * i, map->keySize) == 0)
        {
            index = i;
            break;
        }
    } //查找并记录位置
    if (index >= map->size)
    {
        return;
    } //如果未找到

    void *newKeys = WL_NULL;
    void *newValues = WL_NULL;
    if (map->size > 1)
    {
        newKeys = pvPortMalloc((map->keySize) * (map->size - 1));
        newValues = pvPortMalloc((map->valueSize) * (map->size - 1));
        if (index > 0)
        {
            memcpy(newKeys, map->keys, (map->keySize) * index);
            memcpy(newValues, map->values, (map->valueSize) * index);
        }
        if (index < map->size - 1)
        {
            memcpy(newKeys + (map->keySize) * index, map->keys + (map->keySize) * (index + 1), (map->keySize) * (map->size - index - 1));
            memcpy(newValues + (map->valueSize) * index, map->values + (map->valueSize) * (index + 1), (map->valueSize) * (map->size - index - 1));
        }
    } //如果删除后不为空，则分配新空间并复制内容

    vPortFree(map->keys);
    vPortFree(map->values); //清理原空间

    map->keys = newKeys;
    map->values = newValues;
    map->size--; //记录更改
}

WL_UINT32 WL_Map_Size(WL_Map *map)
{
    if (map == WL_NULL)
    {
        return 0;
    }
    return map->size;
}
