#include "bluetooth.hpp"
#include <cstring>

Bluetooth::Bluetooth() noexcept {
    line_len_ = 0;
    line_ready_ = false;
}

void Bluetooth::Init(UART_HandleTypeDef *huart) {
    BspUart_Register(&uart_, huart, BSP_UART_MODE_DMA, BSP_UART_MODE_DMA, 256, RxCallback);
    uart_.user_data = this;
    line_len_ = 0;
    line_ready_ = false;
}

void Bluetooth::Send(const char *str) {
    BspUart_Transmit(&uart_, (uint8_t *) str, strlen(str));
}

bool Bluetooth::GetLine(char *line, uint16_t max_len) {
    if (!line_ready_)
        return false;

    uint16_t len = line_len_;
    if (len >= max_len)
        len = max_len - 1;
    memcpy(line, line_buf_, len);
    line[len] = '\0';

    // 取走后重置，准备接收下一行
    line_ready_ = false;
    line_len_ = 0;
    return true;
}

void Bluetooth::RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    Bluetooth *self = static_cast<Bluetooth *>(inst->user_data);
    if (self)
        self->ProcessRxData(data, len);
}

void Bluetooth::ProcessRxData(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        char c = data[i];

        // 如果上一行还没被取走，丢弃新数据，避免覆盖
        if (line_ready_) {
            continue;
        }

        if (c == '\r') {
            if (line_len_ > 0) {
                line_buf_[line_len_] = '\0'; // 添加字符串结尾
                line_ready_ = true;          // 通知有新行
                // 注意：不清零 line_len_，留给 GetLine 使用
                // 如果紧跟着 \n，则跳过它
                if (i + 1 < len && data[i + 1] == '\n') {
                    i++;
                }
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

        // 普通字符，存入缓冲区
        if (line_len_ < LINE_BUF_SIZE - 1) {
            line_buf_[line_len_++] = c;
        }
    }
}