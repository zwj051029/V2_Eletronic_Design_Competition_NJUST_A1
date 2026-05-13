#include "SpeedMixer.hpp"
#include "std_math.hpp"

SpeedMixer &speed_mixer = SpeedMixer::GetInstance();

void SpeedMixer::Start() {
    m0_commander.Init(&huart3);
}

void SpeedMixer::Update() {
    // 不需要周期操作
}

void SpeedMixer::SetVisionTrackSpeed(float base_speed, float speed_diff) {
    vision_track_.base_speed = base_speed;
    vision_track_.speed_diff = speed_diff;
    vision_track_.valid = true;
}

void SpeedMixer::SetOvertakeSpeed(float left, float right) {
    overtake_.left = left;
    overtake_.right = right;
    overtake_.valid = true;
}

void SpeedMixer::SetIntersectionTurnSpeed(float left, float right) {
    intersection_turn_.left = left;
    intersection_turn_.right = right;
    intersection_turn_.valid = true;
}

void SpeedMixer::SetGateWaitSpeed(float left, float right) {
    gate_wait_.left = left;
    gate_wait_.right = right;
    gate_wait_.valid = true;
}

void SpeedMixer::SetTrafficLightWaitSpeed(float left, float right) {
    traffic_light_wait_.left = left;
    traffic_light_wait_.right = right;
    traffic_light_wait_.valid = true;
}

void SpeedMixer::SetParkingSpeed(float left, float right) {
    parking_.left = left;
    parking_.right = right;
    parking_.valid = true;
}

void SpeedMixer::SetEmergencyStop() {
    emergency_stop_ = true;
}

void SpeedMixer::ClearSource(Source source) {
    switch (source) {
    case Source::VISION_TRACK:
        vision_track_.valid = false;
        break;
    case Source::OVERTAKE:
        overtake_.valid = false;
        break;
    case Source::INTERSECTION_TURN:
        intersection_turn_.valid = false;
        break;
    case Source::GATE_WAIT:
        gate_wait_.valid = false;
        break;
    case Source::TRAFFIC_LIGHT_WAIT:
        traffic_light_wait_.valid = false;
        break;
    case Source::PARKING:
        parking_.valid = false;
        break;
    case Source::EMERGENCY_STOP:
        emergency_stop_ = false;
        break;
    default:
        break;
    }
}

void SpeedMixer::ClearAll() {
    vision_track_.valid = false;
    overtake_.valid = false;
    intersection_turn_.valid = false;
    gate_wait_.valid = false;
    traffic_light_wait_.valid = false;
    parking_.valid = false;
    emergency_stop_ = false;
}

float SpeedMixer::GetFinalLeftSpeed() const {
    Source src = GetHighestPrioritySource();
    switch (src) {
    case Source::EMERGENCY_STOP:
        return 0.0f;
    case Source::PARKING:
        return StdMath::fclamp(parking_.left, MAX_SPEED);
    case Source::OVERTAKE:
        return StdMath::fclamp(overtake_.left, MAX_SPEED);
    case Source::INTERSECTION_TURN:
        return StdMath::fclamp(intersection_turn_.left, MAX_SPEED);
    case Source::TRAFFIC_LIGHT_WAIT:
        return StdMath::fclamp(traffic_light_wait_.left, MAX_SPEED);
    case Source::GATE_WAIT:
        return StdMath::fclamp(gate_wait_.left, MAX_SPEED);
    case Source::VISION_TRACK:
        if (vision_track_.valid)
            return StdMath::fclamp(vision_track_.base_speed - vision_track_.speed_diff, MAX_SPEED);
        break;
    default:
        break;
    }
    return 0.0f;
}

float SpeedMixer::GetFinalRightSpeed() const {
    Source src = GetHighestPrioritySource();
    switch (src) {
    case Source::EMERGENCY_STOP:
        return 0.0f;
    case Source::PARKING:
        return StdMath::fclamp(parking_.right, MAX_SPEED);
    case Source::OVERTAKE:
        return StdMath::fclamp(overtake_.right, MAX_SPEED);
    case Source::INTERSECTION_TURN:
        return StdMath::fclamp(intersection_turn_.right, MAX_SPEED);
    case Source::TRAFFIC_LIGHT_WAIT:
        return StdMath::fclamp(traffic_light_wait_.right, MAX_SPEED);
    case Source::GATE_WAIT:
        return StdMath::fclamp(gate_wait_.right, MAX_SPEED);
    case Source::VISION_TRACK:
        if (vision_track_.valid)
            return StdMath::fclamp(vision_track_.base_speed + vision_track_.speed_diff, MAX_SPEED);
        break;
    default:
        break;
    }
    return 0.0f;
}

SpeedMixer::Source SpeedMixer::GetHighestPrioritySource() const {
    if (emergency_stop_)
        return Source::EMERGENCY_STOP;
    if (parking_.valid)
        return Source::PARKING;
    if (overtake_.valid)
        return Source::OVERTAKE;
    if (intersection_turn_.valid)
        return Source::INTERSECTION_TURN;
    if (traffic_light_wait_.valid)
        return Source::TRAFFIC_LIGHT_WAIT;
    if (gate_wait_.valid)
        return Source::GATE_WAIT;
    if (vision_track_.valid)
        return Source::VISION_TRACK;
    return Source::NONE;
}