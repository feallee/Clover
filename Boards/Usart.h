#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define USART_ERROR_SUCCESS          0
#define USART_ERROR_INVALID_HANDLE  -1
#define USART_ERROR_ALREADY_OPEN    -2
#define USART_ERROR_OPEN_FAILED     -3
#define USART_ERROR_NOT_OPEN        -4
#define USART_ERROR_INVALID_ARGUMENT -5

#ifdef __cplusplus
extern "C"
{
#endif

    struct Usart_Device;
    typedef struct Usart_Device *Usart_DeviceType;
    typedef void (*Usart_HandlerType)(Usart_DeviceType device, size_t index);

    Usart_DeviceType Usart_GetDevice(const char *name);
    int Usart_SetHandler(Usart_DeviceType device, Usart_HandlerType received, Usart_HandlerType transmitted);
    int Usart_Open(Usart_DeviceType device);
    int Usart_Close(Usart_DeviceType device);
    size_t Usart_Write(Usart_DeviceType device, const void *buffer, size_t length);
    size_t Usart_Read(Usart_DeviceType device, void *buffer, size_t length);

#ifdef __cplusplus
}
#endif
