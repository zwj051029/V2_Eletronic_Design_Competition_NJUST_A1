#include "System.hpp"
#include "bsp_dwt.h"

RobotSystem &System = RobotSystem::GetInstance(); // 定义全局唯一的机器人系统实例

/**
 * @brief 初始化机器人系统
 */
void RobotSystem::Init(bool self_check) {
    // 初始化DWT计时器
    BspDwt_Init(CPU_HERT_F407_MHZ);
}

/**
 * @brief 运行机器人系统
 * @note 该方法应被周期性调用，以处理系统任务
 */
void RobotSystem::Run() {
    // 更新全局时间
    runtime_tick = BspDwt_GetTimeline_Sec();
}

/**
 * @brief 注册应用实例
 */
bool RobotSystem::RegistApp(Application &app_inst) {
    if (app_count >= 24) {
        return false;
    }

    app_list[app_count++] = &app_inst;
    app_list[app_count - 1]->Start(); // 注册后立即启动应用

    return true;
}

/**
 * @brief 查找应用实例
 * @note 通过应用类型和名称查找应用实例，返回指针，如果未找到则返回nullptr
 */
template <typename T>
T *RobotSystem::FindApp(const char *name) {
    for (int i = 0; i < 24; i++) {
        if (app_list[i] != nullptr) {
            // 首先对比类型
            if (typeid(app_list[i]->GetType()) == typeid(T)) {
                // 再对比名字
                if (strncmp(app_list[i]->name, name, 24) == 0) {
                    return dynamic_cast<T *>(app_list[i]);
                }
            }
        }
    }
    // 未找到匹配的应用实例
    return nullptr;
}

/**
 * @brief 运行所有应用实例
 */
void RobotSystem::_Update_Applications() {
    for (int i = 0; i < 24; i++) {
        if (app_list[i] != nullptr) {
            // 如果预分频计数器满了，就更新应用
            if (app_list[i]->CntFull()) {
                // 自动更新应用状态缓存，供零开销跨库查询
                app_list[i]->status = app_list[i]->GetStatus();
                app_list[i]->Update();
            }
        }
    }
}

/**
 * @brief 检查应用预分频计数器是否已满
 * @return true 表示计数器已满，false 表示未满
 */
bool Application::CntFull() {
    if (++prescaler_cnt >= prescaler) {
        prescaler_cnt = 0;
        return true;
    }
    return false;
}
