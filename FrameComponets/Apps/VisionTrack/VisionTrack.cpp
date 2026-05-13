#include "VisionTrack.hpp"
#include "SpeedMixer.hpp"
#include "VisionReceiver.hpp"
#include "std_math.hpp"

VisionTrack &vision_track = VisionTrack::GetInstance();

void VisionTrack::Start() {
    VisionReceiver::GetInstance().Init(&huart1);
    speed_mixer.SetVisionTrackSpeed(base_speed_, 0.0f);

    lost_cnt_ = 0;
    normal_ = true;
}

void VisionTrack::Update() {
    // 更新视觉接收器看门狗
    VisionReceiver::GetInstance().Update();

    if (!System.system_started) {
        speed_mixer.SetVisionTrackSpeed(0.0f, 0.0f);
        lost_cnt_ = 0;
        normal_ = true;
        return;
    }

    VisionReceiver &vr = VisionReceiver::GetInstance();

    if (!vr.IsValid()) {
        lost_cnt_++;
        if (lost_cnt_ > 5) {
            // 丢线保护：低速直行
            speed_mixer.SetVisionTrackSpeed(base_speed_ * 0.3f, 0.0f);
            normal_ = false;
        }
        return;
    }

    lost_cnt_ = 0;
    normal_ = true;

    float offset = vr.GetOffset();
    float angle = vr.GetAngle();

    float turn = kp_ * offset + kd_ * angle;
    turn = StdMath::fclamp(turn, max_turn_);

    // 通过 SpeedMixer 设置巡线速度（含差速）
    speed_mixer.SetVisionTrackSpeed(base_speed_, turn);
}