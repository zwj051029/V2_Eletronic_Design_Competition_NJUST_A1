#include "bsp_dwt.h"

/// @brief DWT时间相关的静态变量
static BspDwt_TimeType SysTime;                              // 全局系统时间结构体
static uint32_t CPU_FREQ_Hz, CPU_FREQ_Hz_ms, CPU_FREQ_Hz_us; // CPU频率相关变量
static uint32_t CYCCNT_RountCount;                           // DWT计数器溢出的次数
static uint32_t CYCCNT_LAST;                                 // 上一次DWT计数器的值
static uint64_t CYCCNT64;                                    // 64位DWT计数器值

/**
 * @brief 用于检查DWT CYCCNT寄存器是否溢出,并更新CYCCNT_RountCount
 * @attention 此函数假设两次调用之间的时间间隔不超过一次溢出
 *
 * @todo 更好的方案是为dwt的时间更新单独设置一个任务?
 *       不过,使用dwt的初衷是定时不被中断/任务等因素影响,因此该实现仍然有其存在的意义
 */
void BspDwt_CntUpdate(void) {
    // 线程锁，防止多线程同时访问
    static volatile uint8_t bit_locker = 0;

    // 判断dwt有没有被锁定
    if (!bit_locker) {
        bit_locker = 1;
        volatile uint32_t cnt_now = DWT->CYCCNT; // 获取当前计数值

        // 当前计数值小于上一次的计数值，说明发生了溢出
        if (cnt_now < CYCCNT_LAST)
            CYCCNT_RountCount++; // 溢出则轮次计数加1

        // 更新上一次计数值
        CYCCNT_LAST = DWT->CYCCNT;
        bit_locker = 0;
    }
}

/**
 * @brief 初始化DWT,传入参数为CPU频率,单位MHz
 * @param CPU_Freq_MHz CPU频率，单位MHz
 */
void BspDwt_Init(uint32_t CPU_Freq_MHz) {
    // 使能DWT和CYCCNT寄存器
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 将CYCCNT寄存器清零
    DWT->CYCCNT = (uint32_t) 0u;

    // 使能Cortex-M DWT CYCCNT寄存器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 计算并存储CPU频率相关变量
    CPU_FREQ_Hz = CPU_Freq_MHz * 1000000;
    CPU_FREQ_Hz_ms = CPU_FREQ_Hz / 1000;
    CPU_FREQ_Hz_us = CPU_FREQ_Hz / 1000000;
    CYCCNT_RountCount = 0;

    // 初始化上一次计数值
    BspDwt_CntUpdate();
}

/**
 * @brief 获取两次调用之间的时间间隔,单位为秒/s
 * @param cnt_last 上一次调用的时间戳（需要在外部调用点处维护）
 */
float BspDwt_GetDeltaTime(uint32_t *cnt_last) {
    volatile uint32_t cnt_now = DWT->CYCCNT;
    float dt = ((uint32_t) (cnt_now - *cnt_last)) / ((float) (CPU_FREQ_Hz));
    *cnt_last = cnt_now;

    BspDwt_CntUpdate();

    return dt;
}

/**
 * @brief 获取两次调用之间的时间间隔,单位为秒/s（精度更高的版本）
 */
double BspDwt_GetDeltaTime64(uint32_t *cnt_last) {
    volatile uint32_t cnt_now = DWT->CYCCNT;
    double dt = ((uint32_t) (cnt_now - *cnt_last)) / ((double) (CPU_FREQ_Hz));
    *cnt_last = cnt_now;

    BspDwt_CntUpdate();

    return dt;
}

/**
 * @brief 更新全局系统时间
 */
void BspDwt_SysTimeUpdate(void) {
    // 如果DWT计数器溢出,则更新轮次计数
    volatile uint32_t cnt_now = DWT->CYCCNT;

    static uint64_t CNT_ms, CNT_us;

    // 检测是否发生溢出，如果是，更新轮数
    BspDwt_CntUpdate();

    // 计算当前的系统时间（单位是晶振tick）
    CYCCNT64 = (uint64_t) CYCCNT_RountCount * (uint64_t) UINT32_MAX + (uint64_t) cnt_now;

    // 经过的秒数为：系统时间（单位是tick）除以CPU频率（整除）
    SysTime.s = CYCCNT64 / CPU_FREQ_Hz;

    // 计算剩余的毫秒数（整除）
    CNT_ms = CYCCNT64 - (SysTime.s * CPU_FREQ_Hz);
    SysTime.ms = CNT_ms / CPU_FREQ_Hz_ms;

    // 计算剩余的微秒数（整除）
    CNT_us = CNT_ms - SysTime.ms * CPU_FREQ_Hz_ms;
    SysTime.us = CNT_us / CPU_FREQ_Hz_us;
}

/**
 * @brief 获取当前时间线，单位为秒/s
 * @return float 当前时间线，单位为秒/s
 */
float BspDwt_GetTimeline_Sec(void) {
    BspDwt_SysTimeUpdate();

    float DWT_Timeline = SysTime.s + SysTime.ms * 0.001f + SysTime.us * 0.000001f;

    return DWT_Timeline;
}

/**
 * @brief 获取当前时间线，单位为毫秒/ms
 * @return float 当前时间线，单位为毫秒/ms
 */
float BspDwt_GetTimeline_MSec(void) {
    BspDwt_SysTimeUpdate();

    float DWT_Timelinef32 = SysTime.s * 1000 + SysTime.ms + SysTime.us * 0.001f;

    return DWT_Timelinef32;
}

/**
 * @brief 获取当前时间线，单位为微秒/us
 * @return uint64_t 当前时间线，单位为微秒/us
 */
uint64_t BspDwt_GetTimeline_USec(void) {
    BspDwt_SysTimeUpdate();

    uint64_t DWT_Timelinef32 = SysTime.s * 1000000 + SysTime.ms * 1000 + SysTime.us;

    return DWT_Timelinef32;
}

/**
 * @brief 使用DWT进行延时，单位为秒/s
 * @param Delay 延时时间，单位为秒/s
 */
void BspDwt_Delay(float Delay) {
    uint32_t tickstart = DWT->CYCCNT;
    float wait = Delay;

    while ((DWT->CYCCNT - tickstart) < wait * (float) CPU_FREQ_Hz)
        ;
}
