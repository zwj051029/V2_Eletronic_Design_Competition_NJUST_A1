/**
 * @file    VisionReceiver.hpp
 * @brief   K230 视觉数据接收模块（Mods 层，不继承 Application）
 * @note
 *          ## 与 K230 的通讯协议（UART1, 115200bps, 200Hz）
 *
 *          帧格式（共 12 字节）：
 *          ------------------------------------------------------------
 *          | 0xAA | 0x01 | offset(4B) | angle(4B) | valid(1B) | XOR  |
 *          ------------------------------------------------------------
 *          - offset  : float (小端), 横向偏移 (mm)，左正右负
 *          - angle   : float (小端), 航向偏差 (°)，左偏为正
 *          - valid   : uint8_t, 1=有效, 0=丢线
 *          - XOR     : 前 11 字节的异或校验
 *
 *          ## 安全机制
 *          - 调用方应周期性调用 Update()（建议 200Hz）
 *          - 200ms 未收到有效帧 → 自动置 valid_ = false
 */

#ifndef VISION_RECEIVER_HPP
#define VISION_RECEIVER_HPP

#include "SysDefs.hpp" // 单例宏
#include "bsp_dwt.h"   // DWT 时间戳
#include "bsp_uart.h"  // UART 多实例封装

class VisionReceiver {
    SINGLETON(VisionReceiver) {};

public:
    /// @brief 初始化 UART1，启动 DMA 接收
    void Init(UART_HandleTypeDef *huart);

    /// @brief 周期性更新（看门狗），应由调用方以 200Hz 频率调用
    void Update();

    // ---------- 数据访问接口 ----------
    float GetOffset() const {
        return offset_;
    }
    float GetAngle() const {
        return angle_;
    }
    bool IsValid() const {
        return valid_;
    }

private:
    UART_HandleTypeDef *huart_;
    BspUart_Instance uart_;

    volatile float offset_ = 0.0f;
    volatile float angle_ = 0.0f;
    volatile bool valid_ = false;
    volatile float last_update_time_ = 0.0f; // 最后有效帧的系统时间 (秒)

    // 帧解析状态
    bool sync_ = false;
    uint8_t frame_buf_[12];
    uint8_t frame_pos_ = 0;
    uint8_t frame_type_ = 0;

    static void RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len);
    void ProcessByte(uint8_t byte);
    void OnValidFrame(float offset, float angle, bool valid);
};

#endif // VISION_RECEIVER_HPP