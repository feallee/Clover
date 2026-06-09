#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif
  typedef  enum
    {        
        TIMER_ID_1,
        TIMER_ID_2,
        TIMER_ID_3,
        TIMER_ID_4,
        TIMER_ID_5,
        TIMER_ID_6,
        TIMER_ID_7,
        TIMER_ID_8,      

        TIMER_ID_COUNT
    } Timer_IdType;

    typedef void (*Timer_TimedType)(Timer_IdType id, void *parameter);

    bool Timer_Open(Timer_IdType id, uint32_t period, uint32_t repeat, Timer_TimedType timed);
    void Timer_Close(Timer_IdType id);

    bool Timer_Start(Timer_IdType id);

    void Timer_Stop(Timer_IdType id);

#ifdef __cplusplus
}
#endif
