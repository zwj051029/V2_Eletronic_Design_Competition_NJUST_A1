#ifndef SPEEDMIXER_HPP
#define SPEEDMIXER_HPP

#include "M0Commander.hpp"
#include "SysDefs.hpp"
#include "System.hpp"

class SpeedMixer : public Application {
    SINGLETON(SpeedMixer) : Application("SpeedMixer") {
        prescaler = 1;
    };
    APPLICATION_OVERRIDE;

public:
    enum class Source : uint8_t {
        NONE = 0,
        VISION_TRACK = 1,
        OVERTAKE = 2,
        INTERSECTION_TURN = 3,
        GATE_WAIT = 4,
        TRAFFIC_LIGHT_WAIT = 5,
        PARKING = 6,
        EMERGENCY_STOP = 7
    };

    void SetVisionTrackSpeed(float base_speed, float speed_diff);
    void SetOvertakeSpeed(float left, float right);
    void SetIntersectionTurnSpeed(float left, float right);
    void SetGateWaitSpeed(float left, float right);
    void SetTrafficLightWaitSpeed(float left, float right);
    void SetParkingSpeed(float left, float right);
    void SetEmergencyStop();

    void ClearSource(Source source);
    void ClearAll();

    float GetFinalLeftSpeed() const;
    float GetFinalRightSpeed() const;

private:
    struct VisionTrackData {
        float base_speed = 80.0f;
        float speed_diff = 0.0f;
        bool valid = false;
    } vision_track_;

    struct DirectSpeed {
        float left = 0.0f;
        float right = 0.0f;
        bool valid = false;
    };

    DirectSpeed overtake_;
    DirectSpeed intersection_turn_;
    DirectSpeed gate_wait_;
    DirectSpeed traffic_light_wait_;
    DirectSpeed parking_;
    bool emergency_stop_ = false;

    static constexpr float MAX_SPEED = 300.0f;
    Source GetHighestPrioritySource() const;
};

extern SpeedMixer &speed_mixer;

#endif