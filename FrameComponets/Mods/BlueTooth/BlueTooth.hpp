#ifndef MOD_BLUETOOTH_HPP_
#define MOD_BLUETOOTH_HPP_

#include "bsp_uart.h"
#include "stm32f4xx_hal.h"

class Bluetooth {
public:
    Bluetooth() noexcept;
    void Init(UART_HandleTypeDef *huart);
    void Send(const char *str); // 阻塞发送，确保完整
    bool GetLine(char *line, uint16_t max_len);

private:
    UART_HandleTypeDef *huart_; // 保存 HAL 句柄，用于阻塞发送
    BspUart_Instance uart_;

    static constexpr uint16_t LINE_BUF_SIZE = 64;
    char line_buf_[LINE_BUF_SIZE];
    volatile uint16_t line_len_ = 0;
    volatile bool line_ready_ = false;

    static void RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len);
    void ProcessRxData(uint8_t *data, uint16_t len);
};

#endif