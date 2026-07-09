#include "Application.h"
#include "SysTick.h"

static SysTick_DeviceType *_SysTick = NULL;
static void Init(Application_MessageType *message)
{
}
APPLICATION_REGISTER_HANDLER_L1(APPLICATION_MESSAGE_ID_INIT, Init);
void Open(Application_MessageType *message)
{
    _SysTick = SysTick_Open("SYSTICK", "100ms");
}
APPLICATION_REGISTER_HANDLER_L2(APPLICATION_MESSAGE_ID_OPEN, Open);

void Idle(Application_MessageType *message)
{
    uint32_t v;
    SysTick_Read(_SysTick, &v);
}
APPLICATION_REGISTER_HANDLER_L2(APPLICATION_MESSAGE_ID_IDLE, Idle);