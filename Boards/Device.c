#include "Device.h"

_DEVICE_REGISTER(begin, 0, 0, 0, 0, 0, 0);
_DEVICE_REGISTER(end, 9, 0, 0, 0, 0, 0);

DeviceType *Device_Open(const char *name, const void *parameter)
{
    for (DeviceType *d = _Table; d < _Table + sizeof(_Table) / sizeof(_Table[0]); d++)
    {
        if (strcmp(d->Name, name) == 0)
        {
            if (d->Open(parameter) >= 0)
                return d;
            else
                return NULL;
        }
    }

    return NULL;
}

int32_t Device_Close(DeviceType *device)
{
    return device->Close();
}

int32_t Device_Read(DeviceType *device, void *buffer, uint32_t length, const void *parameter)
{
    return device->Read(buffer, length, parameter);
}
int32_t Device_Write(DeviceType *device, const void *buffer, uint32_t length, const void *parameter)
{
    return device->Write(buffer, length, parameter);
}
int32_t Device_Control(DeviceType *device, uint32_t command, const void *parameter)
{
    return device->Control(command, parameter);
}