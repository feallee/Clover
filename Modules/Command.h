/**
 * @file Command.h
 * @brief 统一的命令抽象层，为用户提供命令的静态注册、按名查找与执行接口。
 *
 * 用户通过 COMMAND_REGISTER 把命令注册到由链接器收集的命令表中；
 * 上层（串口控制台、Shell 等）从命令行中取出命令名，用 Command_Find 查找，
 * 再用 Command_Execute 把发送者标识与原始命令行交给命令处理器。
 *
 * @note 注意事项:
 * - MDK 编译器不要求显式定义命令段。
 * - GCC 编译器需要在链接脚本显式定义命令段 <COMMAND_LINK_SYMBOL>.* 段（形如 Command.<level>，
 *   无前导 "."）并排序，如:
 *
 *   //... other sections
 *    KEEP(*(SORT(Command.*)))
 *   //... other sections
 *
 * - 其它编译器，根据具体要求进行配置。
 *
 * @copyright Copyright (C) 2026 weas.top team. All rights reserved.
 *
 * @warning 本库只支持 C99 及以上标准。
 *
 * @note 依赖项:
 *       - 链接脚本: GCC 编译器需配置 Command.* 段（无前导 "."）。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者                | 日志                                                                            |
 * | ---- | ---------- | ------------------- | ------------------------------------------------------------------------------- |
 * |      | 2026/10/21 | feallee@hotmail.com | 初版。                                                                          |
 */
#pragma once
#include <inttypes.h>
#include "Config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if 1 /* 错误码 */
    /**
     * 操作成功，无错误。
     */
#define COMMAND_ERROR_NONE (0)
    /**
     * 资源为 NULL。
     */
#define COMMAND_ERROR_NULL (-1)
    /**
     * 不支持的操作。
     */
#define COMMAND_ERROR_NOSUPPORT (-2)
#endif

    typedef struct
    {
        const char *Name;        /**< 命令名，由 COMMAND_REGISTER 字符串化得到，区分大小写。 */
        const char *Description; /**< 命令描述，用于帮助信息，可以为 NULL。 */
        int32_t (*Handler)(uint32_t sender, const char *commandLine); /**< 命令处理器，接收发送者标识与原始命令行，自行解析参数。 */
    } CommandType;

    /**
     * @brief 按名称查找命令。
     *
     * 在链接器收集的命令表中精确匹配命令名。
     *
     * @param name 命令名，不允许为 NULL。
     * @return CommandType* 命令对象，未找到或 name 为 NULL 时返回 NULL。
     *
     * @note 返回值指向只读的命令对象，不允许修改。
     */
    CommandType *Command_Find(const char *name);

    /**
     * @brief 执行命令处理器。
     *
     * 把发送者标识与原始命令行原样传递给命令处理器，由处理器自行解析参数。
     *
     * @param command 由 Command_Find 返回的命令对象，不允许为 NULL。
     * @param sender 发送者标识，由上层定义其含义（常用于表示回显/应答的输出设备）。
     * @param commandLine 原始命令行，不允许为 NULL。
     *
     * @return 返回错误码。
     * @retval COMMAND_ERROR_NULL 参数 command 或 commandLine 为 NULL。
     * @retval COMMAND_ERROR_NOSUPPORT 命令未实现处理器。
     * @retval 其他 命令处理器返回的错误码。
     */
    int32_t Command_Execute(CommandType *command, uint32_t sender, const char *commandLine);

#if 1 /* 内部使用宏，禁止外部使用 */

#define _COMMAND_STRING(a) #a
#define _COMMAND_CONCAT_STRING(a, b) _COMMAND_STRING(a.b)
#define _COMMAND_TO_SECTION(level) _COMMAND_CONCAT_STRING(COMMAND_LINK_SYMBOL, level)

#define _COMMAND_SYMBOL(a, b, c) _##a##_##b##_##c
#define _COMMAND_CONCAT_SYMBOL(a, b, c) _COMMAND_SYMBOL(a, b, c)
#define _COMMAND_TO_MEMBER(name, level) _COMMAND_CONCAT_SYMBOL(COMMAND_LINK_SYMBOL, name, level)

#define _COMMAND_REGISTER(name, description, handler, level)                                                             \
    static const CommandType _COMMAND_TO_MEMBER(name, level) __attribute__((used, section(_COMMAND_TO_SECTION(level)))) = \
        {.Name = _COMMAND_STRING(name), .Description = description, .Handler = handler}
#endif
#define COMMAND_REGISTER(name, description, handler) _COMMAND_REGISTER(name, description, handler, 8)
#ifdef __cplusplus
}
#endif
