#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        POWER_MODE_SLEEP,
        POWER_MODE_STOP,
        POWER_MODE_STANDBY,
    } Power_ModeType;

    void Power_EnterMode(Power_ModeType mode);  

#ifdef __cplusplus
}
#endif
