#include "stm32f10x.h"
#include "Power.h"

void Power_EnterMode(Power_ModeType mode)
{
    switch (mode)
    {
    case POWER_MODE_STOP:
    {
        // 使能PWR和BKP时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

        // 使能对后备寄存器的访问(重要!!!否则会死机)
        PWR_BackupAccessCmd(ENABLE);

        // 进入STOP模式，WFI确保唤醒后优先进入中断处理函数
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

        // 唤醒后必须重新配置系统时钟
        SystemInit();
        SystemCoreClockUpdate();
    }
    break;

    case POWER_MODE_STANDBY:
    {
        // 使能PWR时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

        // 可选: 使能WKUP引脚唤醒
        PWR_WakeUpPinCmd(ENABLE);

        // 清除唤醒标志
        PWR_ClearFlag(PWR_FLAG_WU);

        // 进入STANDBY模式
        PWR_EnterSTANDBYMode();
    }
    break;

    case POWER_MODE_SLEEP:
    default:
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        __WFI();
    }
    break;
    }
}
