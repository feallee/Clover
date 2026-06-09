#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#if __cplusplus
extern "C"
{
#endif

/*最大帧大小*数据安全冗余倍数，默认为 2，如果 Usart_Read() 太慢，可以适当调大*/
#define USART_RECEIVE_BUFFER_SIZE_1 (256 * 8)
#define USART_RECEIVE_BUFFER_SIZE_2 (256 * 2)
#define USART_RECEIVE_BUFFER_SIZE_3 (256 * 2)
#define USART_RECEIVE_BUFFER_SIZE_4 (256 * 2)
#define USART_RECEIVE_BUFFER_SIZE_5 (256 * 2)
#define USART_RECEIVE_BUFFER_SIZE_6 (256 * 2)
#define USART_RECEIVE_BUFFER_SIZE_7 (256 * 2)
#define USART_RECEIVE_BUFFER_SIZE_8 (256 * 2)
    typedef enum
    {
        USART_ID_1,
        USART_ID_2,
        USART_ID_3,
        USART_ID_4,
        USART_ID_5,
        USART_ID_6,
        USART_ID_7,
        USART_ID_8,
    } Usart_IdType;

    typedef void *Usart_DeviceType;
    typedef void (*Usart_Received)(Usart_DeviceType device, size_t length);
    typedef void (*Usart_Transmitted)(Usart_DeviceType device, size_t length);

    Usart_DeviceType Usart_GetDevice(Usart_IdType id);
    bool Usart_Open(Usart_DeviceType device, uint32_t baudrate, Usart_Received received, Usart_Transmitted transmitted);
    void Usart_Close(Usart_DeviceType device);
    size_t Usart_Write(Usart_DeviceType device, const void *buffer, size_t length);
    size_t Usart_Read(Usart_DeviceType device, void *buffer, size_t length);

#ifdef __cplusplus
}
#endif