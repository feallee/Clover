/**
 * @file Application.h
 * @copyright Copyright (c) 2026 weas.top team. All rights reserved.
 * @brief 消息驱动型的裸机应用程序框架，建议在应用程序中把消息处理器当作状态机的事件发生器。
 *
 * 本框架基于消息队列实现的事件驱动型应用程序框架，支持:
 * - 异步投递消息。
 * - 同步发送消息。
 * - 消息支持 8 个级别(L1-L8)的消息处理器，执行顺序：L1 -> L2 -> ... -> L8。每个级别支持任意多的消息处理器，按链接顺序执行。
 * - 消息 Id 值域：[0, APPLICATION_MESSAGE_ID_RANGE-1]。系统保留 {0,1,2,3} 4条用于系统初始化等，
 *   用户可以在 [4,APPLICATION_MESSAGE_ID_RANGE-1] 间编制消息。
 * - 消息中字段最少保留字段 Id，其它字段 BParam、WParam、DParam、QParam 可由用户自由扩展使用，满足不同应用需求。
 *   如果消息 Id 不够可以使用消息中其它字段 BParam、WParam、DParam、QParam 来扩展消息 Id，只需要在消息处理器中进行显式区分。如：
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
 *       extern int Application_Run(void*);
 *       return Application_Run(0);  // 永远不会返回。
 *    }
 *
 * 2. 使用宏注册消息处理器（可在任意文件中注册）:
 *    static void HandleKeyPress(Application_MessageType *msg) {
 *        // 处理按键消息
 *    }
 *    APPLICATION_REGISTER_HANDLER_L1(1, HandleKeyPress);
 *
 * 3. 投递/发送消息:
 *    Application_MessageType msg = {1, 0, 0, 0};
 *    Application_PostMessage(&msg); // 异步投递
 *    // 或
 *    Application_SendMessage(&msg); // 同步发送
 * @endcode
 *
 * @note 注意事项:
 * - MDK 编译器不要求显式定义消息处理器段。
 * - GCC 编译器需要在链接脚本显式定义消息处理器段 <APPLICATION_LINK_ROOT_SYMBOL>.* 段并排序，如:
 *
 *   //... other sections
 *    KEEP(*(SORT(.Entry.*)))
 *   //... other sections
 *
 * - 其它编译器，根据具体要求进行配置。
 *
 * @warning 函数 Application_PostMessage 和 Application_SendMessage 虽然支持递归调用，但必须设计退出机制，
 *         否则引起死循环或栈溢出。所以不建议在消息处理器中再投递或发送消息。
 * @warning 本库只支持 C99 及以上标准。
 *
 * @note 依赖项:
 *       - Port.h:  提供 Port_Lock/Port_Unlock 内联函数用于临界区保护。
 *       - 链接脚本: GCC 编译器需配置 Entry.* 段。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者                | 日志                                                                            |
 * | ---- | ---------- | ------------------- | ------------------------------------------------------------------------------- |
 * |      | 2026/07/05 | feallee@hotmail.com | 临界区改用 Port_Lock/Port_Unlock 内联函数，修复系统消息 payload 残留问题。      |
 * |      | 2026/05/08 | feallee@hotmail.com | 优化代码。                                                                      |
 * |      | 2026/03/06 | feallee@hotmail.com | 初版。                                                                          |
 */
#pragma once
#include <stddef.h>
#include <inttypes.h>
#include "Config.h"
#ifdef __cplusplus
extern "C"
{
#endif

#if 1 /*配置校正*/

#if (!defined(APPLICATION_MESSAGE_TYPE_PAYLOAD)) ||     \
    (APPLICATION_MESSAGE_TYPE_PAYLOAD < UINT32_C(1)) || \
    (APPLICATION_MESSAGE_TYPE_PAYLOAD > UINT32_C(2))
#define APPLICATION_MESSAGE_TYPE_PAYLOAD UINT32_C(1)
#endif

#if (!defined(APPLICATION_MESSAGE_ID_RANGE)) ||       \
    (APPLICATION_MESSAGE_ID_RANGE < UINT32_C(8)) ||   \
    (APPLICATION_MESSAGE_ID_RANGE > UINT32_C(256)) || \
    ((APPLICATION_MESSAGE_ID_RANGE & (APPLICATION_MESSAGE_ID_RANGE - UINT32_C(1))) != UINT32_C(0))
#define APPLICATION_MESSAGE_ID_RANGE UINT32_C(8)
#endif

#if (!defined(APPLICATION_MESSAGE_CAPACITY)) ||              \
    (APPLICATION_MESSAGE_CAPACITY < UINT32_C(2)) ||          \
    (APPLICATION_MESSAGE_CAPACITY > UINT32_C(2147483648)) || \
    ((APPLICATION_MESSAGE_CAPACITY & (APPLICATION_MESSAGE_CAPACITY - UINT32_C(1))) != UINT32_C(0))
#define APPLICATION_MESSAGE_CAPACITY UINT32_C(64)
#endif

#ifndef APPLICATION_LINK_ROOT_SYMBOL
#define APPLICATION_LINK_ROOT_SYMBOL Entry
#endif

#ifndef APPLICATION_LINK_SUB_SYMBOL
#define APPLICATION_LINK_SUB_SYMBOL AppMsg
#endif

#endif

#if 1 /* Application 保留消息 {0,1,2,3}。 */
/**
 * @defgroup application_reserved_ids 保留消息 ID。
 * @{
 */

/**
 * @def APPLICATION_MESSAGE_ID_INIT
 * @brief 系统初始化消息 ID。用于应用程序启动时进行初始化操作(如全局变量等软件类操作)。
 */
#define APPLICATION_MESSAGE_ID_INIT 0

/**
 * @def APPLICATION_MESSAGE_ID_OPEN
 * @brief 打开设备消息 ID。用于应用程序启动时打开设备(如串口等硬件类操作)。
 */
#define APPLICATION_MESSAGE_ID_OPEN 1

/**
 * @def APPLICATION_MESSAGE_ID_FEED
 * @brief 喂狗消息 ID。用于应用程序喂狗。
 */
#define APPLICATION_MESSAGE_ID_FEED 2

/**
 * @def APPLICATION_MESSAGE_ID_IDLE
 * @brief 空闲消息 ID。用于应用程序空闲时执行操作(如休眠操作)。
 */
#define APPLICATION_MESSAGE_ID_IDLE 3

/**
 * @}
 */
#endif

    /**
     * @brief 应用框架错误类型。
     */
    typedef enum
    {
        APPLICATION_ERROR_NONE = 0,   /**< 操作成功，无错误。 */
        APPLICATION_ERROR_NULL = -1,  /**< 资源为 NULL。 */
        APPLICATION_ERROR_RANGE = -2, /**< 资源超出有效范围。 */
        APPLICATION_ERROR_EMPTY = -3, /**< 资源为空。 */
        APPLICATION_ERROR_FULL = -4   /**< 资源已满。 */
    } Application_ErrorType;

    /**
     * @brief 应用消息类型。
     */
    typedef struct
    {
#if APPLICATION_MESSAGE_TYPE_PAYLOAD >= UINT32_C(1)
        uint8_t ID; ///< 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
#endif
#if APPLICATION_MESSAGE_TYPE_PAYLOAD >= UINT32_C(2)
        uint8_t BParam; ///< 字节参数 (8位)
#endif
#if APPLICATION_MESSAGE_TYPE_PAYLOAD >= UINT32_C(3)
        uint16_t WParam; ///< 字参数 (16位)
#endif
#if APPLICATION_MESSAGE_TYPE_PAYLOAD >= UINT32_C(4)
        uint32_t DParam; ///< 双字参数 (32位)
#endif
#if APPLICATION_MESSAGE_TYPE_PAYLOAD >= UINT32_C(5)
        uint64_t QParam; ///< 四字参数 (64位)
#endif
    } Application_MessageType;

    /**
     * @brief 应用消息处理器类型。
     * @param message 指向当前待处理消息的指针。
     */
    typedef void (*Application_MessageHandlerType)(Application_MessageType *message);

    /**
     * @brief 运行应用程序并处理消息循环。
     * 执行消息处理循环，不断从消息队列中取出消息并执行已注册的消息处理器。
     *
     * @param parameter 用户自定义参数（当前未使用，保留用于扩展）。
     *
     * @note 该函数包含无限循环，正常情况下不会返回。
     * @note 函数启动时会先同步发送 APPLICATION_MESSAGE_ID_INIT 消息进行初始化。
     * @note 当队列为空时，发送 APPLICATION_MESSAGE_ID_IDLE 消息，用户可在消息处理器中进入低功耗模式。
     */
    int Application_Run(void *parameter);

    /**
     * @brief 异步投递消息到队列。
     *
     * 将消息添加到消息队列尾部，由主循环异步处理。
     * 如果队列已满，则投递失败返回 APPLICATION_ERROR_FULL。
     *
     * @param message 指向要投递的消息的指针，不允许为 NULL。
     * @return 返回 Application_ErrorType 错误码。
     *
     * @note 该函数是线程/中断安全的，可以在中断中调用。
     */
    Application_ErrorType Application_PostMessage(Application_MessageType *message);

    /**
     * @brief 同步发送消息
     *
     * 立即调用该消息 ID 对应的所有处理器（L1-L8），不经过消息队列。
     * 处理器按级别顺序执行：L1 -> L2 -> ... -> L8，同一级别内的处理器按链接顺序执行。
     *
     * @param message 指向要发送的消息的指针，不允许为 NULL。
     * @return 返回 Application_ErrorType 错误码。
     *
     * @note 该函数是同步的，会阻塞直到所有处理器执行完毕。
     * @note 支持递归调用（处理器中再次调用 Application_SendMessage），但必须设计退出机制。
     */
    Application_ErrorType Application_SendMessage(Application_MessageType *message);

#if 1 /* 内部使用宏，禁止外部使用和修改 */

#define _APPLICATION_STRING(a) #a
#define _APPLICATION_CONCAT_STRING(a, b, c, d) _APPLICATION_STRING(a.b.c.d)
#define _APPLICATION_TO_SECTION(id, level) \
    _APPLICATION_CONCAT_STRING(APPLICATION_LINK_ROOT_SYMBOL, APPLICATION_LINK_SUB_SYMBOL, id, level)

#define _APPLICATION_SYMBOL(a, b, c, d) _##a##_##b##_##c##_##d
#define _APPLICATION_CONCAT_SYMBOL(a, b, c, d) _APPLICATION_SYMBOL(a, b, c, d)
#define _APPLICATION_TO_MEMBER(id, level, handler) \
    _APPLICATION_CONCAT_SYMBOL(APPLICATION_LINK_SUB_SYMBOL, id, level, handler)

#define _APPLICATION_REGISTER_HANDLER(id, level, handler)                                  \
    static const Application_MessageHandlerType _APPLICATION_TO_MEMBER(id, level, handler) \
        __attribute__((used, section(_APPLICATION_TO_SECTION(id, level)))) = handler
#endif

/**
 * @defgroup MessageHandlerMacros 消息处理器注册宏。
 * @{
 *
 * 便捷宏，用于注册不同级别的消息处理器。
 * 消息处理器按级别顺序执行: L1 -> L2 -> ... -> L8。
 * 同一级别也支持注册任意多个处理器，处理器按链接顺序执行。
 */

/**
 * @brief 注册 L1 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L1(id, handler) _APPLICATION_REGISTER_HANDLER(id, 1, handler)

/**
 * @brief 注册 L2 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L2(id, handler) _APPLICATION_REGISTER_HANDLER(id, 2, handler)

/**
 * @brief 注册 L3 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L3(id, handler) _APPLICATION_REGISTER_HANDLER(id, 3, handler)

/**
 * @brief 注册 L4 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L4(id, handler) _APPLICATION_REGISTER_HANDLER(id, 4, handler)

/**
 * @brief 注册 L5 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L5(id, handler) _APPLICATION_REGISTER_HANDLER(id, 5, handler)

/**
 * @brief 注册 L6 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L6(id, handler) _APPLICATION_REGISTER_HANDLER(id, 6, handler)

/**
 * @brief 注册 L7 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L7(id, handler) _APPLICATION_REGISTER_HANDLER(id, 7, handler)

/**
 * @brief 注册 L8 级别的消息处理器。
 * @param id 消息 ID (0 到 APPLICATION_MESSAGE_ID_RANGE-1)。
 * @param handler 消息处理器函数名，必须符合 Application_MessageHandlerType 类型。
 */
#define APPLICATION_REGISTER_HANDLER_L8(id, handler) _APPLICATION_REGISTER_HANDLER(id, 8, handler)

    /** @} */

#ifdef __cplusplus
}
#endif