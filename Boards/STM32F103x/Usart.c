#include "stm32f10x.h"
#include "Usart.h"
#include "Core.h"
#include <string.h>
typedef struct
{
    Usart_Received Received;
    Usart_Transmitted Transmitted;
    bool IsOpen;
    size_t ReceiveReadIndex;
} DeviceType;
static DeviceType _DeviceTable[];
static uint8_t _Usart1_RxBuffer[USART_RECEIVE_BUFFER_SIZE_1];
static uint8_t _Usart2_RxBuffer[USART_RECEIVE_BUFFER_SIZE_2];
static uint8_t _Usart3_RxBuffer[USART_RECEIVE_BUFFER_SIZE_3];

static bool Open1(uint32_t baudrate)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure = {0};
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart1_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART_RECEIVE_BUFFER_SIZE_1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
    return true;
}

static void Close1(void)
{
    USART_Cmd(USART1, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
    USART_DeInit(USART1);
}

static size_t Write1(const void *buffer, size_t length)
{
    if (buffer == NULL || length == 0) return 0;

    uint8_t *data = (uint8_t *)buffer;
    for (size_t i = 0; i < length; i++)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, data[i]);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    return length;
}

static size_t Read1(void *buffer, size_t length)
{
    if (buffer == NULL || length == 0) return 0;

    DeviceType *dev = &_DeviceTable[0];
    uint8_t *data = (uint8_t *)buffer;

    if (length == USART_RECEIVE_BUFFER_SIZE_1)
    {
        length = 0;
    }

    if (length > dev->ReceiveReadIndex)
    {
        size_t readLength = length - dev->ReceiveReadIndex;
        memcpy(data, &_Usart1_RxBuffer[dev->ReceiveReadIndex], readLength);
        dev->ReceiveReadIndex = length;
        return readLength;
    }
    else if (length < dev->ReceiveReadIndex)
    {
        size_t firstPart = USART_RECEIVE_BUFFER_SIZE_1 - dev->ReceiveReadIndex;
        memcpy(data, &_Usart1_RxBuffer[dev->ReceiveReadIndex], firstPart);
        memcpy(&data[firstPart], _Usart1_RxBuffer, length);
        dev->ReceiveReadIndex = length;
        return firstPart + length;
    }

    return 0;
}

static bool Open2(uint32_t baudrate)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure = {0};
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart2_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART_RECEIVE_BUFFER_SIZE_2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);

    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA1_Channel6, ENABLE);

    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
    return true;
}

static void Close2(void)
{
    USART_Cmd(USART2, DISABLE);
    DMA_Cmd(DMA1_Channel6, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
    USART_DeInit(USART2);
}

static size_t Write2(const void *buffer, size_t length)
{
    if (buffer == NULL || length == 0) return 0;

    uint8_t *data = (uint8_t *)buffer;
    for (size_t i = 0; i < length; i++)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, data[i]);
    }
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);

    return length;
}

static size_t Read2(void *buffer, size_t length)
{
    if (buffer == NULL || length == 0) return 0;

    DeviceType *dev = &_DeviceTable[1];
    uint8_t *data = (uint8_t *)buffer;

    if (length == USART_RECEIVE_BUFFER_SIZE_2)
    {
        length = 0;
    }

    if (length > dev->ReceiveReadIndex)
    {
        size_t readLength = length - dev->ReceiveReadIndex;
        memcpy(data, &_Usart2_RxBuffer[dev->ReceiveReadIndex], readLength);
        dev->ReceiveReadIndex = length;
        return readLength;
    }
    else if (length < dev->ReceiveReadIndex)
    {
        size_t firstPart = USART_RECEIVE_BUFFER_SIZE_2 - dev->ReceiveReadIndex;
        memcpy(data, &_Usart2_RxBuffer[dev->ReceiveReadIndex], firstPart);
        memcpy(&data[firstPart], _Usart2_RxBuffer, length);
        dev->ReceiveReadIndex = length;
        return firstPart + length;
    }

    return 0;
}

static bool Open3(uint32_t baudrate)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure = {0};
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart3_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART_RECEIVE_BUFFER_SIZE_3;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
    return true;
}

static void Close3(void)
{
    USART_Cmd(USART3, DISABLE);
    DMA_Cmd(DMA1_Channel3, DISABLE);
    USART_DMACmd(USART3, USART_DMAReq_Rx, DISABLE);
    USART_DeInit(USART3);
}

static size_t Write3(const void *buffer, size_t length)
{
    if (buffer == NULL || length == 0) return 0;

    uint8_t *data = (uint8_t *)buffer;
    for (size_t i = 0; i < length; i++)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, data[i]);
    }
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);

    return length;
}

static size_t Read3(void *buffer, size_t length)
{
    if (buffer == NULL || length == 0) return 0;

    DeviceType *dev = &_DeviceTable[2];
    uint8_t *data = (uint8_t *)buffer;

    if (length == USART_RECEIVE_BUFFER_SIZE_3)
    {
        length = 0;
    }

    if (length > dev->ReceiveReadIndex)
    {
        size_t readLength = length - dev->ReceiveReadIndex;
        memcpy(data, &_Usart3_RxBuffer[dev->ReceiveReadIndex], readLength);
        dev->ReceiveReadIndex = length;
        return readLength;
    }
    else if (length < dev->ReceiveReadIndex)
    {
        size_t firstPart = USART_RECEIVE_BUFFER_SIZE_3 - dev->ReceiveReadIndex;
        memcpy(data, &_Usart3_RxBuffer[dev->ReceiveReadIndex], firstPart);
        memcpy(&data[firstPart], _Usart3_RxBuffer, length);
        dev->ReceiveReadIndex = length;
        return firstPart + length;
    }

    return 0;
}
static const struct
{
    bool (*Open)(uint32_t baudrate);
    void (*Close)(void);
    size_t (*Write)(const void *buffer, size_t length);
    size_t (*Read)(void *buffer, size_t length);
} _OperateTable[] =
    {
        {Open1, Close1, Write1, Read1},
        {Open2, Close2, Write2, Read2},
        {Open3, Close3, Write3, Read3},
};

#define _USART_COUNT (sizeof(_OperateTable) / sizeof(_OperateTable[0]))

static DeviceType _DeviceTable[_USART_COUNT] = {
    {.ReceiveReadIndex = 0, .IsOpen = false},
    {.ReceiveReadIndex = 0, .IsOpen = false},
    {.ReceiveReadIndex = 0, .IsOpen = false},
};

Usart_DeviceType Usart_GetDevice(Usart_IdType id)
{
    if (id >= _USART_COUNT)
    {
        return NULL;
    }
    return &_DeviceTable[id];
}
bool Usart_Open(Usart_DeviceType device, uint32_t baudrate, Usart_Received received, Usart_Transmitted transmitted)
{
    if (device == NULL)
    {
        return false;
    }
    DeviceType *dev = (DeviceType *)device;
    if (dev->IsOpen == true)
    {
        return true;
    }
    dev->Received = received;
    dev->Transmitted = transmitted;
    size_t id = dev - _DeviceTable;
    if (id >= _USART_COUNT)
    {
        return false;
    }
    if (!_OperateTable[id].Open(baudrate))
    {
        return false;
    }
    dev->IsOpen = true;
    return true;
}
void Usart_Close(Usart_DeviceType device)
{
    if (device == NULL)
    {
        return;
    }
    DeviceType *dev = (DeviceType *)device;
    if (dev->IsOpen == false)
    {
        return;
    }
    size_t id = dev - _DeviceTable;
    if (id >= _USART_COUNT)
    {
        return;
    }
    _OperateTable[id].Close();
    dev->IsOpen = false;
    dev->ReceiveReadIndex = 0;
}
size_t Usart_Write(Usart_DeviceType device, const void *buffer, size_t length)
{
    if (device == NULL || buffer == NULL || length == 0)
    {
        return 0;
    }
    DeviceType *dev = (DeviceType *)device;
    if (dev->IsOpen == false)
    {
        return 0;
    }
    size_t id = dev - _DeviceTable;
    if (id >= _USART_COUNT)
    {
        return 0;
    }
    return _OperateTable[id].Write(buffer, length);
}
size_t Usart_Read(Usart_DeviceType device, void *buffer, size_t length)
{
    if (device == NULL || buffer == NULL || length == 0)
    {
        return 0;
    }
    DeviceType *dev = (DeviceType *)device;
    if (dev->IsOpen == false)
    {
        return 0;
    }
    size_t id = dev - _DeviceTable;
    if (id >= _USART_COUNT)
    {
        return 0;
    }
    return _OperateTable[id].Read(buffer, length);
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        uint16_t temp = USART1->SR;
        temp = USART1->DR;
        (void)temp;

        DeviceType *dev = &_DeviceTable[0];
        if (dev->IsOpen == false)
        {
            return;
        }

        size_t writeIndex = USART_RECEIVE_BUFFER_SIZE_1 - DMA_GetCurrDataCounter(DMA1_Channel5);

        if (dev->Received != NULL)
        {
            dev->Received(dev, writeIndex);
        }
    }
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        uint16_t temp = USART2->SR;
        temp = USART2->DR;
        (void)temp;

        DeviceType *dev = &_DeviceTable[1];
        if (dev->IsOpen == false)
        {
            return;
        }

        size_t writeIndex = USART_RECEIVE_BUFFER_SIZE_2 - DMA_GetCurrDataCounter(DMA1_Channel6);

        if (dev->Received != NULL)
        {
            dev->Received(dev, writeIndex);
        }
    }
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {
        uint16_t temp = USART3->SR;
        temp = USART3->DR;
        (void)temp;

        DeviceType *dev = &_DeviceTable[2];
        if (dev->IsOpen == false)
        {
            return;
        }

        size_t writeIndex = USART_RECEIVE_BUFFER_SIZE_3 - DMA_GetCurrDataCounter(DMA1_Channel3);

        if (dev->Received != NULL)
        {
            dev->Received(dev, writeIndex);
        }
    }
}
