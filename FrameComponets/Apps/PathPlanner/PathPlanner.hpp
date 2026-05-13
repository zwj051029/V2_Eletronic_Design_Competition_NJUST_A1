#ifndef APP_PATHPLANNER_HPP_
#define APP_PATHPLANNER_HPP_

#include "SysDefs.hpp"
#include "std_math.hpp"
#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief 转向动作枚举
 */
enum class TurnDirection {
    STRAIGHT = 0, // 直行通过路口
    LEFT = 1,     // 左转
    RIGHT = 2,    // 右转
    ARRIVED = 3   // 已到达终点/目的地
};

/**
 * @brief 路径规划器（单例）
 * @note  输入：用户指定的途径点坐标（Vec2 数组）
 *        输出：补全后的全路径点序列 + 每个路口对应的转向动作
 */
class PathPlanner {
    SINGLETON(PathPlanner) {};

public:
    static constexpr int MAX_FULL_PATH = 32; // 全路径最多点数
    static constexpr int MAP_WIDTH = 4;      // 网格宽度 (x: 0~3)
    static constexpr int MAP_HEIGHT = 5;     // 网格高度 (y: 0~4)

    /**
     * @brief 根据蓝牙输入的坐标序列，计算全路径和转向表
     * @param input_points  蓝牙收到的坐标序列（包含起点 0,0，终点 J，中间途径点）
     * @param input_len     输入点数量
     * @return true         规划成功
     */
    bool Compute(const Vec2 *input_points, int input_len);

    /// 获取全路径点序列（用于显示或其他用途）
    const Vec2 *GetFullPath() const {
        return full_path_;
    }
    int GetFullPathLength() const {
        return full_len_;
    }

    /// 获取转向表：到达第 i 个路口后应该执行的动作
    TurnDirection GetAction(int index) const;

    /// 获取当前路段的目标点（即下一个路口坐标）
    Vec2 GetTargetPoint(int index) const;

private:
    // 全路径点序列（所有相邻路口）
    Vec2 full_path_[MAX_FULL_PATH];
    int full_len_ = 0;

    // 转向动作表（与 full_path_ 长度对应，最后一个元素为 ARRIVED）
    TurnDirection turn_table_[MAX_FULL_PATH];

    // 内部辅助：BFS 在网格上搜索两点间最短路径，返回路径点序列，失败返回 0
    int BfsShortestPath(Vec2 start, Vec2 end, Vec2 *path_out, int max_len);

    // 构建网格邻接关系，返回指定节点的邻居数量（最多 4）
    int GetNeighbors(Vec2 node, Vec2 neighbors[4]);
};

extern PathPlanner &path_planner;
#endif