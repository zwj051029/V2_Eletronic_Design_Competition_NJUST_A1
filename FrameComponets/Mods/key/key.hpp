#ifndef MOD_KEY_HPP_
#define MOD_KEY_HPP_

#include "bsp_gpio.h"

/**
 * @brief 按键事件枚举
 */
enum KeyEvent {
    KEY_PRESS_DOWN,       // 按下瞬间（消抖后第一次检测到按下）
    KEY_PRESS_UP,         // 松开瞬间（消抖后第一次检测到松开）
    KEY_SHORT_PRESS,      // 短按（按下后又在超时前松开）
    KEY_LONG_PRESS_START, // 长按开始（按住超过阈值）
    KEY_LONG_PRESS,       // 长按持续（每周期检测到仍按住）
};

/**
 * @brief 按键类（带消抖、短按/长按检测）
 * @note 必须周期性调用 Update()，推荐频率 50Hz (20ms)。
 *       可通过注册回调或直接读取状态获取按键信息。
 */
class Key {
public:
    using Callback = void (*)(KeyEvent event, void *user_data);

    /**
     * @brief 构造函数
     * @param port      端口字符
     * @param pin       引脚号
     * @param active_low true=按下为低电平，false=按下为高电平（默认按下为低电平）
     * @param long_time 长按触发时间，单位：Update() 调用次数（例如 50Hz 下，50 次 ≈ 1 秒）
     */
    Key(char port, uint8_t pin, bool active_low = true, uint16_t long_threshold = 50);

    /// @brief 注册事件回调
    void AttachCallback(Callback cb, void *user_data = nullptr);

    /// @brief 周期性调用，执行消抖和事件检测
    /// @note 必须由外部以固定频率调用（如 50Hz）
    void Update();

    /// @brief 查询按键当前是否被按下（消抖后的稳定状态）
    bool IsPressed() const {
        return stable_state_;
    }

    /// @brief 查询是否刚刚发生短按（单次有效，读取后自动清除）
    bool IsShortPressed();

    /// @brief 查询是否处于长按状态
    bool IsLongPressed() const {
        return long_press_active_;
    }

private:
    BspGpio_Instance gpio_;
    bool active_low_;                            // true: 按下时为低电平
    bool stable_state_;                          // 消抖后的物理状态（true=按下）
    bool last_stable_state_;                     // 上一次稳定状态
    bool short_press_flag_;                      // 短按标记（置起后由 IsShortPressed 清除）
    bool long_press_active_;                     // 长按激活中
    uint16_t hold_count_;                        // 当前按住持续次数（Update 调用次数）
    uint16_t long_threshold_;                    // 长按阈值（次数）
    uint8_t debounce_cnt_;                       // 消抖计数器
    static constexpr uint8_t DEBOUNCE_LIMIT = 3; // 消抖阈值
    Callback callback_;
    void *callback_data_;

    void ResetDebounce(bool initial_state);
    void FireEvent(KeyEvent event);
};

#endif // MOD_KEY_HPP_