#include <stddef.h>
#include <stdbool.h>
#include "stm32f10x.h"
#include "RealTimer.h"

/* RTC 预分频值配置 */
#define RTC_PRESCALER_REAL 32767 /* 真实硬件：32768Hz LSE */
#define RTC_PRESCALER_SIM 9829   /* 仿真环境：~10922Hz (调整此值匹配实际仿真时钟) */

typedef struct
{
    RealTimer_AlarmedType Alarmed;
    bool IsOpen;
} DeviceType;

static DeviceType _DeviceTable[1] = {0}; /*由于硬件只有1个RTC*/

RealTimer_DeviceType RealTimer_GetDevice(RealTimer_IdType id)
{
    if (id != REALTIMER_ID_1)
    {
        return NULL;
    }
    return &_DeviceTable[REALTIMER_ID_1];
}

bool RealTimer_Open(RealTimer_DeviceType device, RealTimer_AlarmedType alarmed)
{
    if (device == NULL)
    {
        return false;
    }

    DeviceType *dev = (DeviceType *)device;
    dev->Alarmed = alarmed;
    dev->IsOpen = true;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if (PWR_GetFlagStatus(PWR_FLAG_SB) != RESET)
    {
        PWR_ClearFlag(PWR_FLAG_SB);
        RTC_WaitForSynchro();
    }
    else
    {
        BKP_DeInit();
        RCC_LSEConfig(RCC_LSE_ON);
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        RCC_RTCCLKCmd(ENABLE);
        RTC_WaitForSynchro();
        RTC_SetPrescaler(RTC_PRESCALER_REAL);
        RTC_WaitForLastTask();
    }

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    RTC_WaitForLastTask();
    RTC_SetAlarm(RTC_GetCounter() + 1);
    RTC_WaitForLastTask();

    RTC_ClearFlag(RTC_FLAG_RSF);
    RTC_WaitForLastTask();

    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    RTC_WaitForLastTask();

    return true;
}

void RealTimer_Close(RealTimer_DeviceType device)
{
    if (device == NULL)
    {
        return;
    }

    DeviceType *dev = (DeviceType *)device;
    dev->IsOpen = false;
    dev->Alarmed = NULL;

    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    RTC_WaitForLastTask();

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

bool RealTimer_Read(RealTimer_DeviceType device, uint32_t *counter)
{
    if (device == NULL || counter == NULL)
    {
        return false;
    }

    DeviceType *dev = (DeviceType *)device;
    if (!dev->IsOpen)
    {
        return false;
    }

    RTC_WaitForSynchro();
    RTC_WaitForLastTask();
    *counter = RTC_GetCounter();
    RTC_WaitForLastTask();

    return true;
}

bool RealTimer_Write(RealTimer_DeviceType device, uint32_t counter)
{
    if (device == NULL)
    {
        return false;
    }

    DeviceType *dev = (DeviceType *)device;
    if (!dev->IsOpen)
    {
        return false;
    }

    RTC_WaitForLastTask();
    RTC_SetCounter(counter);
    RTC_WaitForLastTask();

    return true;
}

void RTCAlarm_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line17);

    if (PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
    {
        PWR_ClearFlag(PWR_FLAG_WU);
    }

    RTC_WaitForLastTask();
    RTC_ClearITPendingBit(RTC_IT_ALR);
    RTC_WaitForLastTask();

    RTC_WaitForLastTask();
    RTC_SetAlarm(RTC_GetCounter() + 1);
    RTC_WaitForLastTask();

    RTC_ClearFlag(RTC_FLAG_RSF);
    RTC_WaitForLastTask();
    
    DeviceType *dev = &_DeviceTable[0];
    if (dev->IsOpen && dev->Alarmed != NULL)
    {
        dev->Alarmed(dev, NULL);
    }
}
