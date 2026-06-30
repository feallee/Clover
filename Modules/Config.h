#pragma once

/**
 * @file Config.h
 * @brief 应用程序配置文件，定义应用程序相关的配置信息。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者                | 日志                                                                            |
 * | ---- | ---------- | ------------------- | ------------------------------------------------------------------------------- |
 * |      | 2026/03/07 | feallee@hotmail.com | 初版。                                                                          |
 *
 */

#if 1 /** Application */

/**
 * @def APPLICATION_MESSAGE_TYPE_PAYLOAD
 * @brief 消息类型负载，表示负载包含的字段数量。值域[1,5]，如果没有定义或超出值域，应用程序会强制设置为默认值 1U。
 * - 1U: 包含字段：Id(8 bit)。
 * - 2U: 包含字段：Id(8 bit), BParam(8 bit)。
 * - 3U: 包含字段：Id(8 bit), BParam(8 bit), WParam(16 bit)。
 * - 4U: 包含字段：Id(8 bit), BParam(8 bit), WParam(16 bit), DParam(32 bit)。
 * - 5U: 包含字段：Id(8 bit), BParam(8 bit), WParam(16 bit), DParam(32 bit), QParam(64 bit)。
 * @note 由于负载字段会增加消息结构体的大小，建议根据实际需求选择合适的字段数量，以平衡功能和内存占用。
 */
#define APPLICATION_MESSAGE_TYPE_PAYLOAD 2U

/**
 * @def APPLICATION_MESSAGE_ID_RANGE
 * @brief 消息 Id 范围。值域{8,16,32,64,128,256}，如果没有定义或超出值域，应用程序会强制设置为默认值 8U。
 * - 8U：消息 Id 取值范围为 0-7；
 * - 16U：消息 Id 取值范围为 0-15；
 * - 32U：消息 Id 取值范围为 0-31；
 * - 64U：消息 Id 取值范围为 0-63；
 * - 128U：消息 Id 取值范围为 0-127；
 * - 256U：消息 Id 取值范围为 0-255。
 */
#define APPLICATION_MESSAGE_ID_RANGE 8U

/**
 * @def APPLICATION_MESSAGE_CAPACITY
 * @brief 消息队列最大容量。值域[4,2^(8*sizeof(size_t)-1)]，必须是 2 的整数次幂。
 *   如果没有定义或超出值域，应用程序会强制设置为默认值 1024 条。
 *   占用内存(字节) = APPLICATION_MESSAGE_CAPACITY * sizeof(Application_MessageType)。
 */
#define APPLICATION_MESSAGE_CAPACITY 64ULL

/**
 * @def APPLICATION_LINK_ROOT_SYMBOL
 * @brief 消息处理器链接根符号。链接后生成段名格式：<APPLICATION_LINK_ROOT_SYMBOL>.<APPLICATION_LINK_SUB_SYMBOL>.<ID>.<LEVEL>。
 */
#define APPLICATION_LINK_ROOT_SYMBOL Entry

/**
 * @def APPLICATION_LINK_SUB_SYMBOL
 * @brief 消息处理器链接子符号。
 */
#define APPLICATION_LINK_SUB_SYMBOL AppMsg

/**
 * @defgroup application_user_message_ids 用户自定义消息。值域[4,APPLICATION_MESSAGE_ID_RANGE-1]。
 * @{
 */

#define APPLICATION_MESSAGE_ID_TICKED 4          /**< systick定时器消息。 */
#define APPLICATION_MESSAGE_ID_USART1_RECEIVED 5 /**< USART1 接收完成消息。 */
#define APPLICATION_MESSAGE_ID_RTC_TIMED 6       /**< RTC 定时消息。 */

/**
 * @def APPLICATION_DEBUG_MODE
 * @brief 调试模式开关。0 表示关闭调试模式，非 0 表示开启调试模式。
 */
#define APPLICATION_DEBUG_MODE 0

/**
 * @}
 */
#endif