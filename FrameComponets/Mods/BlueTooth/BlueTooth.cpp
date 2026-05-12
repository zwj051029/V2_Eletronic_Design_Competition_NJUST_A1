#include "bluetooth.hpp"
#include <cstring>

// ======================== 公开接口 ========================
void Bluetooth::Init(UART_HandleTypeDef *huart) {
    // 注册 UART：RX DMA，TX DMA，缓冲区 256 字节，回调为 RxCallback
    BspUart_Register(&uart_, huart,
                     BSP_UART_MODE_DMA, // RX
                     BSP_UART_MODE_DMA, // TX
                     256,               // DMA 接收长度（空闲中断时更新实际长度）
                     RxCallback);

    // 将本实例指针保存在 user_data 中，方便回调获取
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

    line_ready_ = false;
    line_len_ = 0;
    return true;
}

// ======================== 内部回调与处理 ========================
void Bluetooth::RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    // 从 user_data 取回 Bluetooth 实例
    Bluetooth *self = static_cast<Bluetooth *>(inst->user_data);
    if (self)
        self->ProcessRxData(data, len);
}

void Bluetooth::ProcessRxData(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        char c = data[i];

        if (c == '\r') {
            // 把 \r 当作行结束，且后面如果紧跟着 \n 就跳过它
            // 先将当前 buffer 内容作为一行
            if (line_len_ > 0) {
                line_buf_[line_len_] = '\0';
                line_ready_ = true;
                line_len_   = 0;
            }
            // 如果下一个字符是 \n，则直接跳过它（由循环继续处理下一个）
            // 注意：我们不在这里跳过，因为可能 DMA 缓冲区里 \r 和 \n 分两次接收
            // 最简单的方法是：置一个标志位，等待下一个字符如果是 \n 就忽略
            // 这里用一个小状态机
            // 为了简化，直接处理完 \r 后，额外判断 next char
            if (i + 1 < len && data[i + 1] == '\n') {
                i++;  // 跳过紧随的 \n
            }
            continue;
        }

        if (c == '\n') {
            // 单独 \n 也作为行结束（处理只有 \n 的情况）
            if (line_len_ > 0) {
                line_buf_[line_len_] = '\0';
                line_ready_ = true;
                line_len_   = 0;
            }
            continue;   // 注意这里用 continue，不把它加入 buffer
        }

        // 普通字符，添加到 line_buf_
        if (line_len_ < LINE_BUF_SIZE - 1) {
            line_buf_[line_len_++] = c;
        }
    }
}