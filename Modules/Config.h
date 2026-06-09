#pragma once

/**
 * @file Config.h
 * @brief 应用程序配置文件，定义应用程序相关的宏和类型。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者            | 日志                                                                                |
 * | ---- | :--------- | :---------------| :-----------------------------------------------------------------------------------|
 * |      | 2026/03/07 | feallee@hotmail | 初版                                                                                |
 *
 */

#if 1 /** Application */
/**
 * @def APPLICATION_DEBUG_MODE
 * @brief 调试模式。
 *
 * - 0: 生产模式，1: 调试模式。
 */
#define APPLICATION_DEBUG_MODE 1
/**
 * @def APPLICATION_MESSAGE_COUNT
 * @brief 消息队列最大容量。
 *
 * - 值域：[1,2147483648]，默认为 1024 条。
 *   占用内存(字节) = APPLICATION_MESSAGE_COUNT * sizeof(Application_MessageType)。
 *
 * - 值配置参考：50% <= 峰值使用率 < 70%，过高可能导致频繁丢消息，过低可能浪费内存。
 *   峰值使用率 = Application_GetMessageCount() * 100 / APPLICATION_MESSAGE_COUNT
 *   还可以查询 Application_GetMessageOverflow() 获取丢失消息数量，指导配置 APPLICATION_MESSAGE_COUNT。
 */
#define APPLICATION_MESSAGE_COUNT 1024U

/**
 * @def APPLICATION_LINK_ROOT_SYMBOL
 * @brief 消息处理器链接根符号。
 */
#define APPLICATION_LINK_ROOT_SYMBOL Entry
/**
 * @def APPLICATION_LINK_SUB_SYMBOL
 * @brief 消息处理器链接子符号。
 */
#define APPLICATION_LINK_SUB_SYMBOL AppMsg
/**
 * @defgroup APPLICATION_MESSAGE_ID 用户自定义消息。从 8 开始编制。
 * @{
 */
#define APPLICATION_MESSAGE_ID_RTC_TIMED 8 ///< 1秒闹钟消息ID
#define APPLICATION_MESSAGE_ID_USART1_RECEIVED 9 ///< USART1接收消息ID
/**
 * @}
 */
#endif
#if 1 /** RTC */

#endif
