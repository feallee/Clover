#include <string.h>
#include "Command.h"

/**
 * @file Command.c
 * @brief 统一的命令抽象层实现。
 *
 * @note 修订记录
 * | 版本 | 日期       | 作者                | 日志                                                                            |
 * | ---- | ---------- | ------------------- | ------------------------------------------------------------------------------- |
 * |      | 2026/11/05 | feallee@hotmail.com | 适配新头文件：Names 名称表、FindByIndex/GetCount 接口。                          |
 */

/**
 * @brief 命令表的哨兵级别。
 *
 * 内部使用 L0 和 L9 标记命令表的边界，用户注册的命令落在 L1-L8
 * （COMMAND_REGISTER 默认注册到 L8）。链接器按段名排序后，两个哨兵
 * 之间的对象即为全部已注册命令。
 */
#define _COMMAND_LEVEL_BEGIN 0
#define _COMMAND_LEVEL_END 9

_COMMAND_REGISTER(0, 0, 0, 0, _COMMAND_LEVEL_BEGIN);
_COMMAND_REGISTER(0, 0, 0, 0, _COMMAND_LEVEL_END);

/**
 * @brief 命令表结构。
 *
 * 由链接器收集的哨兵对象界定命令表范围。
 */
typedef struct
{
    const Command_Type *Begin; ///< 指向 begin 哨兵，其后第一个对象才是真实命令
    const Command_Type *End;   ///< 指向 end 哨兵，本身不属于命令表
} CommandTableType;

/**
 * @brief 命令表实例。
 */
static const CommandTableType _Table = {
    .Begin = &_COMMAND_TO_MEMBER(0, _COMMAND_LEVEL_BEGIN),
    .End = &_COMMAND_TO_MEMBER(0, _COMMAND_LEVEL_END),
};

/**
 * @brief 判断名称表是否包含指定命令名。
 *
 * 名称表以逗号间隔（形如 "name1,name2"），匹配精确且区分大小写，
 * 空令牌被跳过。
 *
 * @param names 名称表，以 '\0' 结尾，不允许为 NULL。
 * @param name 待匹配的命令名，不允许为 NULL。
 * @return 匹配成功返回 1，否则返回 0。
 */
static int _MatchNames(const char *names, const char *name)
{
    const char *cursor = names;
    size_t nameLength = strlen(name);

    while (*cursor != '\0')
    {
        const char *token = cursor;
        size_t tokenLength;

        while ((*cursor != '\0') && (*cursor != ','))
        {
            cursor++;
        }
        tokenLength = (size_t)(cursor - token);

        if ((tokenLength > 0) && (tokenLength == nameLength) && (strncmp(token, name, tokenLength) == 0))
        {
            return 1;
        }

        if (*cursor == ',')
        {
            cursor++;
        }
    }

    return 0;
}

const Command_Type *Command_Find(const char *name)
{
    if (name == NULL)
    {
        return NULL;
    }

    /* begin + 1 跳过 begin 哨兵，c < end 跳过 end 哨兵 */
    for (const Command_Type *command = _Table.Begin + 1; command < _Table.End; command++)
    {
        if (_MatchNames(command->Names, name))
        {
            return command;
        }
    }

    return NULL;
}

const Command_Type *Command_FindByIndex(uint32_t index)
{
    const Command_Type *command = _Table.Begin + 1 + index;

    if (command >= _Table.End)
    {
        return NULL;
    }

    return command;
}

uint32_t Command_GetCount(void)
{
    /* 减去 begin 与 end 两个哨兵 */
    return (uint32_t)(_Table.End - _Table.Begin - 1);
}

int32_t Command_Execute(const Command_Type *command, uint32_t sender, const char *arguments)
{
    if ((command == NULL))
    {
        return COMMAND_ERROR_NULL;
    }

    if (command->Handler == NULL)
    {
        return COMMAND_ERROR_NULL;
    }

    return command->Handler(sender, arguments);
}
