#pragma once
#include <stdint.h>

/*
 * 多编译器 & 多MCU 临界区宏
 * 所有 Cortex-M 的 PRIMASK 操作一致，无需区分 MCU 型号。
 */

/* CMSIS 路径 - STM32 项目始终可用，覆盖所有编译器 */
#if defined(__CORTEX_M)
#define PORT_GET_PRIMASK(s) ((s) = __get_PRIMASK())
#define PORT_SET_PRIMASK(s) __set_PRIMASK(s)
#define PORT_DISABLE_IRQ() __disable_irq()

/* GCC / ARMCLANG (Keil AC6) */
#elif defined(__GNUC__) || (defined(__clang__) && defined(__ARMCC_VERSION))
#define PORT_GET_PRIMASK(s) __asm__ __volatile__("mrs %0, primask" : "=r"(s))
#define PORT_SET_PRIMASK(s) __asm__ __volatile__("msr primask, %0" ::"r"(s))
#define PORT_DISABLE_IRQ() __asm__ __volatile__("cpsid i" ::: "memory")

/* IAR EWARM */
#elif defined(__ICCARM__)
#define PORT_GET_PRIMASK(s) __asm volatile("MRS %0, PRIMASK" : "=r"(s))
#define PORT_SET_PRIMASK(s) __asm volatile("MSR PRIMASK, %0" ::"r"(s))
#define PORT_DISABLE_IRQ() __asm volatile("CPSID I")

/* ARMCC (Keil AC5) - 不支持宏内联变量，必须走 CMSIS */
#elif defined(__CC_ARM)
#error "[Port.h] ARMCC v5 requires CMSIS. Include core_cm3.h before Port.h."
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /* 内联函数版临界区 - 推荐使用 */
    static inline uint32_t Port_Lock(void)
    {
        uint32_t state;
        PORT_GET_PRIMASK(state);
        PORT_DISABLE_IRQ();
        return state;
    }

    static inline void Port_Unlock(uint32_t state)
    {
        PORT_SET_PRIMASK(state);
    }

#ifdef __cplusplus
}
#endif
