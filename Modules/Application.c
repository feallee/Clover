/**
 * @file Application.c
 * @copyright Copyright (c) 2026 weas.top team. All rights reserved.
 * @brief 消息驱动型裸机应用程序框架的实现文件。
 *
 * 本文件实现了基于消息队列的事件驱动框架核心功能，包括：
 * - 消息队列管理（入队/出队）
 * - 消息处理器执行机制
 * - 主循环调度
 *
 * @note 依赖 Core.h 提供临界区保护功能。
 */

#include "Core.h"
#include "Application.h"

#if 1 /* 消息处理器哨兵和接口 */
#define _LIST8(X) \
    X(0)          \
    X(1)          \
    X(2)          \
    X(3)          \
    X(4)          \
    X(5)          \
    X(6)          \
    X(7)

#define _LIST16(X) \
    _LIST8(X)      \
    X(8)           \
    X(9)           \
    X(10)          \
    X(11)          \
    X(12)          \
    X(13)          \
    X(14)          \
    X(15)

#define _LIST32(X) \
    _LIST16(X)     \
    X(16)          \
    X(17)          \
    X(18)          \
    X(19)          \
    X(20)          \
    X(21)          \
    X(22)          \
    X(23)          \
    X(24)          \
    X(25)          \
    X(26)          \
    X(27)          \
    X(28)          \
    X(29)          \
    X(30)          \
    X(31)

#define _LIST64(X) \
    _LIST32(X)     \
    X(32)          \
    X(33)          \
    X(34)          \
    X(35)          \
    X(36)          \
    X(37)          \
    X(38)          \
    X(39)          \
    X(40)          \
    X(41)          \
    X(42)          \
    X(43)          \
    X(44)          \
    X(45)          \
    X(46)          \
    X(47)          \
    X(48)          \
    X(49)          \
    X(50)          \
    X(51)          \
    X(52)          \
    X(53)          \
    X(54)          \
    X(55)          \
    X(56)          \
    X(57)          \
    X(58)          \
    X(59)          \
    X(60)          \
    X(61)          \
    X(62)          \
    X(63)

#define _LIST128(X) \
    _LIST64(X)      \
    X(64)           \
    X(65)           \
    X(66)           \
    X(67)           \
    X(68)           \
    X(69)           \
    X(70)           \
    X(71)           \
    X(72)           \
    X(73)           \
    X(74)           \
    X(75)           \
    X(76)           \
    X(77)           \
    X(78)           \
    X(79)           \
    X(80)           \
    X(81)           \
    X(82)           \
    X(83)           \
    X(84)           \
    X(85)           \
    X(86)           \
    X(87)           \
    X(88)           \
    X(89)           \
    X(90)           \
    X(91)           \
    X(92)           \
    X(93)           \
    X(94)           \
    X(95)           \
    X(96)           \
    X(97)           \
    X(98)           \
    X(99)           \
    X(100)          \
    X(101)          \
    X(102)          \
    X(103)          \
    X(104)          \
    X(105)          \
    X(106)          \
    X(107)          \
    X(108)          \
    X(109)          \
    X(110)          \
    X(111)          \
    X(112)          \
    X(113)          \
    X(114)          \
    X(115)          \
    X(116)          \
    X(117)          \
    X(118)          \
    X(119)          \
    X(120)          \
    X(121)          \
    X(122)          \
    X(123)          \
    X(124)          \
    X(125)          \
    X(126)          \
    X(127)

#define _LIST256(X) \
    _LIST128(X)     \
    X(128)          \
    X(129)          \
    X(130)          \
    X(131)          \
    X(132)          \
    X(133)          \
    X(134)          \
    X(135)          \
    X(136)          \
    X(137)          \
    X(138)          \
    X(139)          \
    X(140)          \
    X(141)          \
    X(142)          \
    X(143)          \
    X(144)          \
    X(145)          \
    X(146)          \
    X(147)          \
    X(148)          \
    X(149)          \
    X(150)          \
    X(151)          \
    X(152)          \
    X(153)          \
    X(154)          \
    X(155)          \
    X(156)          \
    X(157)          \
    X(158)          \
    X(159)          \
    X(160)          \
    X(161)          \
    X(162)          \
    X(163)          \
    X(164)          \
    X(165)          \
    X(166)          \
    X(167)          \
    X(168)          \
    X(169)          \
    X(170)          \
    X(171)          \
    X(172)          \
    X(173)          \
    X(174)          \
    X(175)          \
    X(176)          \
    X(177)          \
    X(178)          \
    X(179)          \
    X(180)          \
    X(181)          \
    X(182)          \
    X(183)          \
    X(184)          \
    X(185)          \
    X(186)          \
    X(187)          \
    X(188)          \
    X(189)          \
    X(190)          \
    X(191)          \
    X(192)          \
    X(193)          \
    X(194)          \
    X(195)          \
    X(196)          \
    X(197)          \
    X(198)          \
    X(199)          \
    X(200)          \
    X(201)          \
    X(202)          \
    X(203)          \
    X(204)          \
    X(205)          \
    X(206)          \
    X(207)          \
    X(208)          \
    X(209)          \
    X(210)          \
    X(211)          \
    X(212)          \
    X(213)          \
    X(214)          \
    X(215)          \
    X(216)          \
    X(217)          \
    X(218)          \
    X(219)          \
    X(220)          \
    X(221)          \
    X(222)          \
    X(223)          \
    X(224)          \
    X(225)          \
    X(226)          \
    X(227)          \
    X(228)          \
    X(229)          \
    X(230)          \
    X(231)          \
    X(232)          \
    X(233)          \
    X(234)          \
    X(235)          \
    X(236)          \
    X(237)          \
    X(238)          \
    X(239)          \
    X(240)          \
    X(241)          \
    X(242)          \
    X(243)          \
    X(244)          \
    X(245)          \
    X(246)          \
    X(247)          \
    X(248)          \
    X(249)          \
    X(250)          \
    X(251)          \
    X(252)          \
    X(253)          \
    X(254)          \
    X(255)

#if APPLICATION_MESSAGE_ID_RANGE == 8U
#define _LIST _LIST8
#elif APPLICATION_MESSAGE_ID_RANGE == 16U
#define _LIST _LIST16
#elif APPLICATION_MESSAGE_ID_RANGE == 32U
#define _LIST _LIST32
#elif APPLICATION_MESSAGE_ID_RANGE == 64U
#define _LIST _LIST64
#elif APPLICATION_MESSAGE_ID_RANGE == 128U
#define _LIST _LIST128
#elif APPLICATION_MESSAGE_ID_RANGE == 256U
#define _LIST _LIST256
#else
#error "Invalid APPLICATION_MESSAGE_ID_COUNT value"
#endif

#define _HANDLER_LEVEL_BEGIN 0
#define _HANDLER_LEVEL_END 9
#define _REG_HANDLER(n)                                        \
    _APPLICATION_REGISTER_HANDLER(n, _HANDLER_LEVEL_BEGIN, 0); \
    _APPLICATION_REGISTER_HANDLER(n, _HANDLER_LEVEL_END, 0);

_LIST(_REG_HANDLER);

typedef struct
{
    const Application_MessageHandlerType *Begin;
    const Application_MessageHandlerType *End;
} MessageTableType;

#define _REG_TABLE(n)                                              \
    {.Begin = &_APPLICATION_TO_MEMBER(n, _HANDLER_LEVEL_BEGIN, 0), \
     .End = &_APPLICATION_TO_MEMBER(n, _HANDLER_LEVEL_END, 0)},

static const MessageTableType _Table[] = {_LIST(_REG_TABLE)};

/**
 * @brief 执行指定消息 ID 的所有消息处理器。
 *
 * 根据消息 ID 查找对应的处理器表，按级别顺序(L1-L8)执行所有已注册的处理器。
 * 内部使用 L0 和 L9 作为哨兵标记处理器表的边界，用户实际使用 L1-L8。
 *
 * @param message 指向待处理消息的指针。
 */
static void ExecuteHandler(Application_MessageType *message)
{
    const MessageTableType *t = &_Table[message->ID];
    for (const Application_MessageHandlerType *h = t->Begin + 1; h < t->End; h++)
    {
        if (*h != NULL)
        {
            (*h)(message);
        }
    }
}

#endif

/**
 * @brief 消息队列结构。
 *
 * 使用环形缓冲区实现，Head 指向队头（下一个要读取的位置），
 * Tail 指向队尾（下一个要写入的位置）。队列容量必须是 2 的整数次幂，
 * 以便使用位掩码进行高效的取模运算。
 */
static struct
{
    volatile size_t Head;                                         ///< 读指针，指向下一个要读取的消息位置
    volatile size_t Tail;                                         ///< 写指针，指向下一个要写入的消息位置
    Application_MessageType Buffer[APPLICATION_MESSAGE_CAPACITY]; ///< 消息缓冲区（环形队列）
} _Queue = {0};

/**
 * @brief 队列容量位掩码。
 *
 * 用于将 Tail/Head 索引映射到缓冲区数组索引，要求容量必须是 2 的整数次幂。
 * 等效于 `index % APPLICATION_MESSAGE_CAPACITY`，但位运算更高效。
 */
#define _MESSAGE_CAPACITY_MASK (APPLICATION_MESSAGE_CAPACITY - 1ULL)

int Application_Run(void *parameter)
{
    (void)parameter;
    Application_MessageType msg = {0};
    msg.ID = APPLICATION_MESSAGE_ID_INIT;
    ExecuteHandler(&msg);
    msg.ID = APPLICATION_MESSAGE_ID_OPEN;
    ExecuteHandler(&msg);
    while (1)
    {
        msg.ID = APPLICATION_MESSAGE_ID_FEED;
        ExecuteHandler(&msg);
        {
            uint32_t mask = Core_EnterCritical();
            if (_Queue.Head == _Queue.Tail) /*队列为空*/
            {
                Core_ExitCritical(mask);
                msg.ID = APPLICATION_MESSAGE_ID_IDLE;
                ExecuteHandler(&msg);
            }
            else
            {
                msg = _Queue.Buffer[_Queue.Head & _MESSAGE_CAPACITY_MASK];
                _Queue.Head++;
                Core_ExitCritical(mask);
                ExecuteHandler(&msg);
            }
        }
    }
}

int Application_PostMessage(Application_MessageType *message)
{
    if (message == NULL)
    {
        return 0;
    }
    if (message->ID >= APPLICATION_MESSAGE_ID_RANGE)
    {
        return 0;
    }
    uint32_t mask = Core_EnterCritical();
    if (_Queue.Tail - _Queue.Head == APPLICATION_MESSAGE_CAPACITY) /*队列已满*/
    {
        Core_ExitCritical(mask);
        return 0;
    }
    else
    {
        _Queue.Buffer[_Queue.Tail & _MESSAGE_CAPACITY_MASK] = *message; /*消息最大16字节，直接赋值*/
        _Queue.Tail++;
        Core_ExitCritical(mask);
        return 1;
    }
}

int Application_SendMessage(Application_MessageType *message)
{
    if (message == NULL)
    {
        return 0;
    }
    if (message->ID >= APPLICATION_MESSAGE_ID_RANGE)
    {
        return 0;
    }
    ExecuteHandler(message);
    return 1;
}