#include "M0Commander.hpp"
#include "string.h"

// 静态实例（由 SINGLETON 宏自动生成）
M0Commander &m0_commander = M0Commander::GetInstance();

void M0Commander::Init(UART_HandleTypeDef *huart) {
    huart_ = huart;
    left_speed_ = 0.0f;
    right_speed_ = 0.0f;
}

void M0Commander::SetSpeed(float left, float right) {
    // 限幅保护
    if (left > MAX_SPEED)
        left = MAX_SPEED;
    if (left < -MAX_SPEED)
        left = -MAX_SPEED;
    if (right > MAX_SPEED)
        right = MAX_SPEED;
    if (right < -MAX_SPEED)
        right = -MAX_SPEED;

    left_speed_ = left;
    right_speed_ = right;
}

void M0Commander::Send() {
    if (huart_ == nullptr)
        return;

    uint8_t frame[10];
    frame[0] = 0xBB;                     // 帧头
    memcpy(&frame[1], &left_speed_, 4);  // 左轮速度 (float)
    memcpy(&frame[5], &right_speed_, 4); // 右轮速度 (float)

    // 计算校验和（所有字节异或）
    uint8_t checksum = 0;
    for (int i = 0; i < 9; i++) {
        checksum ^= frame[i];
    }
    frame[9] = checksum;

    // 阻塞发送（10 字节，1ms 内完全可以完成）
    HAL_UART_Transmit(huart_, frame, 10, 2); // 超时 2ms
}