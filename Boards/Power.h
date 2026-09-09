#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        POWER_MODE_SLEEP,/*睡眠模式，仅关闭CPU时钟，任意中断或事件唤醒。*/
        POWER_MODE_STOP,/*停止模式，关闭所有高速时钟，可通过外部中断唤醒。*/
        POWER_MODE_STANDBY,/*待机模式，关闭所有时钟除备份域。唤醒后系统复位。*/        
    } Power_ModeType;

    void Power_EnterMode(Power_ModeType mode);  

#ifdef __cplusplus
}
#endif
