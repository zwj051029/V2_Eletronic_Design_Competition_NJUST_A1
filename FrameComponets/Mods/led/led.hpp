#ifndef MOD_LED_HPP_
#define MOD_LED_HPP_

#include "bsp_gpio.h"

/**
 * @brief LED 控制类
 * @note 封装单个 GPIO 引脚，提供开/关/翻转操作。
 *       默认高电平点亮，可在构造函数中指定有效电平。
 */
class Led {
public:
    /**
     * @brief 构造函数，绑定一个 GPIO 引脚
     * @param port     端口字符，如 'A', 'B'
     * @param pin      引脚号 0~15
     * @param active   有效电平，true=高电平亮，false=低电平亮（默认低电平亮）
     */
    Led(char port, uint8_t pin, bool active_high = false);

    /// @brief 点亮 LED
    void On();

    /// @brief 熄灭 LED
    void Off();

    /// @brief 翻转 LED 状态
    void Toggle();

    /// @brief 直接设置状态
    /// @param on  true=点亮，false=熄灭
    void Set(bool on);

    /// @brief 读取当前输出电平（物理电平，非逻辑亮灭）
    BspGpio_PinState Read();

private:
    BspGpio_Instance gpio_;
    bool active_high_;   // true: 高电平有效, false: 低电平有效
    bool current_state_; // 当前逻辑亮灭状态
};

#endif // MOD_LED_HPP_