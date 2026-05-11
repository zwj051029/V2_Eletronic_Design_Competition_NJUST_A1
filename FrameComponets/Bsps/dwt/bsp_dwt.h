#ifndef __BSP_DWT_H__
#define __BSP_DWT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include "stdint.h"

#define CPU_HERT_F407_MHZ 168 // F407 CPU主频168MHz

    // 定义DWT时间结构体
    typedef struct
    {
        uint32_t s;
        uint16_t ms;
        uint16_t us;
    } BspDwt_TimeType;

    /**
     * @brief 该宏用于计算代码段执行时间,单位为秒/s,返回值为float类型
     *        首先需要创建一个float类型的变量,用于存储时间间隔
     *        计算得到的时间间隔同时还会通过RTT打印到日志终端,你也可以将你的dt变量添加到查看
     */
#define TIME_ELAPSE(dt, code)                    \
    do                                           \
    {                                            \
        float tstart = BspDwt_GetTimeline_Sec(); \
        code;                                    \
        dt = BspDwt_GetTimeline_Sec() - tstart;  \
        LOGINFO("[DWT] " #dt " = %f s\r\n", dt); \
    } while (0)

    /// @brief  初始化DWT
    void BspDwt_Init(uint32_t CPU_Freq_MHz);

    /// @brief 获取两次调用之间的时间间隔,单位为秒/s
    float BspDwt_GetDeltaTime(uint32_t *cnt_last);

    /// @brief 获取两次调用之间的时间间隔,单位为秒/s（精度更高的版本）
    double BspDwt_GetDeltaTime64(uint32_t *cnt_last);

    /// @brief 获取当前时间线，单位为秒/s
    float BspDwt_GetTimeline_Sec(void);

    /// @brief 获取当前时间线，单位为毫秒/ms
    float BspDwt_GetTimeline_MSec(void);

    /// @brief 获取当前时间线，单位为微秒/us
    uint64_t BspDwt_GetTimeline_USec(void);

    /// @brief 使用DWT进行延时，单位为秒/s
    void BspDwt_Delay(float Delay);

    /// @brief 更新全局系统时间
    void BspDwt_SysTimeUpdate(void);

    /// @brief 更新DWT计数器溢出轮次
    void BspDwt_CntUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DWT_H__ */
