/**
 * @file Application.h
 * @copyright Copyright (c) 2026 weas.top team. All rights reserved.
 * @brief 消息驱动型的裸机应用程序框架，推荐在应用程序中把消息处理器当作状态机的事件发生器。
 *
 * 本框架基于消息队列实现的事件驱动型应用程序框架，支持:
 * - 异步投递消息。
 * - 同步发送消息。
 * - 消息队列监控：获取溢出次数、当前消息数、队列容量。
 * - 单条消息支持 8 个级别(L1-L8)的消息处理器，每个级别还支持任意多的消息处理器。
 * - 消息 ID 值域[0, 255]。但系统保留 [0, 7] 8 条用于系统初始化等，用户可以从 8 开始编制消息。
 *   如果消息 ID 不够使用，可以使用消息中其它字段 BParam、WParam、DParam 来扩展消息 ID，但需要在消息处理器中进行显式区分。如：
 * @code
 *    static void HandleKeyPress(Application_MessageType *msg) {
 *        if(msg->BParam == 0) {
 *           // do something
 *        } else if(msg->BParam == 1) {
 *           // do something else
 *        }
 *        else {
 *           // do default something
 *        }
 *    }
 * @endcode
 * 使用方法:
 * @code
 * 1. 在 main() 中调用 Application_Run() 启动主循环（只允许调用一次）:
 *    int main(void) {
 *        Application_Run(NULL);  // 不会返回
 *    }
 *
 * 2. 使用宏注册消息处理器（可在任意文件中注册）:
 *    static void HandleKeyPress(Application_MessageType *msg) {
 *        // 处理按键事件
 *    }
 *    APPLICATION_REG_MESSAGE_HANDLER_L1(1, HandleKeyPress);
 *
 * 3. 投递/发送消息:
 *    Application_MessageType msg = {1, 0, 0, 0};
 *    Application_PostMessage(&msg); // 异步投递
 *    // 或
 *    Application_SendMessage(&msg); // 同步发送
 * @endcode
 *
 * @note 注意事项:
 * - MDK 编译器不要求显式定义 Entry.* 段。
 * - GCC 编译器需要在链接脚本显式定义 Entry.* 段并排序，如:
 * .Entry :
 * {
 *   //... other sections
 *    KEEP(*(SORT(.Entry.*)))
 *   //... other sections
 * } > FLASH
 * - 其它编译器，根据具体要求进行配置。
 *
 * @warning 函数 Application_PostMessage 和 Application_SendMessage 虽然支持递归调用，但必须设计退出机制，
 *         否则引起死循环或栈溢出。所以不建议在消息处理器中再投递或发送消息。
 * @warning 本库只支持 C99 及以上标准。
 *
 * @note 依赖项:
 *       - Config.h: 必须定义 APPLICATION_MESSAGE_COUNT、APPLICATION_BASEPRI_PRIORITY 等宏。
 *       - 链接脚本: GCC 编译器需配置 Entry.* 段。
 *       - 硬件相关: 需配置 QUEUE_CRITICAL_ENTER/EXIT 宏。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者            | 日志                                                                                |
 * | ---- | :--------- | :-------------- | :-----------------------------------------------------------------------------------|
 * |      | 2026/03/06 | feallee@hotmail | 初版。                                                                              |
 *
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "Config.h"
#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief 应用消息类型。
     */
    typedef struct
    {
        uint8_t ID; ///< 消息 ID (0-255)。
        /*TODO 自行扩展字段*/
        uint8_t BParam; ///< 字节参数 (8位)
        uint16_t WParam; ///< 字参数 (16位)
    } Application_MessageType;

    /**
     * @brief 消息处理器类型
     *
     * @param message 指向消息的指针。
     */
    typedef void (*Application_MessageHandlerType)(Application_MessageType *message);

    /**
     * @brief 运行应用程序并处理消息循环。
     *
     * 执行消息处理循环，不断从消息队列中取出消息并调用相应的消息处理器。
     *
     * @param parameter 用户自定义参数（当前未使用，保留用于扩展）。
     *
     * @note 该函数包含无限循环，正常情况下不会返回。
     * @note 函数启动时会先同步发送 APPLICATION_MESSAGE_ID_INIT 消息进行初始化。
     * @note 当队列为空时，会调用 __WFI() 进入低功耗模式等待中断唤醒。
     */
    int Application_Run(void *parameter);

    /**
     * @brief 异步投递消息到队列。
     *
     * 将消息添加到消息队列尾部，由主循环异步处理。
     * 如果队列已满，则投递失败并增加溢出计数器。
     *
     * @param message 指向要投递的消息的指针，不允许为 NULL。
     * @return 返回投递消息结果。
     * @retval true 成功。
     * @retval false 队列已满或消息指针为 NULL。
     *
     * @note 该函数是线程/中断安全的，可以在中断中调用。
     * @note 使用 BASEPRI 临界区保护入队操作，保证原子性。
     * @note 投递失败时溢出计数器会自增，可通过 Application_GetMessageOverflow() 查询。
     */
    bool Application_PostMessage(Application_MessageType *message);

    /**
     * @brief 同步发送消息
     *
     * 立即调用该消息 ID 对应的所有处理器（L1-L8），不经过消息队列。
     * 处理器按级别顺序执行：L1 -> L2 -> ... -> L8，同一级别内的处理器按注册顺序执行。
     *
     * @param message 指向要发送的消息的指针，不允许为 NULL。
     * @return 返回发送消息结果。
     * @retval true 成功（没有找到消息处理器也会返回 true）。
     * @retval false 消息指针为 NULL 或未找到任何处理器。
     *
     * @note 该函数是同步的，会阻塞直到所有处理器执行完毕。
     * @note 支持递归调用（处理器中再次调用 Application_SendMessage），但必须设计退出机制。
     */
    bool Application_SendMessage(Application_MessageType *message);

    /**
     * @brief 获取消息溢出次数。
     * @return 返回消息溢出次数，即投递消息失败的次数（队列已满）。
     * @note 该函数使用临界区保护，保证读取的原子性。
     * @note 溢出计数器不会自动清零，如需重置需要自行实现。
     */
    uint32_t Application_GetMessageOverflow(void);

    /**
     * @brief 获取消息队列中的消息数量。
     * @return 返回当前消息队列中的消息数量（范围: 0 - APPLICATION_MESSAGE_COUNT）。
     * @note 该函数使用临界区保护，保证读取的原子性。计数公式基于 mirror bit 机制：
     *       当 WriteMirror == ReadMirror 时，表示 WriteIndex 在 ReadIndex 之后或相等，
     *       计数 = WriteIndex - ReadIndex；
     *       否则表示 WriteIndex 已回绕，计数 = CAPACITY - (ReadIndex - WriteIndex)。
     */
    uint32_t Application_GetMessageCount(void);

#if 1 /*宏校验*/
#ifndef APPLICATION_MESSAGE_COUNT
#error "Please define APPLICATION_MESSAGE_COUNT in Config.h."
#endif

#if (APPLICATION_MESSAGE_COUNT < 1U) || (APPLICATION_MESSAGE_COUNT > 2147483648U)
#error "APPLICATION_MESSAGE_COUNT must be >= 1 and <= 2147483648."
#endif

#ifndef APPLICATION_LINK_ROOT_SYMBOL
#error "Please define APPLICATION_LINK_ROOT_SYMBOL in Config.h."
#endif

#ifndef APPLICATION_LINK_SUB_SYMBOL
#error "Please define APPLICATION_LINK_SUB_SYMBOL in Config.h."
#endif
#endif

#if 1 /* 内部使用宏，禁止外部使用和修改 */

#define _APPLICATION_STRING(a) #a
#define _APPLICATION_CONCAT_STRING(a, b, c, d) _APPLICATION_STRING(a.b.c.d)
#define _APPLICATION_TO_SECTION(id, level) \
    _APPLICATION_CONCAT_STRING(APPLICATION_LINK_ROOT_SYMBOL, APPLICATION_LINK_SUB_SYMBOL, id, level)

#define _APPLICATION_SYMBOL(a, b, c, d) _##a##_##b##_##c##_##d
#define _APPLICATION_CONCAT_SYMBOL(a, b, c, d) _APPLICATION_SYMBOL(a, b, c, d)
#define _APPLICATION_TO_MEMBER(id, level, handler) _APPLICATION_CONCAT_SYMBOL(APPLICATION_LINK_SUB_SYMBOL, id, level, handler)

#define _APPLICATION_REG_MESSAGE_HANDLER(id, level, handler)                               \
    static const Application_MessageHandlerType _APPLICATION_TO_MEMBER(id, level, handler) \
        __attribute__((used, section(_APPLICATION_TO_SECTION(id, level)))) = handler
#endif

/**
 * @defgroup APPLICATION_MESSAGE_ID 保留消息。
 * @{
 */

/**
 * @def APPLICATION_MESSAGE_ID_IDLE
 * @brief 空闲消息 ID。用于应用程序空闲时执行操作(如休眠操作)。
 */
#define APPLICATION_MESSAGE_ID_IDLE 0
/**
 * @def APPLICATION_MESSAGE_ID_INIT
 * @brief 系统初始化消息 ID。用于应用程序启动时进行初始化操作(如全局变量等软件类操作)。
 */
#define APPLICATION_MESSAGE_ID_INIT 1
/**
 * @def APPLICATION_MESSAGE_ID_OPEN
 * @brief 打开设备消息 ID。用于应用程序启动时打开设备(如串口等硬件类操作)。
 */
#define APPLICATION_MESSAGE_ID_OPEN 2

/**
 * @def APPLICATION_MESSAGE_ID_FEED
 * @brief 喂狗消息 ID。用于应用程序空闲时喂狗。
 */
#define APPLICATION_MESSAGE_ID_FEED 3

/**
 * @}
 */

/**
 * @defgroup MessageHandlerMacros 消息处理器注册宏。
 * @{
 *
 * 便捷宏，用于注册不同级别的消息处理器。
 * 消息处理器按级别顺序执行: L1 -> L2 -> ... -> L8。
 * 同一级别也支持注册任意多个处理器。
 *
 * @warning 消息处理器禁止为 NULL。
 */

/**
 * @brief 注册 L1 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L1(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 1, handler)

/**
 * @brief 注册 L2 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L2(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 2, handler)

/**
 * @brief 注册 L3 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L3(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 3, handler)

/**
 * @brief 注册 L4 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L4(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 4, handler)

/**
 * @brief 注册 L5 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L5(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 5, handler)

/**
 * @brief 注册 L6 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L6(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 6, handler)

/**
 * @brief 注册 L7 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L7(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 7, handler)

/**
 * @brief 注册 L8 级别的消息处理器。
 * @param id 消息 ID (0-255)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REG_MESSAGE_HANDLER_L8(id, handler) _APPLICATION_REG_MESSAGE_HANDLER(id, 8, handler)

    /** @} */

#ifdef __cplusplus
}
#endif
