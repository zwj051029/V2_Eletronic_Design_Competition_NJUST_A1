#include "StateCore.hpp"
#include "bsp_dwt.h"
#include "cmsis_os.h"
#include "stdio.h"

/**
 * @brief 为状态块添加状态链接
 */
bool StateBlock::LinkTo(bool *condition, StateBlock &nextState) {
    if (linkNums < 16) {
        links[linkNums].condition = condition;
        links[linkNums].nextState = &nextState;
        linkNums++;
        return true;
    }
    return false;
}

/**
 * @brief 进行状态转移，返回下一个状态ID
 * @return 如果没有状态转移则返回当前状态ID，否则返回下一个状态ID
 */
uint8_t StateBlock::Transition() {
    for (int i = 0; i < linkNums; i++) {
        if (*(links[i].condition)) {
            return links[i].nextState->id;
        }
    }
    return id;
}

/**
 * @brief 向状态图中添加状态块
 */
StateBlock &StateGraph::AddState(const char *name) {
    if (stateNums < 24) {
        // 获取目标状态
        StateBlock &target = states[stateNums];

        // 初始化目标状态
        target = StateBlock(name);
        target.id = stateNums;
        target.Complete = false;

        // 增加状态数量
        stateNums++;
        return target;
    }
    // 如果状态数量已满，返回第一个状态（虽然这可能会导致问题，但这是一个简单的错误处理方式）
    return states[0];
}

/**
 * @brief 设置状态图的全局状态函数
 */
void StateGraph::SetGlobalAct(void (*GlobalAction)(StateCore *core)) {
    this->GlobalAction = GlobalAction;
}

/**
 * @brief 状态机的简并初始化，一般用于调试
 * @details 只有两个状态：working和end
 */
bool StateGraph::Degenerate(void (*DegenAction)(StateCore *core)) {
    // 固定两个状态

    // 第一个状态：working
    StateBlock &state_work = AddState("working");
    state_work.StateAction = DegenAction;
    state_work.LinkTo(&(states[0].Complete), states[1]); // 当working状态完成时，转移到end状态

    // 第二个状态：end
    StateBlock &state_end = AddState("end");
    state_end.StateAction = nullptr; // end状态没有执行函数

    return true;
}

/**
 * @brief 运行状态机核心
 */
void StateCore::Run(void) {
    // 避免空运行
    if (graphNums < 0 || !_enabled) {
        return;
    }

    // 计算时间间隔
    _dt = BspDwt_GetDeltaTime(&_dwt_tick);

    // 执行 `当前状态图` 的 `对应状态`的 状态函数
    StateGraph &graph = *graphs[at_graph_id];
    StateBlock &state = graph.current_state;

    // 执行当前状态图的全局状态函数
    if (graph.GlobalAction != nullptr) {
        graph.GlobalAction(this);
    }

    /**
     * @note 这个写法代表着，一般只有状态函数完全执行完了才会进行状态转移
     * 所以后面应该会加入 在中间打断动作 的机制（确保动作打断是经过作者设计的）
     * @warning 只有非空状态函数才会被执行
     */
    if (state.StateAction != nullptr) {
        state.StateAction(this);
    }

    // 进行状态转移
    graph.executor_at_id = state.Transition();
    graph.current_state = graph.states[graph.executor_at_id];
}

/**
 * @brief 启动状态机核心
 */
void StateCore::Enable(uint8_t first_graph) {
    if (first_graph < graphNums) {
        at_graph_id = first_graph;
        _enabled = true;
    }
}

/**
 * @brief 注册状态图
 */
void StateCore::RegistGraph(StateGraph &graph) {
    if (graphNums < 4) {
        graphs[graphNums] = &graph;
        graphNums++;
    } else {
        // // 处理状态图数量已满的情况
        // Monitor::GetInstance().LogWarning("StateCore: Too much state graph!");
    }
}

/**
 * @brief 获得当前状态的引用
 */
StateBlock &StateCore::GetCurState() {
    StateGraph &graph = *graphs[at_graph_id];
    return graph.current_state;
}

/**
 * @brief 绘制状态机图
 * @details 通过遍历整个状态机，将状态机的状态和状态转换关系以图的形式发送到指定串口上 'Mermaid'
 * @warning 该函数会阻塞程序运行！！因此禁止在线程中调用，仅做Debug用途
 */
// void StateCore::CoreGraph(const StateGraph &graph) {
//     uint8_t buf[60];
//     Monitor::GetInstance().LogInfo("StateGraph\n");

//     HAL_Delay(10);
//     for (int i = 0; i < graph.stateNums; i++) {
//         // 发送状态转换关系（mermaid格式）
//         for (int j = 0; j < graph.states[i].linkNums; j++) {
//             int len = snprintf((char *) buf, 48, "%s --> %s\n", graph.states[i].name,
//                                graph.states[i].links[j].nextState->name);
//             Monitor::GetInstance().LogInfo((char *) buf);
//             HAL_Delay(10);
//         }
//     }
// }

namespace Seq {
    /**
     * @brief 等待指定时间
     * @param sec 等待时间，单位秒
     * @details 利用osDelay实现
     */
    void Wait(float sec) {
        osDelay((uint32_t) (sec * 1000)); // 将秒转换为毫秒
    }

    /**
     * @brief 等待直到条件满足或超时
     * @param condition 指向布尔条件的引用
     * @param timeout_sec 超时时间，单位秒，默认300秒
     * @details 利用阻塞+让步实现
     */
    void WaitUntil(bool &condition, float timeout_sec) {
        uint32_t start_tick = xTaskGetTickCount();                // 获取当前系统时间（tick）
        uint32_t timeout_ticks = (uint32_t) (timeout_sec * 1000); // 将超时时间转换为tick

        // 储存当前任务优先级
        osPriority original_priority = osThreadGetPriority(osThreadGetId());
        // 降低任务优先级，避免死循环占用过多CPU时间
        osThreadSetPriority(osThreadGetId(), osPriorityIdle);

        while (!condition) {
            // 检查超时
            uint32_t current_tick = xTaskGetTickCount();
            if ((current_tick - start_tick) >= timeout_ticks) {
                break;
            }

            // 让出CPU时间，避免死循环占用过多资源
            osThreadYield();
        }

        // 恢复任务优先级
        osThreadSetPriority(osThreadGetId(), original_priority);
    }

    namespace _Private {
        // 私有代理函数的实现
        void WaitUntil_Impl(CheckFunctionPtr check_func_ptr, void *context, float timeout_sec) {
            uint32_t start_tick = xTaskGetTickCount();
            uint32_t timeout_ticks = (uint32_t) (timeout_sec * 1000);

            // 存储当前任务优先级
            osThreadId current_thread_id = osThreadGetId();
            osPriority original_priority = osThreadGetPriority(current_thread_id);
            // 降低任务优先级，避免死循环占用过多CPU时间
            osThreadSetPriority(osThreadGetId(), osPriorityIdle);

            // 使用传入的函数指针和上下文来检查条件
            while (!check_func_ptr(context)) {
                // 检查超时
                uint32_t current_tick = xTaskGetTickCount();
                if ((current_tick - start_tick) >= timeout_ticks) {
                    break;
                }

                osThreadYield();
            }

            // 恢复任务优先级
            osThreadSetPriority(current_thread_id, original_priority);
        }
    } // namespace _Private
} // namespace Seq
