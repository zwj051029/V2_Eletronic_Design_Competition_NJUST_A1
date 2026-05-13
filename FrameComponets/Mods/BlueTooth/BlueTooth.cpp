#include "bluetooth.hpp"
#include <cstring>

Bluetooth::Bluetooth() noexcept {
    line_len_ = 0;
    line_ready_ = false;
    huart_ = nullptr;
}

void Bluetooth::Init(UART_HandleTypeDef *huart) {
    huart_ = huart; // 保存句柄，供 Send 用

    // RX 用 DMA，TX 用普通模式（避免 DMA 冲突）
    BspUart_Register(&uart_, huart,
                     BSP_UART_MODE_DMA,    // RX: DMA + 空闲中断
                     BSP_UART_MODE_NORMAL, // TX: 阻塞模式
                     256, RxCallback);
    uart_.user_data = this;

    line_len_ = 0;
    line_ready_ = false;
}

// 阻塞发送，确保数据完整发出
void Bluetooth::Send(const char *str) {
    if (huart_ == nullptr)
        return;
    uint16_t len = strlen(str);
    if (len == 0)
        return;
    HAL_UART_Transmit(huart_, (uint8_t *) str, len, 100); // 100ms 超时
}

bool Bluetooth::GetLine(char *line, uint16_t max_len) {
    if (!line_ready_)
        return false;

    uint16_t len = line_len_;
    if (len >= max_len)
        len = max_len - 1;
    memcpy(line, line_buf_, len);
    line[len] = '\0';

    line_ready_ = false;
    line_len_ = 0;
    return true;
}

// ===== DMA 接收回调 =====
void Bluetooth::RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    Bluetooth *self = static_cast<Bluetooth *>(inst->user_data);
    if (self)
        self->ProcessRxData(data, len);
}

void Bluetooth::ProcessRxData(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        char c = data[i];

        // 上一行还没取走，丢弃新数据
        if (line_ready_)
            continue;

        if (c == '\r') {
            if (line_len_ > 0) {
                line_buf_[line_len_] = '\0';
                line_ready_ = true; // 不清零 line_len_
                // 跳过紧跟的 \n
                if (i + 1 < len && data[i + 1] == '\n')
                    i++;
            }
            continue;
        }

        if (c == '\n') {
            if (line_len_ > 0) {
                line_buf_[line_len_] = '\0';
                line_ready_ = true;
            }
            continue;
        }

        if (line_len_ < LINE_BUF_SIZE - 1) {
            line_buf_[line_len_++] = c;
        }
    }
}