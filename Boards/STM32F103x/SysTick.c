#include <stdbool.h>
#include <string.h>
#include "stm32f10x.h"
#include "SysTick.h"

struct _SysTick_DeviceType
{
    const char *Name;
    const char *Mode;
    bool IsOpen;
    SysTick_TickedType Ticked;
};

static struct _SysTick_DeviceType _Devices[1] = {{.Name = "SYSTICK"}};

void SysTick_Handler(void)
{
    if (_Devices[0].Ticked != NULL)
    {
        _Devices[0].Ticked(&_Devices[0]);
    }
}

SysTick_DeviceType *SysTick_Open(const char *name, const char *mode)
{
    uint32_t reload;

    if ((name == NULL) || (mode == NULL))
    {
        return NULL;
    }

    if (strcmp(name, _Devices[0].Name) != 0)
    {
        return NULL;
    }

    if (_Devices[0].IsOpen)
    {
        if (strcmp(mode, _Devices[0].Mode) == 0)
        {
            return &_Devices[0];
        }
        return NULL;
    }

    if (strcmp(mode, "10ms") == 0)
    {
        reload = SystemCoreClock / 100U;
    }
    else if (strcmp(mode, "100ms") == 0)
    {
        reload = SystemCoreClock / 10U;
    }
    else
    {
        reload = SystemCoreClock / 1000U;
    }

    if (SysTick_Config(reload))
    {
        return NULL;
    }

    _Devices[0].Mode = mode;
    _Devices[0].IsOpen = true;

    return &_Devices[0];
}

SysTick_ErrorType SysTick_Close(SysTick_DeviceType *device)
{
    if (device == NULL)
    {
        return SYSTICK_ERROR_NULL;
    }

    if (!device->IsOpen)
    {
        return SYSTICK_ERROR_NONE;
    }

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    device->Ticked = NULL;
    device->Mode = NULL;
    device->IsOpen = false;

    return SYSTICK_ERROR_NONE;
}

SysTick_ErrorType SysTick_SetTicked(SysTick_DeviceType *device, SysTick_TickedType ticked)
{
    if (device == NULL)
    {
        return SYSTICK_ERROR_NULL;
    }

    device->Ticked = ticked;

    return SYSTICK_ERROR_NONE;
}

SysTick_ErrorType SysTick_Read(SysTick_DeviceType *device, uint32_t *value)
{
    if ((device == NULL) || (value == NULL) || (!device->IsOpen))
    {
        return SYSTICK_ERROR_NULL;
    }

    *value = SysTick->VAL & SysTick_LOAD_RELOAD_Msk;

    return SYSTICK_ERROR_NONE;
}

SysTick_ErrorType SysTick_Write(SysTick_DeviceType *device, uint32_t value)
{
    if ((device == NULL) || (!device->IsOpen))
    {
        return SYSTICK_ERROR_NULL;
    }

    if (value > SysTick_LOAD_RELOAD_Msk)
    {
        return SYSTICK_ERROR_RANGE;
    }

    SysTick->LOAD = value;
    SysTick->VAL = 0;

    return SYSTICK_ERROR_NONE;
}

SysTick_ErrorType SysTick_GetMaxValue(SysTick_DeviceType *device, uint32_t *value)
{
    if (value == NULL)
    {
        return SYSTICK_ERROR_NULL;
    }

    (void)device;
    *value = SysTick_LOAD_RELOAD_Msk;

    return SYSTICK_ERROR_NONE;
}
