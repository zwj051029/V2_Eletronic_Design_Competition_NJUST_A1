#ifndef MOD_M0COMMANDER_HPP_
#define MOD_M0COMMANDER_HPP_

#include "SysDefs.hpp"
#include "stm32f4xx_hal.h"

/**
 * @brief M0 速度指令发送器
 * @note  单例。使用 UART3 阻塞发送，每 1ms 由 ControlTask 调用 Send()。
 *        应用层通过 SetSpeed() 更新左右轮目标转速。
 */
class M0Commander {
    SINGLETON(M0Commander) {};

public:
    /// @brief 初始化，绑定 UART4 HAL 句柄
    void Init(UART_HandleTypeDef *huart);

    /// @brief 设置目标左右轮速度 (rpm)
    /// @param left  左轮目标转速，正转前进
    /// @param right 右轮目标转速
    void SetSpeed(float left, float right);

    /// @brief 将当前目标速度打包发送给 M0
    /// @note  由 1000Hz 任务调用，阻塞发送保证完整
    void Send();

private:
    UART_HandleTypeDef *huart_; // UART4 句柄
    float left_speed_;          // 当前目标左轮速度 (rpm)
    float right_speed_;         // 当前目标右轮速度 (rpm)

    static constexpr float MAX_SPEED = 300.0f; // 速度限幅 (rpm)
};

extern M0Commander &m0_commander;

#endif