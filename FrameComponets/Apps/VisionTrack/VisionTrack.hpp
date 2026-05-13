#ifndef VISION_TRACK_HPP
#define VISION_TRACK_HPP

#include "SysDefs.hpp"
#include "System.hpp"

class VisionTrack : public Application {
    SINGLETON(VisionTrack) : Application("VisionTrack") {
        prescaler = 1; // 200Hz
    };
    APPLICATION_OVERRIDE;

public:
    void SetKp(float kp) {
        kp_ = kp;
    }
    void SetKd(float kd) {
        kd_ = kd;
    }
    void SetBaseSpeed(float speed) {
        base_speed_ = speed;
    }

private:
    float kp_ = 0.015f;
    float kd_ = 0.008f;
    float base_speed_ = 80.0f;
    float max_turn_ = 100.0f;

    int lost_cnt_ = 0;
    bool normal_ = true;
};

extern VisionTrack &vision_track;
#endif