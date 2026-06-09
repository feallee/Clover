/**
 * @file Mealy.h
 * @copyright Copyright (c) 2026 weas.top team. All rights reserved.
 * @brief Mealy 状态机库。
 * @details 提供一个轻量级的 Mealy 状态机实现，适用于嵌入式系统和其他 C/C++ 项目。
 * @warning 本库只支持 C99 及以上标准。
 * @warning 本库提供的所有接口不是线程安全的，在多线程环境中使用时注意保护状态机实例。
 * @warning 本库不支持递归状态机调用，即在状态机的回调函数中再次调用状态机接口。
 * @warning 本库在调用 Mealy_Raise 前必须确保已正确调用 Mealy_Start 启动了状态机，否则有可能出现未定义行为。
 * @section 状态索引布局（约定状态索引必须连续；状态数量 STATE_LENGTH 等于最终状态索引 STATE_FINAL），参考《用法示例》：
 * | 状态名称   | 建议状态符号  |状态索引           | 
 * | ---------- | ------------- | ----------------- | 
 * | 起始状态   | STATE_INITIAL | 0                 |
 * | 自定义状态 | (用户自定义)  | 1 ~ STATE_FINAL-1 |
 * | 最终状态   | STATE_FINAL   | >= STATE_FINAL    |
 *
 * @section 修订记录
 * | 版本 | 日期       | 作者                | 日志                                                                           |
 * | ---- | ---------- | ------------------- | ------------------------------------------------------------------------------ |
 * |    1 | 2025/12/30 | feallee@hotmail.com | 初版。                                                                         |
 *
 * @code 用法示例。
 *
 *  // 本示例演示有`播放暂停`和`停止`两个按键的播放器。
 *  //             +---------------+
 *  //             | STATE_INITIAL |
 *  //             +---------------+
 *  //                     |
 *  //                     |EVENT_STOP
 *  //                     v
 *  //             +---------------+
 *  //             | STATE_STOPPED |<-------------+
 *  //             +---------------+              |
 *  //                     |                      |
 *  //                     |EVENT_PLAY_PAUSE      |
 *  //                     v                      |
 *  //             +---------------+  EVENT_STOP  |
 *  //             | STATE_PLAYING |--------------+
 *  //             +---------------+              |
 *  //                  |     ^                   |
 *  //  EVENT_PLAY_PAUSE|     |EVENT_PLAY_PAUSE   |
 *  //                  v     |                   |
 *  //             +---------------+  EVENT_STOP  |
 *  //             | STATE_PAUSING |--------------+
 *  //             +---------------+
 *  //
 *  //
 *  //
 *  //             +---------------+
 *  //             |  STATE_FINAL  |
 *  //             +---------------+
 *
 *  #include "Mealy.h"
 *  enum //状态索引必须基于0且连续，建议显式定义成员值。
 *  {
 *      STATE_INITIAL = 0,//起始状态，必要且固定位置
 *
 *      STATE_STOPPED = 1,//停止状态
 *      STATE_PLAYING = 2,//播放状态
 *      STATE_PAUSING = 3,//暂停状态
 *      //可自定义更多状态在STATE_INITIAL和STATE_FINAL之间...
 *
 *      STATE_FINAL = 4,          //最终状态，必要且固定位置
 *      STATE_LENGTH = STATE_FINAL//状态数量，必要且固定位置
 *  };
 *
 *  enum//事件索引必须基于0且连续，建议显式定义成员值。
 *  {
 *      EVENT_PLAY_PAUSE = 0,//`播放暂停`事件
 *      EVENT_STOP = 1,      //`停止`事件
 *      //可自定义更多事件...
 *
 *      EVENT_LENGTH = 2//事件数量，必要且固定位置
 *  };
 *
 *  static void Stop(uint32_t current,
 *      uint32_t next,
 *      uint32_t event,
 *      void* parameter)
 *  {
 *      printf("Stop:%s, Current:%u, Event:%u, Next:%u\n", (char*)parameter, current, next, event);
 *  }
 *
 *  static void Play(uint32_t current,
 *      uint32_t next,
 *      uint32_t event,
 *      void* parameter)
 *  {
 *      printf("Play:%s, Current:%u, Event:%u, Next:%u\n", (char*)parameter, current, next, event);
 *  }
 *
 *  static void Pause(uint32_t current,
 *      uint32_t next,
 *      uint32_t event,
 *      void* parameter)
 *  {
 *      printf("Pause:%s, Current:%u, Event:%u, Next:%u\n", (char*)parameter, current, next, event);
 *  }
 *
 *  static void Resume(uint32_t current,
 *      uint32_t next,
 *      uint32_t event,
 *      void* parameter)
 *  {
 *      printf("Resume:%s, Current:%u, Event:%u, Next:%u\n", (char*)parameter, current, next, event);
 *  }
 *
 *  static const Mealy_StateType mStateTable[STATE_LENGTH] =//状态表。必须显式声明状态数量STATE_LENGTH。
 *  {
 *     [STATE_INITIAL] =
 *     {
 *         .Transitions = (const Mealy_TransitionType[EVENT_LENGTH])//状态转换表。必须显式声明事件数量EVENT_LENGTH。
 *         {
 *             //相当于任意按键都进入停止状态
 *             [EVENT_PLAY_PAUSE] = {.Handler = Stop,.Next = STATE_STOPPED},
 *             [EVENT_STOP] = {.Handler = Stop,.Next = STATE_STOPPED},
 *         },
 *     },
 *     [STATE_STOPPED] = {
 *         .Transitions = (const Mealy_TransitionType[EVENT_LENGTH])//状态转换表。必须显式声明事件数量EVENT_LENGTH。
 *         {
 *             [EVENT_PLAY_PAUSE] = {.Handler = Play,.Next = STATE_PLAYING },
 *             //注意：不需要处理的事件，配置目标状态为起始状态，或者直接不定义。
 *             //[EVENT_STOP] = {.Handler = NULL,.Next = STATE_INITIAL },
 *         },
 *     },
 *     [STATE_PLAYING] = {
 *         .Transitions = (const Mealy_TransitionType[EVENT_LENGTH])//状态转换表。必须显式声明事件数量EVENT_LENGTH。
 *         {
 *             [EVENT_PLAY_PAUSE] = {.Handler = Pause,.Next = STATE_PAUSING },
 *             [EVENT_STOP] = {.Handler = Stop,.Next = STATE_STOPPED }
 *         },
 *     },
 *     [STATE_PAUSING] = {
 *         .Transitions = (const Mealy_TransitionType[EVENT_LENGTH])//状态转换表。必须显式声明事件数量EVENT_LENGTH。
 *         {
 *             [EVENT_PLAY_PAUSE] = {.Handler = Resume,.Next = STATE_PLAYING },
 *             [EVENT_STOP] = {.Handler = Stop,.Next = STATE_STOPPED }
 *         },
 *     },
 *  };
 *
 *  int main(void)
 *  {
 *      Mealy_MachineType m = { 0 };//定义状态机实例
 *      Mealy_Start(&m, mStateTable, STATE_LENGTH, EVENT_LENGTH);//启动状态机
 *      Mealy_Raise(&m, EVENT_STOP, NULL);//由于启动后，状态机在起始状态。手动引发停止事件，从起始状态到停止状态
 *
 *      char* file = "a.mp3";//也可以定义一个结构体作为上下文，传递给状态机
 *      Mealy_Raise(&m, EVENT_PLAY_PAUSE, file);//模拟按下了`播放暂停`键，引发播放事件，从停止状态到播放状态
 *      Mealy_Raise(&m, EVENT_PLAY_PAUSE, file);//模拟按下了`播放暂停`键，引发暂停事件，从播放状态到暂停状态
 *      Mealy_Raise(&m, EVENT_PLAY_PAUSE, file);//模拟按下了`播放暂停`键，引发播放事件，从暂停状态到播放状态
 *      Mealy_Raise(&m, EVENT_STOP, file);//模拟按下了`停止`键，引发停止事件，从播放状态到停止状态
 *
 *      Mealy_Raise(&m, EVENT_PLAY_PAUSE, file);//模拟按下了`播放暂停`键，引发播放事件，从停止状态到播放状态
 *      Mealy_Raise(&m, EVENT_PLAY_PAUSE, file);//模拟按下了`播放暂停`键，引发暂停事件，从播放状态到暂停状态
 *      Mealy_Raise(&m, EVENT_STOP, file);//模拟按下了`停止`键，引发停止事件，从暂停状态到停止状态
 *
 *      Mealy_Stop(&m);//停止状态机
 *      return 0;
 *  }
 *
 *  // 输出结果：
 *  // Stop:(null), Current:0, Event:1, Next:1
 *  // Play:a.mp3, Current:1, Event:0, Next:2
 *  // Pause:a.mp3, Current:2, Event:0, Next:3
 *  // Resume:a.mp3, Current:3, Event:0, Next:2
 *  // Stop:a.mp3, Current:2, Event:1, Next:1
 *  // Play:a.mp3, Current:1, Event:0, Next:2
 *  // Pause:a.mp3, Current:2, Event:0, Next:3
 *  // Stop:a.mp3, Current:3, Event:1, Next:1
 *
 * @endcode
 */
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 版本号。
     */
#define MEALY_VERSION 1

     /**
      * @brief 返回值类型。
      */
    typedef enum
    {
        /**
         * @brief 操作成功。
         */
        MEALY_RETURN_OK = 1,
        /**
         * @brief 参数为空指针或必需资源为空。
         */
        MEALY_RETURN_NULL = 0,
        /**
         * @brief 参数越界或非法值。
         */
        MEALY_RETURN_OVERFLOW = -1,
        /**
         * @brief 忽略进入起始状态。
         */
        MEALY_RETURN_IGNORED_INITIAL = -2,
        /**
         * @brief 最终状态忽略引发事件。
         */
        MEALY_RETURN_IGNORED_FINAL = -3,
    } Mealy_ReturnType;

    /**
     * @brief 状态机活动回调函数类型。
     * @param current 当前状态索引。
     * @param next 目标状态索引。
     * @param event 引发事件索引。
     * @param parameter 事件关联参数，可为 NULL。
     */
    typedef void (*Mealy_ActivityType)(uint32_t current,
        uint32_t next,
        uint32_t event,
        void* parameter);

    /**
     * @brief 状态机状态转换类型。
     */
    typedef struct
    {
        /**
         * @brief 状态转换前调用的回调函数（可为 NULL）。由离开当前状态和进入目标状态的回调函数合并而成。
         */
        Mealy_ActivityType Handler;
        /**
         * @brief 目标状态索引。状态索引约定：0 为起始状态，大于等于状态数量为最终状态，中间的索引为自定义状态。
         * @warning 如果为起始状态，状态机将忽略引发事件。特别适用于在设计状态时隐式声明状态转换。
         */
        uint32_t Next;
    } Mealy_TransitionType;

    /**
     * @brief 状态机状态类型。
     */
    typedef struct
    {
        /**
         * @brief 状态转换表。
         */
        const Mealy_TransitionType* Transitions;
    } Mealy_StateType;

    /**
     * @brief 状态机类型。
     */
    typedef struct
    {
        /**
         * @brief 当前状态索引。
         */
        uint32_t Current;
        /**
         * @brief 状态机包含的状态表。
         */
        const Mealy_StateType* States;
        /**
         * @brief 状态数量。必须与状态表中状态数量匹配，最少 2 个状态。
         */
        uint32_t StateLength;
        /**
         * @brief 状态转换数量，由于事件与状态转换是一对一关系，所以由事件数量决定，且每个状态都需要处理相同数量的事件。
         *        必须与状态转换表中转换数量匹配，最少 1 个状态转换。
         */
        uint32_t TransitionLength;
    } Mealy_MachineType;

    /**
     * @brief 启动状态机并转换到起始状态。
     * @param machine 状态机实例。
     * @param states 状态表。
     * @param stateLength 状态数量。
     * @param transitionLength 状态转换数量（等于事件数量）。
     * @return 返回启动结果。
     * @retval MEALY_RETURN_OK 启动成功。
     * @retval MEALY_RETURN_NULL 参数为空指针或必需资源为空。
     * @retval MEALY_RETURN_OVERFLOW 参数越界或非法值。
     */
    Mealy_ReturnType Mealy_Start(Mealy_MachineType* machine,
        const Mealy_StateType* states,
        uint32_t stateLength,
        uint32_t transitionLength);

    /**
     * @brief 强制停止状态机并转换到最终状态。
     * @param machine 状态机实例。
     * @return 返回停止结果。
     * @retval MEALY_RETURN_OK 停止成功。
     * @retval MEALY_RETURN_NULL 参数为空指针或必需资源为空。
     */
    Mealy_ReturnType Mealy_Stop(Mealy_MachineType* machine);

    /**
     * @brief 引发事件。
     * @param machine 状态机实例。
     * @param event 事件索引。
     * @param parameter 事件关联参数。
     * @return 返回事件处理结果。
     * @retval MEALY_RETURN_OK 引发事件成功。
     * @retval MEALY_RETURN_NULL 参数为空指针或必需资源为空。
     * @retval MEALY_RETURN_OVERFLOW 参数越界或非法值。
     * @retval MEALY_RETURN_IGNORED_INITIAL 忽略进入起始状态的所有事件。
     * @retval MEALY_RETURN_IGNORED_FINAL 状态机处于最终状态，忽略所有事件。
     */
    Mealy_ReturnType Mealy_Raise(Mealy_MachineType* machine,
        uint32_t event,
        void* parameter);
#ifdef __cplusplus
}
#endif
