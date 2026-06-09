#include <stddef.h>
#include "stm32f10x.h"
#include "Timer.h"

typedef struct
{
    TIM_TypeDef *instance;
    uint32_t period;
    uint32_t repeat;
    uint32_t current_count;
    Timer_TimedType timed;
    bool is_open;
    bool is_running;
} Timer_ContextType;

static Timer_ContextType Timer_Context[TIMER_ID_COUNT] = {0};

static TIM_TypeDef *Timer_GetInstance(Timer_IdType id)
{
    switch (id)
    {
        case TIMER_ID_1: return TIM1;
        case TIMER_ID_2: return TIM2;
        case TIMER_ID_3: return TIM3;
        case TIMER_ID_4: return TIM4;       
        default: return NULL;
    }
}

static IRQn_Type Timer_GetIRQn(Timer_IdType id)
{
    switch (id)
    {
        case TIMER_ID_1: return TIM1_UP_IRQn;
        case TIMER_ID_2: return TIM2_IRQn;
        case TIMER_ID_3: return TIM3_IRQn;
        case TIMER_ID_4: return TIM4_IRQn; 
        default: return (IRQn_Type)(-1);
    }
}

static void Timer_IRQHandler_Internal(Timer_IdType id)
{
    if (id >= TIMER_ID_COUNT) return;

    TIM_TypeDef *tim = Timer_GetInstance(id);
    if (tim == NULL) return;

    if (tim->SR & TIM_SR_UIF)
    {
        tim->SR = ~TIM_SR_UIF;

        Timer_ContextType *ctx = &Timer_Context[id];
        if (ctx->timed != NULL)
        {
            ctx->timed(id, NULL);
        }

        if (ctx->repeat != 0)
        {
            ctx->current_count++;
            if (ctx->current_count >= ctx->repeat)
            {
                Timer_Stop(id);
            }
        }
    }
}

void TIM1_UP_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_1);
}

void TIM2_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_2);
}

void TIM3_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_3);
}

void TIM4_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_4);
}

void TIM5_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_5);
}

void TIM6_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_6);
}

void TIM7_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_7);
}

void TIM8_UP_IRQHandler(void)
{
    Timer_IRQHandler_Internal(TIMER_ID_8);
}

bool Timer_Open(Timer_IdType id, uint32_t period, uint32_t repeat, Timer_TimedType timed)
{
    if (id >= TIMER_ID_COUNT) return false;

    TIM_TypeDef *tim = Timer_GetInstance(id);
    if (tim == NULL) return false;

    Timer_ContextType *ctx = &Timer_Context[id];

    if (ctx->is_open) return false;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_TIM8, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4 |
                           RCC_APB1Periph_TIM5 | RCC_APB1Periph_TIM6 | RCC_APB1Periph_TIM7, ENABLE);

    TIM_TimeBaseInitTypeDef tim_init;
    tim_init.TIM_Period = period - 1;
    tim_init.TIM_Prescaler = (72000000 / 1000000) - 1;
    tim_init.TIM_ClockDivision = 0;
    tim_init.TIM_CounterMode = TIM_CounterMode_Up;
    tim_init.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(tim, &tim_init);

    ctx->period = period;
    ctx->repeat = repeat;
    ctx->current_count = 0;
    ctx->timed = timed;
    ctx->is_open = true;
    ctx->is_running = false;

    return true;
}

void Timer_Close(Timer_IdType id)
{
    if (id >= TIMER_ID_COUNT) return;

    TIM_TypeDef *tim = Timer_GetInstance(id);
    if (tim == NULL) return;

    Timer_ContextType *ctx = &Timer_Context[id];

    Timer_Stop(id);

    TIM_DeInit(tim);

    ctx->is_open = false;
    ctx->timed = NULL;
}

bool Timer_Start(Timer_IdType id)
{
    if (id >= TIMER_ID_COUNT) return false;

    TIM_TypeDef *tim = Timer_GetInstance(id);
    if (tim == NULL) return false;

    Timer_ContextType *ctx = &Timer_Context[id];

    if (!ctx->is_open || ctx->is_running) return false;

    ctx->current_count = 0;

    TIM_ITConfig(tim, TIM_IT_Update, ENABLE);

    TIM_Cmd(tim, ENABLE);

    IRQn_Type irqn = Timer_GetIRQn(id);
    if (irqn != (IRQn_Type)(-1))
    {
        NVIC_SetPriority(irqn, 0);
        NVIC_EnableIRQ(irqn);
    }

    ctx->is_running = true;
    return true;
}

void Timer_Stop(Timer_IdType id)
{
    if (id >= TIMER_ID_COUNT) return;

    TIM_TypeDef *tim = Timer_GetInstance(id);
    if (tim == NULL) return;

    Timer_ContextType *ctx = &Timer_Context[id];

    if (!ctx->is_open) return;

    TIM_Cmd(tim, DISABLE);

    TIM_ITConfig(tim, TIM_IT_Update, DISABLE);

    IRQn_Type irqn = Timer_GetIRQn(id);
    if (irqn != (IRQn_Type)(-1))
    {
        NVIC_DisableIRQ(irqn);
    }

    ctx->is_running = false;
}