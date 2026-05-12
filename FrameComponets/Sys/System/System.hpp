#pragma once

#include "StateCore.hpp"
#include "SysDefs.hpp"
#include "arm_math.h"
#include "std_cpp.h"
#include "std_math.hpp"
#include "stm32f4xx_hal.h"
#include "typeinfo"

namespace App {
    enum Status : uint8_t {
        Normal = 0,  // 运行正常
        Warning = 1, // 局部异常，正在尝试本地恢复或降级运行
        Error = 2,   // 致命异常，相关依赖项需立即响应并保护
    };
}

class Application {
    friend class RobotSystem;

private:
    char name[24];             // 应用名称
    uint8_t prescaler_cnt = 0; // 预分频计数器

public:
    uint8_t prescaler = 1; // 应用预分频
    bool CntFull();        // 监测预分频计数器是否满了

    // [开机自检] 系统 SELF_CHECK 阶段集中调用，子类按需重写
    // 返回 true 表示硬件/连接正常，自检通过
    virtual bool WatchPoint() {
        return true;
    }

    // [状态量] 用于向系统与其他 App 暴露当前健康状况，外部仅具有只读权限
    // 注意：App 内部不要直接修改这个变量，应当覆写下方的 GetStatus()
    App::Status status = App::Normal;

    // [状态更新接口] 子类只需覆写此方法描述自身状态
    // 该方法会在 Update 执行前被 System 自动调用并缓存至 status 变量
    virtual App::Status GetStatus() {
        return App::Normal;
    }

    const char *GetName() const {
        return name;
    }

protected:
    Application(const char *name) {
        strncpy(this->name, name, 23);
        this->name[23] = '\0'; // 确保字符串结尾
    }

    // 纯虚函数，要求子类必须实现这些方法来定义应用的行为
    virtual void Start() = 0;                    // 启动应用
    virtual void Update() = 0;                   // 更新应用
    virtual const std::type_info &GetType() = 0; // 获取应用类型
};

/**
 * @brief 机器人系统
 * @warning 机器人系统是一个单例类，禁止实例化多个对象
 */
class RobotSystem {
    friend void RobotSystemCpp();
    friend void ApplicationCpp();
    friend void StateCoreCpp();

    SINGLETON(RobotSystem) {};

private:
    void _Update_Applications();

    Application *app_list[24]; // 系统中的应用实例列表
    uint8_t app_count = 0;     // 当前注册的应用实例数量

public:
    float runtime_tick; // 全局时间戳，单位s

    /// @brief 全局唯一的自动状态机核心
    const StateCore &core = StateCore::GetInstance();

    /// @brief 系统初始化
    void Init(bool self_check = true);

    /// @brief 运行机器人系统主进程
    void Run();

    /// @brief 注册应用实例
    bool RegistApp(Application &app_inst);

    /// @brief 查找应用实例
    template <typename T>
    T *FindApp(const char *name);
};

/// @brief 全局唯一的机器人系统实例
extern RobotSystem &System;
