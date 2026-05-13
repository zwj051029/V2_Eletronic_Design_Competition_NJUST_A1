#include "PathPlanner.hpp"
#include <cstdio>
#include <cstring>

PathPlanner &path_planner = PathPlanner::GetInstance();

// ====================== 公开接口 ======================
bool PathPlanner::Compute(const Vec2 *input_points, int input_len) {
    if (input_len < 2 || input_len > MAX_FULL_PATH)
        return false;

    // 临时存储全路径，先放起点
    Vec2 temp_path[MAX_FULL_PATH];
    int total_len = 0;

    // 逐个处理输入点对，用 BFS 补全中间节点
    for (int i = 0; i < input_len - 1; i++) {
        Vec2 start = input_points[i];
        Vec2 end = input_points[i + 1];

        // 如果两点相同，跳过
        if (start == end)
            continue;

        // BFS 搜索从 start 到 end 的最短路径
        Vec2 segment_path[MAX_FULL_PATH];
        int seg_len = BfsShortestPath(start, end, segment_path, MAX_FULL_PATH);
        if (seg_len == 0) {
            // BFS 失败（理论上不会，除非输入点超出地图范围）
            return false;
        }

        // 将这段路径追加到 temp_path 中（跳过起点，避免与上一段终点重复）
        int start_idx = (total_len == 0) ? 0 : 1; // 第一段保留起点，后续段跳过起点
        for (int j = start_idx; j < seg_len; j++) {
            if (total_len >= MAX_FULL_PATH)
                return false;
            temp_path[total_len++] = segment_path[j];
        }
    }

    if (total_len < 2)
        return false; // 至少需要两个点

    // 将临时路径复制到正式成员变量
    memcpy(full_path_, temp_path, total_len * sizeof(Vec2));
    full_len_ = total_len;

    // 计算每个路口的转向动作
    // 注意：转向表的大小 = full_len_ - 1（有 n 个点则有 n-1 个路段，最后一个路段后是到达终点）
    for (int i = 0; i < full_len_ - 1; i++) {
        Vec2 curr = full_path_[i];
        Vec2 next = full_path_[i + 1];
        Vec2 dir_curr = next - curr; // 当前路段方向向量

        if (i == full_len_ - 2) {
            // 最后一个路段：到达终点
            turn_table_[i] = TurnDirection::ARRIVED;
        } else {
            Vec2 after_next = full_path_[i + 2];
            Vec2 dir_next = after_next - next; // 下一路段方向向量

            // 叉积：dir_curr.x * dir_next.y - dir_curr.y * dir_next.x
            float cross = dir_curr.x * dir_next.y - dir_curr.y * dir_next.x;

            if (cross > 0.1f) {
                turn_table_[i] = TurnDirection::LEFT;
            } else if (cross < -0.1f) {
                turn_table_[i] = TurnDirection::RIGHT;
            } else {
                turn_table_[i] = TurnDirection::STRAIGHT;
            }
        }
    }

    return true;
}

TurnDirection PathPlanner::GetAction(int index) const {
    if (index < 0 || index >= full_len_ - 1)
        return TurnDirection::STRAIGHT; // 错误返回直行（实际不应发生）
    return turn_table_[index];
}

Vec2 PathPlanner::GetTargetPoint(int index) const {
    if (index < 0 || index >= full_len_ - 1)
        return Vec2(0, 0);
    return full_path_[index + 1]; // 下一路口坐标即为目标点
}

// ====================== BFS 实现 ======================
int PathPlanner::BfsShortestPath(Vec2 start, Vec2 end, Vec2 *path_out, int max_len) {
    // 节点 ID = y * MAP_WIDTH + x
    int start_id = (int) (start.y * MAP_WIDTH + start.x);
    int end_id = (int) (end.y * MAP_WIDTH + end.x);
    if (start_id == end_id) {
        path_out[0] = start;
        return 1;
    }

    // visited 和 parent 数组
    int total_nodes = MAP_WIDTH * MAP_HEIGHT;
    int visited[20] = {0}; // 4x5
    int parent[20] = {-1};
    int queue[20];
    int front = 0, rear = 0;

    visited[start_id] = 1;
    queue[rear++] = start_id;

    bool found = false;
    while (front < rear) {
        int curr_id = queue[front++];
        if (curr_id == end_id) {
            found = true;
            break;
        }

        int cx = curr_id % MAP_WIDTH;
        int cy = curr_id / MAP_WIDTH;

        // 四个方向
        int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int d = 0; d < 4; d++) {
            int nx = cx + dirs[d][0];
            int ny = cy + dirs[d][1];
            if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT)
                continue;
            int nid = ny * MAP_WIDTH + nx;
            if (!visited[nid]) {
                visited[nid] = 1;
                parent[nid] = curr_id;
                queue[rear++] = nid;
            }
        }
    }

    if (!found)
        return 0; // 找不到路径

    // 回溯路径
    int path_len = 0;
    int cur = end_id;
    while (cur != start_id) {
        int x = cur % MAP_WIDTH;
        int y = cur / MAP_WIDTH;
        path_out[path_len++] = Vec2((float) x, (float) y);
        cur = parent[cur];
    }
    path_out[path_len++] = start; // 加入起点

    // 反转路径（从起点到终点）
    for (int i = 0; i < path_len / 2; i++) {
        Vec2 tmp = path_out[i];
        path_out[i] = path_out[path_len - 1 - i];
        path_out[path_len - 1 - i] = tmp;
    }

    return path_len;
}

// 辅助函数：获取邻居（未直接用到，保留备用）
int PathPlanner::GetNeighbors(Vec2 node, Vec2 neighbors[4]) {
    int count = 0;
    int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int d = 0; d < 4; d++) {
        int nx = (int) (node.x + dirs[d][0]);
        int ny = (int) (node.y + dirs[d][1]);
        if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
            neighbors[count++] = Vec2((float) nx, (float) ny);
        }
    }
    return count;
}