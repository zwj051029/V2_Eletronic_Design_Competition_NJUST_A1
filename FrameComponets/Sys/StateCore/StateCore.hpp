#ifndef __STATE_CORE_HPP__
#define __STATE_CORE_HPP__

#include "SysDefs.hpp"
#include "bsp_uart.h"
#include "string.h"

class StateBlock;
class StateCore;

#define PASTE2(a, b) a##b
#define NEW_STATE(core, name)                                                                                          \
    StateBlock PASTE2(St_, name)(#name, PASTE2(Robo, name));                                                           \
    core.AddState(&PASTE2(St_, name));

/**
 * @brief 状态链接，代表了状态之间的转换关系
 */
typedef struct {
    bool *condition;       // 条件
    StateBlock *nextState; // 下一个状态
} StateLink;

/**
 * @brief 状态块，代表了一个状态
 * @warning 状态块是少有的在构造函数中初始化的类，因此绝不能被定义成全局变量
 * 因为这会使得其在HAL库初始化之前就被初始化，从而引发不可预知的错误
 */
class StateBlock {
public:
    StateBlock() {};
    StateBlock(const char *name) {
        strncpy(this->name, name, 15);
        this->name[15] = '\0'; // 确保字符串以'\0'结尾
        linkNums = 0;
        Complete = false;
    }

    void (*StateAction)(StateCore *core); // 状态执行函数
    char name[16];                        // 状态名称
    uint8_t id;                           // 状态ID
    StateLink links[16];                  // 状态链接
    uint8_t linkNums;                     // 状态链接数量
    bool Complete;                        // 状态完成标志

    /// @brief 添加状态链接
    bool LinkTo(bool *condition, StateBlock &nextState);

    /// @brief 状态转换函数
    uint8_t Transition();
};

/**
 * @brief 状态图，包含了一系列状态块
 */
class StateGraph {
    friend class StateCore;

private:
    void (*GlobalAction)(StateCore *core); // 全局状态函数

public:
    StateGraph(const char *name) {
        stateNums = 0;
        strncpy(this->name, name, 15);
        this->name[15] = '\0'; // 确保字符串以'\0'结尾
    }

    char name[16];         // 状态图名称
    StateBlock states[24]; // 状态图包含的状态块，最多24个
    uint8_t stateNums;     // 状态图包含的状态块数量

    uint8_t executor_at_id = 0;            // 当前执行状态块的ID
    StateBlock &current_state = states[0]; // 当前状态块

    /// @brief 添加状态块，返回添加的状态块的引用
    StateBlock &AddState(const char *name);

    /// @brief 设置全局状态函数
    void SetGlobalAct(void (*GlobalAction)(StateCore *core));

    /// @brief 状态机的简并初始化，一般用于调试
    bool Degenerate(void (*DegenAction)(StateCore *core));
};

/**
 * @brief 状态机核心，即状态机本体
 */
class StateCore {
    SINGLETON(StateCore) {};

private:
    bool _enabled = false; // 状态机核心是否启用
    uint32_t _dwt_tick;    // dwt计时器句柄
    float _dt;             // 两次状态切换的时间间隔，单位为秒

public:
    StateGraph *graphs[4]; // 状态机核心包含的状态图，最多4个
    uint8_t graphNums = 0; // 状态机核心包含的状态图数量
    uint8_t at_graph_id;   // 当前执行状态图的ID

    /// @brief 循环运行状态机核心
    void Run(void);

    /// @brief 启动状态机核心，可指定初始状态图
    void Enable(uint8_t first_graph = 0);

    /// @brief 注册状态图
    void RegistGraph(StateGraph &graph);

    /// @brief 获得当前状态的引用
    StateBlock &GetCurState();

    /// @brief 根据状态图输出Mermaid代码
    static void CoreGraph(const StateGraph &graph);
};

namespace Seq {
    void Wait(float sec);
    void WaitUntil(bool &condition, float timeout_sec = 3600.0f);

    using CheckFunctionPtr = bool (*)(void *);

    namespace _Private {
        /**
         * @brief 等待直到（私有代理函数）
         * @param check_func_ptr: 实际检查函数的地址
         * @param context: 传递给检查函数的Lambda闭包
         * @param timeout_sec: 超时时间，单位秒
         * @note 注意：Lambda 对象 'condition' 是在栈上创建的，它的生命周期仅在 WaitUntil 期间有效。
         */
        void WaitUntil_Impl(CheckFunctionPtr check_func_ptr, void *context, float timeout_sec);
    } // namespace _Private

    /**
     * @brief 等待直到（表达式重载）
     * @param condition 布尔条件表达式 (Lambda/Functor)
     * @param timeout_sec 超时时间，单位秒
     */
    template <typename ConditionFunc>
    void WaitUntil(ConditionFunc condition, float timeout_sec = 3600.0f) {
        // 注意：Lambda 闭包对象会被传递到这里作为 context
        static auto callback_adapter = [](void *context) -> bool {
            // 将 void* 转换回原始的 Lambda 指针
            ConditionFunc *lambda_ptr = static_cast<ConditionFunc *>(context);
            // 执行 Lambda 返回结果
            return (*lambda_ptr)();
        };

        // 调用私有代理函数，传递 Lambda 对象的地址作为 context
        _Private::WaitUntil_Impl(callback_adapter, &condition, timeout_sec);
    }
} // namespace Seq

#endif /* __STATE_CORE_HPP__ */
