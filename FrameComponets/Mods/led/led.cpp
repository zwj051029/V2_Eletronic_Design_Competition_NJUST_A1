#include "led.hpp"

Led::Led(char port, uint8_t pin, bool active_high) : active_high_(active_high), current_state_(false) {
    BspGpio_Register(&gpio_, port, pin);
    // 初始化为熄灭状态
    Off();
}

void Led::On() {
    BspGpio_PinState phys = active_high_ ? BSP_GPIO_HIGH : BSP_GPIO_LOW;
    BspGpio_Write(&gpio_, phys);
    current_state_ = true;
}

void Led::Off() {
    BspGpio_PinState phys = active_high_ ? BSP_GPIO_LOW : BSP_GPIO_HIGH;
    BspGpio_Write(&gpio_, phys);
    current_state_ = false;
}

void Led::Toggle() {
    if (current_state_)
        Off();
    else
        On();
}

void Led::Set(bool on) {
    if (on)
        On();
    else
        Off();
}

BspGpio_PinState Led::Read() {
    return BspGpio_Read(&gpio_);
}