#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        REALTIMER_ID_1,
        REALTIMER_ID_2,
        REALTIMER_ID_3,
        REALTIMER_ID_4,

        REALTIMER_ID_COUNT
    } RealTimer_IdType;

    typedef void *RealTimer_DeviceType;
    typedef void (*RealTimer_AlarmedType)(RealTimer_DeviceType device, void *parameter);
    RealTimer_DeviceType RealTimer_GetDevice(RealTimer_IdType id);
    bool RealTimer_Open(RealTimer_DeviceType device, RealTimer_AlarmedType alarmed);
    void RealTimer_Close(RealTimer_DeviceType device);
    bool RealTimer_Read(RealTimer_DeviceType device, uint32_t *timestamp);
    bool RealTimer_Write(RealTimer_DeviceType device, uint32_t timestamp);


#ifdef __cplusplus
}
#endif
