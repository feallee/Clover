#include "Application.h"
#include "SysTick.h"

static SysTick_DeviceType *_SysTick = NULL;
static void Init(const Application_MessageType *message)
{
    (void)message;
}
APPLICATION_REGISTER_HANDLER_L1(APPLICATION_MESSAGE_ID_INIT, Init);
void Open(const Application_MessageType *message)
{
    (void)message;
    _SysTick = SysTick_Open("SYSTICK", "100ms");
}
APPLICATION_REGISTER_HANDLER_L2(APPLICATION_MESSAGE_ID_OPEN, Open);

void Idle(const Application_MessageType *message)
{
    uint32_t v;
    (void)message;
    SysTick_Read(_SysTick, &v);
}
APPLICATION_REGISTER_HANDLER_L2(APPLICATION_MESSAGE_ID_IDLE, Idle);
