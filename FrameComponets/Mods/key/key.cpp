#include "key.hpp"

Key::Key(char port, uint8_t pin, bool active_low, uint16_t long_threshold)
    : active_low_(active_low), stable_state_(false), last_stable_state_(false), short_press_flag_(false),
      long_press_active_(false), hold_count_(0), long_threshold_(long_threshold), debounce_cnt_(0), callback_(nullptr),
      callback_data_(nullptr) {
    BspGpio_Register(&gpio_, port, pin);
    // 读取初始电平，设置消抖状态
    bool initial = (BspGpio_Read(&gpio_) == BSP_GPIO_LOW) ? true : false;
    if (!active_low_)
        initial = !initial; // 转换为“按下”逻辑
    stable_state_ = initial;
    last_stable_state_ = initial;
    ResetDebounce(initial);
}

void Key::AttachCallback(Callback cb, void *user_data) {
    callback_ = cb;
    callback_data_ = user_data;
}

void Key::ResetDebounce(bool initial_state) {
    debounce_cnt_ = initial_state ? DEBOUNCE_LIMIT : 0;
}

void Key::Update() {
    // 读取当前物理电平，并转换为逻辑“按下”
    bool raw = (BspGpio_Read(&gpio_) == BSP_GPIO_LOW) ? true : false;
    if (!active_low_)
        raw = !raw;

    // 消抖计数器
    if (raw) {
        if (debounce_cnt_ < DEBOUNCE_LIMIT)
            debounce_cnt_++;
    } else {
        if (debounce_cnt_ > 0)
            debounce_cnt_--;
    }

    // 根据消抖结果更新稳定状态
    bool new_state = (debounce_cnt_ >= DEBOUNCE_LIMIT);

    // 检测边沿
    if (new_state && !stable_state_) {
        // 按下事件
        hold_count_ = 0;
        long_press_active_ = false;
        FireEvent(KEY_PRESS_DOWN);
    } else if (!new_state && stable_state_) {
        // 松开事件
        FireEvent(KEY_PRESS_UP);
        // 判断是否短按（之前没有触发长按）
        if (!long_press_active_ && hold_count_ < long_threshold_) {
            short_press_flag_ = true;
            FireEvent(KEY_SHORT_PRESS);
        }
        hold_count_ = 0;
        long_press_active_ = false;
    }

    // 如果处于按下状态，处理长按
    if (new_state) {
        hold_count_++;
        if (hold_count_ == long_threshold_) {
            long_press_active_ = true;
            FireEvent(KEY_LONG_PRESS_START);
        }
        if (long_press_active_) {
            FireEvent(KEY_LONG_PRESS);
        }
    }

    last_stable_state_ = stable_state_;
    stable_state_ = new_state;
}

bool Key::IsShortPressed() {
    bool ret = short_press_flag_;
    short_press_flag_ = false;
    return ret;
}

void Key::FireEvent(KeyEvent event) {
    if (callback_)
        callback_(event, callback_data_);
}