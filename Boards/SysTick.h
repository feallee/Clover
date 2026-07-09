/**
 * @file SysTick.h
 * @copyright Copyright (c) 2026 weas.top team. All rights reserved.
 * @brief SysTick 设备驱动接口。
 *
 * 提供类 Unix 设备接口操作 SysTick 硬件定时器，支持打开/关闭、
 * 滴答回调注册、读写计数寄存器等操作。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者                | 日志                                            |
 * | ---- | ---------- | ------------------- | ----------------------------------------------- |
 * |      | 2026/07/09 | feallee@hotmail.com | 统一接口返回类型为 SysTick_ErrorType。           |
 * |      | 2026/07/09 | feallee@hotmail.com | 初版。                                          |
 */
#pragma once
#include <stddef.h>
#include <inttypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief SysTick 操作返回的返回错误码。
     */
    typedef enum
    {
        SYSTICK_ERROR_NONE = 0,   /**< 操作成功 */
        SYSTICK_ERROR_NULL = -1,  /**< 参数为空 */
        SYSTICK_ERROR_RANGE = -2, /**< 值越界 */
    } SysTick_ErrorType;

    /**
     * @brief SysTick 设备结构体（不透明类型，仅通过指针操作）。
     */
    typedef struct _SysTick_DeviceType SysTick_DeviceType;

    /**
     * @brief 滴答回调函数类型。
     * @param device 触发滴答的设备指针。
     */
    typedef void (*SysTick_TickedType)(SysTick_DeviceType *device);

    /**
     * @brief 打开 SysTick 设备。
     *
     * 设备为单例，若已通过相同 name 打开则直接返回现有实例。
     * 若已打开但 mode 不一致则拒绝返回。
     *
     * @param name 设备名称，必须与设备实例名称一致。
     * @param mode 滴答周期："1ms"、"10ms" 或 "100ms"，非法值默认使用 "1ms"。
     * @return 设备指针。
     * @retval 非NULL 成功。
     * @retval NULL 失败（参数为空、名称不匹配、mode 冲突或硬件配置失败）。
     */
    SysTick_DeviceType *SysTick_Open(const char *name, const char *mode);

    /**
     * @brief 关闭 SysTick 设备并释放硬件资源。
     *
     * 清零 CTRL/LOAD/VAL 寄存器以降低功耗，并重置设备状态。
     *
     * @param device 设备指针。
     * @return 返回错误码。
     * @retval SYSTICK_ERROR_NONE 成功。
     * @retval SYSTICK_ERROR_NULL device 为空。
     */
    SysTick_ErrorType SysTick_Close(SysTick_DeviceType *device);

    /**
     * @brief 设置滴答回调函数。
     *
     * 注册一个每次滴答中断时调用的回调函数。传入 NULL 可取消回调。
     *
     * @param device 设备指针。
     * @param ticked 回调函数指针，NULL 表示取消。
     * @return 返回错误码。
     * @retval SYSTICK_ERROR_NONE 成功。
     * @retval SYSTICK_ERROR_NULL device 为空。
     */
    SysTick_ErrorType SysTick_SetTicked(SysTick_DeviceType *device, SysTick_TickedType ticked);

    /**
     * @brief 读取 SysTick 当前计数值。
     *
     * 读取 SysTick->VAL（24 位递减计数器当前值）并通过 value 传出。
     *
     * @param device 设备指针。
     * @param[out] value 传出当前计数寄存器值。
     * @return 返回错误码。
     * @retval SYSTICK_ERROR_NONE 成功。
     * @retval SYSTICK_ERROR_NULL 参数为空或设备未打开。
     */
    SysTick_ErrorType SysTick_Read(SysTick_DeviceType *device, uint32_t *value);

    /**
     * @brief 写入 SysTick 重载寄存器。
     *
     * 直接设置 SysTick->LOAD（24 位重载值），写入后自动清空 VAL。
     *
     * @param device 设备指针。
     * @param value 重载值（24 位，即 SystemCoreClock / 频率 - 1）。
     * @return 返回错误码。
     * @retval SYSTICK_ERROR_NONE 成功。
     * @retval SYSTICK_ERROR_NULL device 为空或设备未打开。
     * @retval SYSTICK_ERROR_RANGE value 越界。
     */
    SysTick_ErrorType SysTick_Write(SysTick_DeviceType *device, uint32_t value);

    /**
     * @brief 获取 SysTick 最大重载值。
     *
     * 通过 value 传出 SysTick_LOAD_RELOAD_Msk（0xFFFFFF）。
     *
     * @param device 设备指针。
     * @param[out] value 传出最大重载值。
     * @return 返回错误码。
     * @retval SYSTICK_ERROR_NONE 成功。
     * @retval SYSTICK_ERROR_NULL value 为空。
     */
    SysTick_ErrorType SysTick_GetMaxValue(SysTick_DeviceType *device, uint32_t *value);

#ifdef __cplusplus
}
#endif
