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
 * @brief 消息类型负载，表示负载包含的字段数量。值域[1,5]，如果没有定义或超出值域，应用程序会强制设置为默认值 1。
 * - 1U: 包含字段：Id(8 bit)。
 * - 2U: 包含字段：Id(8 bit), BParam(8 bit)。
 * - 3U: 包含字段：Id(8 bit), BParam(8 bit), WParam(16 bit)。
 * - 4U: 包含字段：Id(8 bit), BParam(8 bit), WParam(16 bit), DParam(32 bit)。
 * - 5U: 包含字段：Id(8 bit), BParam(8 bit), WParam(16 bit), DParam(32 bit), QParam(64 bit)。
 * @note 由于负载字段会增加消息结构体的大小，建议根据实际需求选择合适的字段数量，以平衡功能和内存占用。
 */
#define APPLICATION_MESSAGE_TYPE_PAYLOAD UINT32_C(1)

/**
 * @def APPLICATION_MESSAGE_ID_RANGE
 * @brief 消息 Id 范围。值域{8,16,32,64,128,256}，如果没有定义或超出值域，应用程序会强制设置为默认值 8。
 * - 8：消息 Id 取值范围为 0-7；
 * - 16：消息 Id 取值范围为 0-15；
 * - 32：消息 Id 取值范围为 0-31；
 * - 64：消息 Id 取值范围为 0-63；
 * - 128：消息 Id 取值范围为 0-127；
 * - 256：消息 Id 取值范围为 0-255。
 */
#define APPLICATION_MESSAGE_ID_RANGE UINT32_C(8)

/**
 * @def APPLICATION_MESSAGE_CAPACITY
 * @brief 消息队列最大容量。值域[2,2147483648]，必须是 2 的整数次幂。
 *   如果没有定义或超出值域，应用程序会强制设置为默认值 64 条。
 *   占用内存(字节) = APPLICATION_MESSAGE_CAPACITY * sizeof(Application_MessageType)。
 */
#define APPLICATION_MESSAGE_CAPACITY UINT32_C(64)

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
/*TODO 在这里自定义消息*/
#define APPLICATION_MESSAGE_ID_CLOCK_SECONDED 4 /**< 秒闹钟 */

/**
 * @}
 */
#endif