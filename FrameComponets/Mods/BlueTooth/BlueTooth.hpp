#ifndef MOD_BLUETOOTH_HPP_
#define MOD_BLUETOOTH_HPP_

#include "bsp_uart.h"
#include "stm32f4xx_hal.h"

/**
 * @brief 蓝牙模块（UART2，DMA 收发）
 * @note  普通类，由外部持有实例并调用 Init
 */
class Bluetooth {
public:
    Bluetooth() = default;

    /// @brief 初始化 UART，绑定到指定 HAL 句柄，设置 DMA 收发和行缓冲
    void Init(UART_HandleTypeDef *huart);

    /// @brief 发送字符串（DMA 发送，非阻塞）
    void Send(const char *str);

    /// @brief 获取一行已接收的数据（若没有新行返回 false）
    bool GetLine(char *line, uint16_t max_len);

private:
    BspUart_Instance uart_;

    // 行缓冲区
    static constexpr uint16_t LINE_BUF_SIZE = 64;
    char line_buf_[LINE_BUF_SIZE];
    volatile uint16_t line_len_ = 0;
    volatile bool line_ready_ = false;

    // 静态 DMA 回调，内部通过 user_data 找回实例
    static void RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len);

    // 内部数据处理
    void ProcessRxData(uint8_t *data, uint16_t len);
};

#endif // MOD_BLUETOOTH_HPP_