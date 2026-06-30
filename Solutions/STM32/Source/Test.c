#include "Application.h"
#include "RealTimer.h"
#include "Power.h"
#include "Core.h"
#include "Usart.h"
#include "Config.h"

#define _TEST_USART_BUFFER_SIZE (256 * 2)

static RealTimer_DeviceType rtc = NULL;
static Usart_DeviceType usart1 = NULL;
static uint8_t _TxBuffer[_TEST_USART_BUFFER_SIZE];

/* USART1接收回调 */
static void Usart1Received(Usart_DeviceType device, size_t length)
{
    Application_MessageType msg;
    (void)device;
    msg = (Application_MessageType){
        .ID = APPLICATION_MESSAGE_ID_USART1_RECEIVED,
        .WParam = (uint16_t)length
    };
    Application_PostMessage(&msg);
}

/* USART1发送回调 */
static void Usart1Transmitted(Usart_DeviceType device, size_t length)
{
    (void)device;
    (void)length;
}

/* RTC闹钟回调 */
static void RTCAlarmed(RealTimer_DeviceType device, void *parameter)
{
    Application_MessageType msg;
    (void)device;
    (void)parameter;
    msg = (Application_MessageType){.ID = APPLICATION_MESSAGE_ID_RTC_TIMED};
    Application_PostMessage(&msg);
}

static void OpenDevice(Application_MessageType *message)
{
    (void)message;
    rtc = RealTimer_GetDevice(REALTIMER_ID_1);
    if (rtc != NULL)
    {
        if (RealTimer_Open(rtc, RTCAlarmed) == true)
        {
        }
    }
    usart1 = Usart_GetDevice("COM1,115200,8,N,1");
    if (usart1 != NULL)
    {
        Usart_SetHandler(usart1, Usart1Received, Usart1Transmitted);
        Usart_Open(usart1);
    }
#if APPLICATION_DEBUG_MODE == 1
    Core_EnableDebug();
#endif
}
APPLICATION_REG_MESSAGE_HANDLER_L1(APPLICATION_MESSAGE_ID_OPEN, OpenDevice);

static void Timed1(Application_MessageType *message)
{
    (void)message;
    if (rtc != NULL)
    {
    }
}
APPLICATION_REG_MESSAGE_HANDLER_L1(APPLICATION_MESSAGE_ID_RTC_TIMED, Timed1);

static void Idle(Application_MessageType *message)
{
    static uint8_t d = 0;
    (void)message;
    if (++d == 10)
    {
        d = 0;

       //Power_EnterMode(POWER_MODE_SLEEP);
    }
}
APPLICATION_REG_MESSAGE_HANDLER_L1(APPLICATION_MESSAGE_ID_IDLE, Idle);

static void Usart1ReceivedHandler(Application_MessageType *message)
{
    if (usart1 != NULL && message != NULL)
    {
        size_t writeIndex = message->WParam;
        size_t readCount = Usart_Read(usart1, _TxBuffer, writeIndex);
        if (readCount > 0)
        {
            Usart_Write(usart1, _TxBuffer, readCount);
        }
    }
}
APPLICATION_REG_MESSAGE_HANDLER_L1(APPLICATION_MESSAGE_ID_USART1_RECEIVED, Usart1ReceivedHandler);
