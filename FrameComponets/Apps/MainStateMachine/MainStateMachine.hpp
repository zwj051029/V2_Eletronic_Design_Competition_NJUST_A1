#ifndef MAINSTATE_MACHINE_HPP
#define MAINSTATE_MACHINE_HPP

#include "StateCore.hpp"

class MainStateMachine {
public:
    // 公共条件变量（供状态机 LinkTo 使用）
    static bool cond_start;
    static bool cond_intersection;
    static bool cond_turn_done;
    static bool cond_red_light;
    static bool cond_green_light;
    static bool cond_obstacle;
    static bool cond_obstacle_clear;
    static bool cond_arrive_gate;
    static bool cond_gate_open;
    static bool cond_parking_done;
    static bool cond_path_done;

    /**
     * @brief 初始化整个应用层：
     *        1. 注册所有 App 到 System
     *        2. 构建状态图并启动状态机
     */
    static void Init();

    /// 获取当前状态名（调试用）
    static const char *GetCurrentName();

private:
    static StateGraph main_graph_;
    static StateBlock *st_idle_;
    static StateBlock *st_vision_track_;
    static StateBlock *st_intersection_turn_;
    static StateBlock *st_red_light_wait_;
    static StateBlock *st_obstacle_bypass_;
    static StateBlock *st_gate_wait_;
    static StateBlock *st_parking_;
    static StateBlock *st_finish_;

    static void RegisterAllApps(); // 注册所有 App
    static void InitStateBlocks(); // 添加状态块并绑定动作
    static void InitTransitions(); // 设置状态转换链接

    // 状态动作函数
    static void ActionIdle(StateCore *core);
    static void ActionVisionTrack(StateCore *core);
    static void ActionIntersectionTurn(StateCore *core);
    static void ActionRedLightWait(StateCore *core);
    static void ActionObstacleBypass(StateCore *core);
    static void ActionGateWait(StateCore *core);
    static void ActionParking(StateCore *core);
    static void ActionFinish(StateCore *core);
};

#endif