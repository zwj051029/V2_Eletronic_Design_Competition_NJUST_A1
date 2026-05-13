#include "MainStateMachine.hpp"
#include "GetCoord.hpp"
#include "PathPlanner.hpp"
#include "SpeedMixer.hpp"
#include "StateCore.hpp"
#include "System.hpp"
#include "VisionTrack.hpp"

StateGraph MainStateMachine::main_graph_("MainGraph");
StateBlock *MainStateMachine::st_idle_ = nullptr;
StateBlock *MainStateMachine::st_vision_track_ = nullptr;
StateBlock *MainStateMachine::st_intersection_turn_ = nullptr;
StateBlock *MainStateMachine::st_red_light_wait_ = nullptr;
StateBlock *MainStateMachine::st_obstacle_bypass_ = nullptr;
StateBlock *MainStateMachine::st_gate_wait_ = nullptr;
StateBlock *MainStateMachine::st_parking_ = nullptr;
StateBlock *MainStateMachine::st_finish_ = nullptr;

bool MainStateMachine::cond_start = false;          // 空闲状态等待系统启动条件
bool MainStateMachine::cond_intersection = false;   // 视觉跟踪状态等待进入路口条件
bool MainStateMachine::cond_turn_done = false;      // 路口转弯状态等待转弯完成条件
bool MainStateMachine::cond_red_light = false;      // 视觉跟踪状态等待红灯条件
bool MainStateMachine::cond_green_light = false;    // 红灯等待状态等待绿灯条件
bool MainStateMachine::cond_obstacle = false;       // 视觉跟踪状态等待检测到障碍物条件
bool MainStateMachine::cond_obstacle_clear = false; // 障碍物绕行状态等待障碍物清除条件
bool MainStateMachine::cond_arrive_gate = false;    // 视觉跟踪状态等待到达大门条件
bool MainStateMachine::cond_gate_open = false;      // 大门等待状态等待大门打开条件
bool MainStateMachine::cond_parking_done = false;   // 停车状态等待停车完成条件
bool MainStateMachine::cond_path_done = false;      // 视觉跟踪状态等待路径完成条件

// ====================== 公开接口 ======================
void MainStateMachine::Init() {
    RegisterAllApps(); // 1. 注册 App（自动调用各 App 的 Start 进行硬件初始化）
    InitStateBlocks(); // 2. 创建状态块并绑定动作
    InitTransitions(); // 3. 设置状态转换

    StateCore &core = StateCore::GetInstance();
    core.RegistGraph(main_graph_);
    core.Enable(0); // 启动状态机（进入 Idle）
}

const char *MainStateMachine::GetCurrentName() {
    return StateCore::GetInstance().GetCurState().name;
}

// ====================== 注册所有 App ======================
void MainStateMachine::RegisterAllApps() {
    System.RegistApp(get_coord_app);
    System.RegistApp(vision_track);
    System.RegistApp(speed_mixer);
}

// ====================== 状态块初始化 ======================
void MainStateMachine::InitStateBlocks() {
    st_idle_ = &main_graph_.AddState("Idle");
    st_vision_track_ = &main_graph_.AddState("VisionTrack");
    st_intersection_turn_ = &main_graph_.AddState("IntersectionTurn");
    st_red_light_wait_ = &main_graph_.AddState("RedLightWait");
    st_obstacle_bypass_ = &main_graph_.AddState("ObstacleBypass");
    st_gate_wait_ = &main_graph_.AddState("GateWait");
    st_parking_ = &main_graph_.AddState("Parking");
    st_finish_ = &main_graph_.AddState("Finish");

    st_idle_->StateAction = ActionIdle;
    st_vision_track_->StateAction = ActionVisionTrack;
    st_intersection_turn_->StateAction = ActionIntersectionTurn;
    st_red_light_wait_->StateAction = ActionRedLightWait;
    st_obstacle_bypass_->StateAction = ActionObstacleBypass;
    st_gate_wait_->StateAction = ActionGateWait;
    st_parking_->StateAction = ActionParking;
    st_finish_->StateAction = ActionFinish;
}

void MainStateMachine::InitTransitions() {
    st_idle_->LinkTo(&cond_start, *st_vision_track_);

    st_vision_track_->LinkTo(&cond_intersection, *st_intersection_turn_);
    st_vision_track_->LinkTo(&cond_red_light, *st_red_light_wait_);
    st_vision_track_->LinkTo(&cond_obstacle, *st_obstacle_bypass_);
    st_vision_track_->LinkTo(&cond_arrive_gate, *st_gate_wait_);
    st_vision_track_->LinkTo(&cond_path_done, *st_parking_);

    st_intersection_turn_->LinkTo(&cond_turn_done, *st_vision_track_);
    st_red_light_wait_->LinkTo(&cond_green_light, *st_vision_track_);
    st_obstacle_bypass_->LinkTo(&cond_obstacle_clear, *st_vision_track_);
    st_gate_wait_->LinkTo(&cond_gate_open, *st_vision_track_);
    st_parking_->LinkTo(&cond_parking_done, *st_finish_);
}

// ====================== 状态动作函数（骨架） ======================
void MainStateMachine::ActionIdle(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("System is idle. Waiting for START command...\r\n");
        first_entry = false;
    }

    get_coord_app.SetEnable(true);
    vision_track.SetEnable(false);

    speed_mixer.ClearAll();

    cond_start = System.system_started;
}

void MainStateMachine::ActionVisionTrack(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering VisionTrack state. Starting path following...\r\n");
        first_entry = false;
    }

    get_coord_app.SetEnable(false);
    vision_track.SetEnable(true);

    speed_mixer.ClearSource(SpeedMixer::Source::INTERSECTION_TURN);
    speed_mixer.ClearSource(SpeedMixer::Source::TRAFFIC_LIGHT_WAIT);
    speed_mixer.ClearSource(SpeedMixer::Source::OVERTAKE);
    speed_mixer.ClearSource(SpeedMixer::Source::GATE_WAIT);
    speed_mixer.ClearSource(SpeedMixer::Source::PARKING);
}

void MainStateMachine::ActionIntersectionTurn(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering IntersectionTurn state. Executing turn...\r\n");
        first_entry = false;
    }
    // 待实现：IntersectionTurn App
}

void MainStateMachine::ActionRedLightWait(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering RedLightWait state. Waiting for green light...\r\n");
        first_entry = false;
    }
    // 待实现：TrafficLightWait App
}

void MainStateMachine::ActionObstacleBypass(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering ObstacleBypass state. Bypassing obstacle...\r\n");
        first_entry = false;
    }
    // 待实现：Overtake App
}

void MainStateMachine::ActionGateWait(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering GateWait state. Waiting for gate to open...\r\n");
        first_entry = false;
    }
    // 待实现：GateWait App
}

void MainStateMachine::ActionParking(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering Parking state. Executing parking maneuver...\r\n");
        first_entry = false;
    }
    // 待实现：Parking App
}

void MainStateMachine::ActionFinish(StateCore *core) {
    static bool first_entry = true;
    if (first_entry) {
        get_coord_app.bt_.Send("Entering Finish state. Path completed!\r\n");
        first_entry = false;
    }
    // 待实现：完成状态，可能需要停止所有运动并闪烁 LED
}