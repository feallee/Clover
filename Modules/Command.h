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

#ifdef __cplusplus
extern "C"
{
#endif
#define COMMAND_LINK_SYMBOL Command

#if 1 /* 错误码类型 */

    /**
     * @brief 操作成功，无错误。
     */
#define COMMAND_ERROR_NONE 0
    /**
     * @brief 资源为 NULL。
     */
#define COMMAND_ERROR_NULL -1

#endif

    typedef struct
    {
        /**
         * @brief 命令名称表。以逗号间隔，区分大小写。
         */
        const char *Names;
        /**
         * @brief 命令处理器。
         * @param sender 发送者标识，由上层定义其含义（常用于表示回显/应答的输出设备）。
         * @param arguments 命令参数表，不包含命令名。
         */
        int32_t (*Handler)(uint32_t sender, const char *arguments);
        /**
         * @brief 命令标题。用于生成帮助信息。
         */
        const char *Title;
        /**
         * @brief 命令语法。用于生成帮助信息。
         */
        const char *Syntax;
    } Command_Type;

    /**
     * @brief 按名称查找命令。
     *
     * 在链接器收集的命令表中精确匹配命令名。
     *
     * @param name 命令名，不允许为 NULL。
     * @return Command_Type* 命令对象，未找到或 name 为 NULL 时返回 NULL。
     *
     * @note 返回值指向只读的命令对象，不允许修改。
     */
    const Command_Type *Command_Find(const char *name);
    /**
     * @brief 按索引查找命令。
     *
     * @param index 命令索引。
     * @return Command_Type* 命令对象，未找到时返回 NULL。
     *
     * @note 返回值指向只读的命令对象，不允许修改。
     */
    const Command_Type *Command_FindByIndex(uint32_t index);
    /**
     * @brief 获取命令数量。
     *
     * @return 命令数量。
     */
    uint32_t Command_GetCount(void);

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
    int32_t Command_Execute(const Command_Type *command, uint32_t sender, const char *arguments);

#if 1 /* 内部使用宏，禁止外部使用 */

#define _COMMAND_STRING(a) #a
#define _COMMAND_CONCAT_STRING(a, b) _COMMAND_STRING(a.b)
#define _COMMAND_TO_SECTION(level) _COMMAND_CONCAT_STRING(COMMAND_LINK_SYMBOL, level)

#define _COMMAND_SYMBOL(a, b, c) _##a##_##b##_##c
#define _COMMAND_CONCAT_SYMBOL(a, b, c) _COMMAND_SYMBOL(a, b, c)
#define _COMMAND_TO_MEMBER(handler, level) _COMMAND_CONCAT_SYMBOL(COMMAND_LINK_SYMBOL, handler, level)

#define _COMMAND_REGISTER(name, handler, title, syntax, level)                                                                \
    static const Command_Type _COMMAND_TO_MEMBER(handler, level) __attribute__((used, section(_COMMAND_TO_SECTION(level)))) = \
        {.Names = name, .Handler = handler, .Title = title, .Syntax = syntax}
#endif
#define COMMAND_REGISTER(name, handler, title, syntax) _COMMAND_REGISTER(name, handler, title, syntax, 8)
#ifdef __cplusplus
}
#endif
