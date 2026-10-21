#include <string.h>
#include "Command.h"

/**
 * @brief 命令表的哨兵级别。
 *
 * 内部使用 L0 和 L9 标记命令表的边界，用户注册的命令落在 L1-L8
 * （COMMAND_REGISTER 默认注册到 L8）。链接器按段名排序后，两个哨兵
 * 之间的对象即为全部已注册命令。
 */
#define _COMMAND_LEVEL_BEGIN 0
#define _COMMAND_LEVEL_END 9

_COMMAND_REGISTER(begin, 0, 0, _COMMAND_LEVEL_BEGIN);
_COMMAND_REGISTER(end, 0, 0, _COMMAND_LEVEL_END);

/**
 * @brief 命令表结构。
 *
 * 由链接器收集的哨兵对象界定命令表范围。
 */
typedef struct
{
    const CommandType *Begin; ///< 指向 begin 哨兵，其后第一个对象才是真实命令
    const CommandType *End;   ///< 指向 end 哨兵，本身不属于命令表
} CommandTableType;

/**
 * @brief 命令表实例。
 */
static const CommandTableType _Table = {
    .Begin = &_COMMAND_TO_MEMBER(begin, _COMMAND_LEVEL_BEGIN),
    .End = &_COMMAND_TO_MEMBER(end, _COMMAND_LEVEL_END),
};

CommandType *Command_Find(const char *name)
{
    if (name == NULL)
    {
        return NULL;
    }

    /* begin + 1 跳过 begin 哨兵，c < end 跳过 end 哨兵 */
    for (const CommandType *c = _Table.Begin + 1; c < _Table.End; c++)
    {
        if (strcmp(c->Name, name) == 0)
        {
            return (CommandType *)c;
        }
    }

    return NULL;
}

int32_t Command_Execute(CommandType *command, uint32_t sender, const char *commandLine)
{
    if ((command == NULL) || (commandLine == NULL))
    {
        return COMMAND_ERROR_NULL;
    }

    if (command->Handler == NULL)
    {
        return COMMAND_ERROR_NOSUPPORT;
    }

    return command->Handler(sender, commandLine);
}
