#include "VisionTrack.hpp"
#include "M0Commander.hpp"
#include "VisionReceiver.hpp"
#include "std_math.hpp"

VisionTrack &vision_track = VisionTrack::GetInstance();

void VisionTrack::Start() {
    lost_cnt_ = 0;
    normal_ = true;
}

void VisionTrack::Update() {
    // 1. 先更新 VisionReceiver 的看门狗
    VisionReceiver::GetInstance().Update();

    if (!System.system_started) {
        M0Commander::GetInstance().SetSpeed(0.0f, 0.0f);
        lost_cnt_ = 0;
        normal_ = true;
        return;
    }

    VisionReceiver &vr = VisionReceiver::GetInstance();

    if (!vr.IsValid()) {
        lost_cnt_++;
        if (lost_cnt_ > 5) {
            // 丢线保护：低速直行
            M0Commander::GetInstance().SetSpeed(base_speed_ * 0.3f, base_speed_ * 0.3f);
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

    float left = base_speed_ - turn;
    float right = base_speed_ + turn;

    left = StdMath::fclamp(left, 0.0f, 300.0f);
    right = StdMath::fclamp(right, 0.0f, 300.0f);

    M0Commander::GetInstance().SetSpeed(left, right);
}