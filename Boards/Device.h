/**
 * @file Device.h
 * @brief 统一的设备抽象层，为用户提供设备的注册与访问接口。
 *
 * 用户通过 DEVICE_REGISTER 把设备注册到由链接器收集的设备表中，
 * 再按名称查找设备，打开之后使用统一的读、写、控制接口访问设备。
 *
 * @note 注意事项:
 * - MDK 编译器不要求显式定义设备段。
 * - GCC 编译器需要在链接脚本显式定义设备段 <DEVICE_LINK_SYMBOL>.* 段（形如 Device.<level>，
 *   无前导 "."）并排序，如:
 *
 *   //... other sections
 *    KEEP(*(SORT(Device.*)))
 *   //... other sections
 *
 * - 其它编译器，根据具体要求进行配置。
 *
 * @copyright Copyright (C) 2026 weas.top team. All rights reserved.
 *
 * @warning 本库只支持 C99 及以上标准。
 *
 * @note 依赖项:
 *       - 链接脚本: GCC 编译器需配置 Device.* 段（无前导 "."）。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者                | 日志                                                                            |
 * | ---- | ---------- | ------------------- | ------------------------------------------------------------------------------- |
 * |      | 2026/10/20 | feallee@hotmail.com | 实现设备表遍历与 DEVICE_REGISTER 注册，修正设备段链接脚本注释。                 |
 */
#pragma once
#include <inttypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

#if 1 /* 错误码 */
/**
 * 操作成功，无错误。
 */
#define DEVICE_ERROR_NONE (0)
/**
 * 资源为 NULL。
 */
#define DEVICE_ERROR_NULL (-1)
/**
 * 资源超出有效范围。
 */
#define DEVICE_ERROR_RANGE (-2)
/**
 * 资源为空。
 */
#define DEVICE_ERROR_EMPTY (-3)
/**
 * 资源已满。
 */
#define DEVICE_ERROR_FULL (-4)

#define DEVICE_ERROR_BUSY (-5)
#define DEVICE_ERROR_TIMEOUT (-6)
#define DEVICE_ERROR_NOSUPPORT (-7)

#endif

#if 1 /* 命令码*/

#define DEVICE_COMMAND_ERASE UINT32_C(0x80000000)
#endif

#define DEVICE_LINK_SYMBOL Device

    typedef struct
    {
        const char *Name;
        int32_t (*Open)(const void *parameter);
        int32_t (*Close)(void);
        int32_t (*Read)(void *buffer, uint32_t length, const void *parameter);
        int32_t (*Write)(const void *buffer, uint32_t length, const void *parameter);
        int32_t (*Control)(uint32_t command, const void *parameter);
    } DeviceType;

    DeviceType *Device_Find(const char *name);
    int32_t Device_Open(DeviceType *device, const void *parameter);
    int32_t Device_Close(DeviceType *device);
    int32_t Device_Read(DeviceType *device, void *buffer, uint32_t length, const void *parameter);
    int32_t Device_Write(DeviceType *device, const void *buffer, uint32_t length, const void *parameter);
    int32_t Device_Control(DeviceType *device, uint32_t command, const void *parameter);

#if 1 /* 内部使用宏，禁止外部使用 */

#define _DEVICE_STRING(a) #a
#define _DEVICE_CONCAT_STRING(a, b) _DEVICE_STRING(a.b)
#define _DEVICE_TO_SECTION(level) _DEVICE_CONCAT_STRING(DEVICE_LINK_SYMBOL, level)

#define _DEVICE_SYMBOL(a, b, c) _##a##_##b##_##c
#define _DEVICE_CONCAT_SYMBOL(a, b, c) _DEVICE_SYMBOL(a, b, c)
#define _DEVICE_TO_MEMBER(name, level) _DEVICE_CONCAT_SYMBOL(DEVICE_LINK_SYMBOL, name, level)

#define _DEVICE_REGISTER(name, open, close, read, write, control, level)                                               \
    static const DeviceType _DEVICE_TO_MEMBER(name, level) __attribute__((used, section(_DEVICE_TO_SECTION(level)))) = \
        {.Name = _DEVICE_STRING(name), .Open = open, .Close = close, .Read = read, .Write = write, .Control = control}
#endif
#define DEVICE_REGISTER(name, open, close, read, write, control) _DEVICE_REGISTER(name, open, close, read, write, control, 8)
#ifdef __cplusplus
}
#endif