#include "stm32f10x.h"
#include "Usart.h"
#include "Core.h"
#include <string.h>

/* 内部缓冲区大小 */
#define _USART_BUFFER_SIZE_1 (256 * 2)
#define _USART_BUFFER_SIZE_2 (256 * 2)
#define _USART_BUFFER_SIZE_3 (256 * 2)

/* IRQ handlers (declared here to satisfy -Wmissing-prototypes) */
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

struct Usart_Device
{
    uint32_t BaudRate;
    size_t ReceiveReadIndex;
    Usart_HandlerType Received;
    Usart_HandlerType Transmitted;
    uint16_t WordLength;
    uint16_t Parity;
    uint16_t StopBits;
    bool IsOpen;
    uint8_t _reserved;
};

static uint8_t _Usart1_RxBuffer[_USART_BUFFER_SIZE_1];
static uint8_t _Usart2_RxBuffer[_USART_BUFFER_SIZE_2];
static uint8_t _Usart3_RxBuffer[_USART_BUFFER_SIZE_3];

static uint8_t _Usart1_TxBuffer[_USART_BUFFER_SIZE_1];
static uint8_t _Usart2_TxBuffer[_USART_BUFFER_SIZE_2];
static uint8_t _Usart3_TxBuffer[_USART_BUFFER_SIZE_3];

static struct Usart_Device _DeviceTable[3];

static bool Open1(struct Usart_Device *dev, uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_InitTypeDef DMA_TX_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitStructure = (GPIO_InitTypeDef){0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure = (USART_InitTypeDef){0};
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = dev->WordLength;
    USART_InitStructure.USART_StopBits = dev->StopBits;
    USART_InitStructure.USART_Parity = dev->Parity;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart1_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = _USART_BUFFER_SIZE_1;
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

    DMA_DeInit(DMA1_Channel4);
    DMA_TX_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_TX_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart1_TxBuffer;
    DMA_TX_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_TX_InitStructure.DMA_BufferSize = 0;
    DMA_TX_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_TX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_TX_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_TX_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_TX_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_TX_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_TX_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_TX_InitStructure);

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
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
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
    USART_DeInit(USART1);
}

static size_t Write1(const void *buffer, size_t length)
{
    const uint8_t *data;
    struct Usart_Device *dev;

    if (buffer == NULL || length == 0) return 0;
    if (length > _USART_BUFFER_SIZE_1) length = _USART_BUFFER_SIZE_1;

    dev = &_DeviceTable[0];
    data = (const uint8_t *)buffer;

    /* 等待上一次 TX DMA 传输完成 */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
    DMA_ClearFlag(DMA1_FLAG_TC4);

    /* 拷贝数据到 TX 缓冲区并启动 DMA 发送 */
    memcpy(_Usart1_TxBuffer, data, length);

    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA1_Channel4->CNDTR = (uint16_t)length;
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel4, ENABLE);

    /* 等待 DMA 传输完成 */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

    DMA_Cmd(DMA1_Channel4, DISABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);

    /* 确保 USART 最后字节移位完成 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    if (dev->Transmitted != NULL)
    {
        dev->Transmitted(dev, length);
    }

    return length;
}

static size_t Read1(void *buffer, size_t length)
{
    struct Usart_Device *dev = &_DeviceTable[0];
    uint8_t *data = (uint8_t *)buffer;

    if (buffer == NULL || length == 0) return 0;

    if (length == _USART_BUFFER_SIZE_1)
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
        size_t firstPart = _USART_BUFFER_SIZE_1 - dev->ReceiveReadIndex;
        memcpy(data, &_Usart1_RxBuffer[dev->ReceiveReadIndex], firstPart);
        memcpy(&data[firstPart], _Usart1_RxBuffer, length);
        dev->ReceiveReadIndex = length;
        return firstPart + length;
    }

    return 0;
}

static bool Open2(struct Usart_Device *dev, uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_InitTypeDef DMA_TX_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitStructure = (GPIO_InitTypeDef){0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure = (USART_InitTypeDef){0};
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = dev->WordLength;
    USART_InitStructure.USART_StopBits = dev->StopBits;
    USART_InitStructure.USART_Parity = dev->Parity;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    DMA_InitStructure = (DMA_InitTypeDef){0};
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart2_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = _USART_BUFFER_SIZE_2;
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

    DMA_DeInit(DMA1_Channel7);
    DMA_TX_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
    DMA_TX_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart2_TxBuffer;
    DMA_TX_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_TX_InitStructure.DMA_BufferSize = 0;
    DMA_TX_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_TX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_TX_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_TX_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_TX_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_TX_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_TX_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_TX_InitStructure);

    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    NVIC_InitStructure = (NVIC_InitTypeDef){0};
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
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_Cmd(DMA1_Channel6, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
    USART_DeInit(USART2);
}

static size_t Write2(const void *buffer, size_t length)
{
    const uint8_t *data;
    struct Usart_Device *dev;

    if (buffer == NULL || length == 0) return 0;
    if (length > _USART_BUFFER_SIZE_2) length = _USART_BUFFER_SIZE_2;

    dev = &_DeviceTable[1];
    data = (const uint8_t *)buffer;

    /* 等待上一次 TX DMA 传输完成 */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);
    DMA_ClearFlag(DMA1_FLAG_TC7);

    /* 拷贝数据到 TX 缓冲区并启动 DMA 发送 */
    memcpy(_Usart2_TxBuffer, data, length);

    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA1_Channel7->CNDTR = (uint16_t)length;
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel7, ENABLE);

    /* 等待 DMA 传输完成 */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);

    DMA_Cmd(DMA1_Channel7, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);

    /* 确保 USART 最后字节移位完成 */
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);

    if (dev->Transmitted != NULL)
    {
        dev->Transmitted(dev, length);
    }

    return length;
}

static size_t Read2(void *buffer, size_t length)
{
    struct Usart_Device *dev = &_DeviceTable[1];
    uint8_t *data = (uint8_t *)buffer;

    if (buffer == NULL || length == 0) return 0;

    if (length == _USART_BUFFER_SIZE_2)
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
        size_t firstPart = _USART_BUFFER_SIZE_2 - dev->ReceiveReadIndex;
        memcpy(data, &_Usart2_RxBuffer[dev->ReceiveReadIndex], firstPart);
        memcpy(&data[firstPart], _Usart2_RxBuffer, length);
        dev->ReceiveReadIndex = length;
        return firstPart + length;
    }

    return 0;
}

static bool Open3(struct Usart_Device *dev, uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_InitTypeDef DMA_TX_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitStructure = (GPIO_InitTypeDef){0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure = (USART_InitTypeDef){0};
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = dev->WordLength;
    USART_InitStructure.USART_StopBits = dev->StopBits;
    USART_InitStructure.USART_Parity = dev->Parity;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    DMA_InitStructure = (DMA_InitTypeDef){0};
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart3_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = _USART_BUFFER_SIZE_3;
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

    DMA_TX_InitStructure = (DMA_InitTypeDef){0};
    DMA_DeInit(DMA1_Channel2);
    DMA_TX_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
    DMA_TX_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_Usart3_TxBuffer;
    DMA_TX_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_TX_InitStructure.DMA_BufferSize = 0;
    DMA_TX_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_TX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_TX_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_TX_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_TX_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_TX_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_TX_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_TX_InitStructure);

    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    NVIC_InitStructure = (NVIC_InitTypeDef){0};
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
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_Cmd(DMA1_Channel3, DISABLE);
    USART_DMACmd(USART3, USART_DMAReq_Tx, DISABLE);
    USART_DMACmd(USART3, USART_DMAReq_Rx, DISABLE);
    USART_DeInit(USART3);
}

static size_t Write3(const void *buffer, size_t length)
{
    const uint8_t *data;
    struct Usart_Device *dev;

    if (buffer == NULL || length == 0) return 0;
    if (length > _USART_BUFFER_SIZE_3) length = _USART_BUFFER_SIZE_3;

    dev = &_DeviceTable[2];
    data = (const uint8_t *)buffer;

    /* 等待上一次 TX DMA 传输完成 */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);
    DMA_ClearFlag(DMA1_FLAG_TC2);

    /* 拷贝数据到 TX 缓冲区并启动 DMA 发送 */
    memcpy(_Usart3_TxBuffer, data, length);

    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA1_Channel2->CNDTR = (uint16_t)length;
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    /* 等待 DMA 传输完成 */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);

    DMA_Cmd(DMA1_Channel2, DISABLE);
    USART_DMACmd(USART3, USART_DMAReq_Tx, DISABLE);

    /* 确保 USART 最后字节移位完成 */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);

    if (dev->Transmitted != NULL)
    {
        dev->Transmitted(dev, length);
    }

    return length;
}

static size_t Read3(void *buffer, size_t length)
{
    struct Usart_Device *dev = &_DeviceTable[2];
    uint8_t *data = (uint8_t *)buffer;

    if (buffer == NULL || length == 0) return 0;

    if (length == _USART_BUFFER_SIZE_3)
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
        size_t firstPart = _USART_BUFFER_SIZE_3 - dev->ReceiveReadIndex;
        memcpy(data, &_Usart3_RxBuffer[dev->ReceiveReadIndex], firstPart);
        memcpy(&data[firstPart], _Usart3_RxBuffer, length);
        dev->ReceiveReadIndex = length;
        return firstPart + length;
    }

    return 0;
}

static const struct
{
    bool (*Open)(struct Usart_Device *dev, uint32_t baudrate);
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

static struct Usart_Device _DeviceTable[_USART_COUNT] = {
    {.ReceiveReadIndex = 0, .IsOpen = false, .WordLength = USART_WordLength_8b, .Parity = USART_Parity_No, .StopBits = USART_StopBits_1},
    {.ReceiveReadIndex = 0, .IsOpen = false, .WordLength = USART_WordLength_8b, .Parity = USART_Parity_No, .StopBits = USART_StopBits_1},
    {.ReceiveReadIndex = 0, .IsOpen = false, .WordLength = USART_WordLength_8b, .Parity = USART_Parity_No, .StopBits = USART_StopBits_1},
};

Usart_DeviceType Usart_GetDevice(const char *name)
{
    uint8_t port;
    size_t index;
    struct Usart_Device *dev;
    const char *p;
    uint8_t dataBits;
    char parity;
    uint8_t stopBits;

    if (name == NULL) return NULL;

    /* 解析格式: COMx,<baudrate>,<data>,<parity>,<stop>  例如 COM1,9600,8,N,1 */
    if (name[0] != 'C' || name[1] != 'O' || name[2] != 'M') return NULL;

    port = (uint8_t)(name[3] - '0');
    if (port < 1 || port > 3) return NULL;
    index = port - 1;

    dev = &_DeviceTable[index];

    /* 定位4个逗号分隔的字段: baudrate, data, parity, stop */
    p = name + 4;
    if (*p != ',') return dev;
    p++;

    /* 解析 baudrate */
    dev->BaudRate = 0;
    while (*p >= '0' && *p <= '9')
    {
        dev->BaudRate = dev->BaudRate * 10 + (uint32_t)(*p - '0');
        p++;
    }

    /* 解析 data bits */
    if (*p != ',') return dev;
    p++;
    dataBits = (uint8_t)(*p - '0');
    dev->WordLength = (dataBits == 9) ? USART_WordLength_9b : USART_WordLength_8b;

    /* 解析 parity */
    p++;
    if (*p != ',') return dev;
    p++;
    parity = *p;
    if (parity == 'E' || parity == 'e')
        dev->Parity = USART_Parity_Even;
    else if (parity == 'O' || parity == 'o')
        dev->Parity = USART_Parity_Odd;
    else
        dev->Parity = USART_Parity_No;

    /* 解析 stop bits */
    p++;
    if (*p != ',') return dev;
    p++;
    stopBits = (uint8_t)(*p - '0');
    if (stopBits == 2)
        dev->StopBits = USART_StopBits_2;
    else if (stopBits == 0)
    {
        p++;
        if (*p == '.' && *(p + 1) == '5')
            dev->StopBits = USART_StopBits_0_5;
    }
    else if (stopBits == 1)
    {
        p++;
        if (*p == '.' && *(p + 1) == '5')
            dev->StopBits = USART_StopBits_1_5;
    }

    return dev;
}

int Usart_SetHandler(Usart_DeviceType device, Usart_HandlerType received, Usart_HandlerType transmitted)
{
    if (device == NULL) return USART_ERROR_INVALID_ARGUMENT;

    device->Received = received;
    device->Transmitted = transmitted;

    return USART_ERROR_SUCCESS;
}

int Usart_Open(Usart_DeviceType device)
{
    size_t id;

    if (device == NULL) return USART_ERROR_INVALID_HANDLE;

    if (device->IsOpen) return USART_ERROR_ALREADY_OPEN;

    id = (size_t)(device - _DeviceTable);
    if (id >= _USART_COUNT) return USART_ERROR_INVALID_HANDLE;

    if (!_OperateTable[id].Open(device, device->BaudRate)) return USART_ERROR_OPEN_FAILED;

    device->IsOpen = true;
    return USART_ERROR_SUCCESS;
}

int Usart_Close(Usart_DeviceType device)
{
    size_t id;

    if (device == NULL) return USART_ERROR_INVALID_HANDLE;

    if (!device->IsOpen) return USART_ERROR_NOT_OPEN;

    id = (size_t)(device - _DeviceTable);
    if (id >= _USART_COUNT) return USART_ERROR_INVALID_HANDLE;

    _OperateTable[id].Close();
    device->IsOpen = false;
    device->ReceiveReadIndex = 0;

    return USART_ERROR_SUCCESS;
}

size_t Usart_Write(Usart_DeviceType device, const void *buffer, size_t length)
{
    size_t id;

    if (device == NULL || buffer == NULL || length == 0) return 0;

    if (!device->IsOpen) return 0;

    id = (size_t)(device - _DeviceTable);
    if (id >= _USART_COUNT) return 0;

    return _OperateTable[id].Write(buffer, length);
}

size_t Usart_Read(Usart_DeviceType device, void *buffer, size_t length)
{
    size_t id;

    if (device == NULL || buffer == NULL || length == 0) return 0;

    if (!device->IsOpen) return 0;

    id = (size_t)(device - _DeviceTable);
    if (id >= _USART_COUNT) return 0;

    return _OperateTable[id].Read(buffer, length);
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        struct Usart_Device *dev = &_DeviceTable[0];
        size_t writeIndex;
        uint16_t temp;

        temp = USART1->SR;
        temp = USART1->DR;
        (void)temp;

        if (!dev->IsOpen) return;

        writeIndex = _USART_BUFFER_SIZE_1 - DMA_GetCurrDataCounter(DMA1_Channel5);

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
        struct Usart_Device *dev = &_DeviceTable[1];
        size_t writeIndex;
        uint16_t temp;

        temp = USART2->SR;
        temp = USART2->DR;
        (void)temp;

        if (!dev->IsOpen) return;

        writeIndex = _USART_BUFFER_SIZE_2 - DMA_GetCurrDataCounter(DMA1_Channel6);

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
        struct Usart_Device *dev = &_DeviceTable[2];
        size_t writeIndex;
        uint16_t temp;

        temp = USART3->SR;
        temp = USART3->DR;
        (void)temp;

        if (!dev->IsOpen) return;

        writeIndex = _USART_BUFFER_SIZE_3 - DMA_GetCurrDataCounter(DMA1_Channel3);

        if (dev->Received != NULL)
        {
            dev->Received(dev, writeIndex);
        }
    }
}
