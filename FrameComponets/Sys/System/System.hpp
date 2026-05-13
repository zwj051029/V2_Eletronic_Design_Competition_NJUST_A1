#pragma once

#include "StateCore.hpp"
#include "SysDefs.hpp"
#include "arm_math.h"
#include "led.hpp"
#include "std_cpp.h"
#include "std_math.hpp"
#include "stm32f4xx_hal.h"
#include "typeinfo"

namespace App {
    enum Status : uint8_t {
        Normal = 0,
        Warning = 1,
        Error = 2,
    };
}

class Application {
    friend class RobotSystem;

private:
    char name[24];
    uint8_t prescaler_cnt = 0;

public:
    uint8_t prescaler = 1;
    bool is_enabled = false;
    bool CntFull();

    void SetEnable(bool enable) {
        is_enabled = enable;
    }

    virtual bool WatchPoint() {
        return true;
    }

    App::Status status = App::Normal;

    virtual App::Status GetStatus() {
        return App::Normal;
    }

    const char *GetName() const {
        return name;
    }

protected:
    Application(const char *name) {
        strncpy(this->name, name, 23);
        this->name[23] = '\0';
    }

    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual const std::type_info &GetType() = 0;
};

class RobotSystem {
    friend void RobotSystemCpp();
    friend void ApplicationCpp();
    friend void StateCoreCpp();

    SINGLETON(RobotSystem);

private:
    // 硬件
    Led status_led; // PC13 低电平点亮

    // 应用管理
    void _Update_Applications();
    Application *app_list[24];
    uint8_t app_count = 0;

    // LED 闪烁相关
    bool last_app_normal_;
    uint16_t blink_counter_;
    static constexpr uint16_t BLINK_HALF_PERIOD = 100; // 200Hz下500ms

public:
    float runtime_tick;

    const StateCore &core = StateCore::GetInstance();

    bool system_started = false; // 交由外部（如蓝牙 START）设为 true

    void Init(bool self_check = true);
    void Run();
    bool RegistApp(Application &app_inst);

private:
    void _UpdateLed();
};

extern RobotSystem &System;