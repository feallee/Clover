#include <string.h>
#include "Device.h"

/**
 * @brief 设备表的哨兵级别。
 *
 * 内部使用 L0 和 L9 标记设备表的边界，用户注册的设备落在 L1-L8
 * （DEVICE_REGISTER 默认注册到 L8）。链接器按段名排序后，两个哨兵
 * 之间的对象即为全部已注册设备。
 */
#define _DEVICE_LEVEL_BEGIN 0
#define _DEVICE_LEVEL_END 9

_DEVICE_REGISTER(begin, 0, 0, 0, 0, 0, _DEVICE_LEVEL_BEGIN);
_DEVICE_REGISTER(end, 0, 0, 0, 0, 0, _DEVICE_LEVEL_END);

/**
 * @brief 设备表结构。
 *
 * 由链接器收集的哨兵对象界定设备表范围。
 */
typedef struct
{
    const DeviceType *Begin; ///< 指向 begin 哨兵，其后第一个对象才是真实设备
    const DeviceType *End;   ///< 指向 end 哨兵，本身不属于设备表
} DeviceTableType;

/**
 * @brief 设备表实例。
 */
static const DeviceTableType _Table = {
    .Begin = &_DEVICE_TO_MEMBER(begin, _DEVICE_LEVEL_BEGIN),
    .End = &_DEVICE_TO_MEMBER(end, _DEVICE_LEVEL_END),
};

DeviceType *Device_Find(const char *name)
{
    if (name == NULL)
    {
        return NULL;
    }

    /* begin + 1 跳过 begin 哨兵，d < end 跳过 end 哨兵 */
    for (const DeviceType *d = _Table.Begin + 1; d < _Table.End; d++)
    {
        if (strcmp(d->Name, name) == 0)
        {
            return (DeviceType *)d;
        }
    }

    return NULL;
}

int32_t Device_Open(DeviceType *device, const void *parameter)
{
    if (device == NULL)
    {
        return DEVICE_ERROR_NULL;
    }

    if (device->Open == NULL)
    {
        return DEVICE_ERROR_NOSUPPORT;
    }

    return device->Open(parameter);
}

int32_t Device_Close(DeviceType *device)
{
    if (device == NULL)
    {
        return DEVICE_ERROR_NULL;
    }

    if (device->Close == NULL)
    {
        return DEVICE_ERROR_NOSUPPORT;
    }

    return device->Close();
}

int32_t Device_Read(DeviceType *device, void *buffer, uint32_t length, const void *parameter)
{
    if (device == NULL)
    {
        return DEVICE_ERROR_NULL;
    }

    if (device->Read == NULL)
    {
        return DEVICE_ERROR_NOSUPPORT;
    }

    return device->Read(buffer, length, parameter);
}

int32_t Device_Write(DeviceType *device, const void *buffer, uint32_t length, const void *parameter)
{
    if (device == NULL)
    {
        return DEVICE_ERROR_NULL;
    }

    if (device->Write == NULL)
    {
        return DEVICE_ERROR_NOSUPPORT;
    }

    return device->Write(buffer, length, parameter);
}

int32_t Device_Control(DeviceType *device, uint32_t command, const void *parameter)
{
    if (device == NULL)
    {
        return DEVICE_ERROR_NULL;
    }

    if (device->Control == NULL)
    {
        return DEVICE_ERROR_NOSUPPORT;
    }

    return device->Control(command, parameter);
}
