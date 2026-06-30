#include "stm32f10x.h"

#include "Core.h"

extern uint32_t Core_EnterCritical(void)
{
    uint32_t primask = __get_PRIMASK();
    __set_PRIMASK(1);
    return primask;
}

extern void Core_ExitCritical(uint32_t primask)
{
    __set_PRIMASK(primask);
}

extern void Core_EnableDebug(void)
{
    // 使能调试接口在STOP模式下的工作
    DBGMCU_Config(DBGMCU_STOP, ENABLE);
}
