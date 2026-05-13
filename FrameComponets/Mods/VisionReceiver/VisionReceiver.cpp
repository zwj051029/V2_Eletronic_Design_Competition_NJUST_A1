#include "VisionReceiver.hpp"
#include <cstring>

void VisionReceiver::Init(UART_HandleTypeDef *huart) {
    huart_ = huart;
    last_update_time_ = BspDwt_GetTimeline_Sec(); // 使用 DWT 时间

    BspUart_Register(&uart_, huart,
                     BSP_UART_MODE_DMA,    // RX DMA
                     BSP_UART_MODE_NORMAL, // TX 未使用
                     128,                  // DMA 缓冲
                     RxCallback);
    uart_.user_data = this;
}

void VisionReceiver::Update() {
    float now = BspDwt_GetTimeline_Sec();
    if ((now - last_update_time_) > 0.2f) { // 200ms 超时
        valid_ = false;
    }
}

// DMA 空闲中断回调
void VisionReceiver::RxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    VisionReceiver *self = static_cast<VisionReceiver *>(inst->user_data);
    if (self == nullptr)
        return;
    for (uint16_t i = 0; i < len; i++) {
        self->ProcessByte(data[i]);
    }
}

void VisionReceiver::ProcessByte(uint8_t byte) {
    if (!sync_) {
        if (byte == 0xAA) {
            sync_ = true;
            frame_pos_ = 0;
            frame_type_ = 0;
        }
        return;
    }

    if (frame_pos_ == 0) {
        frame_type_ = byte;
        frame_pos_++;
    } else {
        frame_buf_[frame_pos_ - 1] = byte;
        frame_pos_++;

        // 车道线数据帧：类型 0x01，共 11 字节（类型 + 9 数据），总帧长12
        if (frame_type_ == 0x01 && frame_pos_ >= 11) {
            float recv_offset, recv_angle;
            uint8_t valid_byte, checksum;

            // 用 memcpy 安全拷贝 float（避免对齐问题）
            memcpy(&recv_offset, &frame_buf_[0], 4);
            memcpy(&recv_angle, &frame_buf_[4], 4);
            valid_byte = frame_buf_[8];
            checksum = frame_buf_[9];

            // 计算校验：帧头 0xAA ^ 类型 0x01 ^ 数据9字节
            uint8_t calc = 0xAA ^ 0x01;
            for (int i = 0; i < 9; i++) {
                calc ^= frame_buf_[i];
            }

            if (calc == checksum) {
                OnValidFrame(recv_offset, recv_angle, valid_byte != 0);
            }
            sync_ = false; // 重新同步
        } else if (frame_pos_ >= 12) {
            // 未知类型帧或超长，重置
            sync_ = false;
        }
    }
}

void VisionReceiver::OnValidFrame(float offset, float angle, bool valid) {
    offset_ = offset;
    angle_ = angle;
    valid_ = valid;
    last_update_time_ = BspDwt_GetTimeline_Sec(); // 更新最后有效时间
}