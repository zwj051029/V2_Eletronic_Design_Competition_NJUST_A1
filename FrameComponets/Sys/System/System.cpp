#include "System.hpp"
#include "bsp_dwt.h"

RobotSystem &System = RobotSystem::GetInstance();

RobotSystem::RobotSystem()
    : status_led('C', 13, false), // 低电平点亮
      last_app_normal_(true), blink_counter_(0) {
}

void RobotSystem::Init(bool self_check) {
    BspDwt_Init(CPU_HERT_F407_MHZ);
    status_led.On(); // 初始化完成，点亮 LED
}

void RobotSystem::Run() {
    runtime_tick = BspDwt_GetTimeline_Sec();

    // LED 状态更新
    _UpdateLed();
}

void RobotSystem::_UpdateLed() {
    bool all_normal = true;
    for (int i = 0; i < app_count; i++) {
        if (app_list[i] != nullptr && app_list[i]->status != App::Normal) {
            all_normal = false;
            break;
        }
    }

    if (all_normal) {
        status_led.On();
        blink_counter_ = 0;
    } else {
        blink_counter_++;
        if (blink_counter_ >= BLINK_HALF_PERIOD) {
            blink_counter_ = 0;
            status_led.Toggle();
        }
    }
}

bool RobotSystem::RegistApp(Application &app_inst) {
    if (app_count >= 24)
        return false;
    app_list[app_count++] = &app_inst;
    app_list[app_count - 1]->Start();
    return true;
}

void RobotSystem::_Update_Applications() {
    for (int i = 0; i < app_count; i++) {
        if (app_list[i] != nullptr) {
            if (app_list[i]->CntFull()) {
                app_list[i]->status = app_list[i]->GetStatus();
                app_list[i]->Update();
            }
        }
    }
}

bool Application::CntFull() {
    if (++prescaler_cnt >= prescaler) {
        prescaler_cnt = 0;
        return true;
    }
    return false;
}