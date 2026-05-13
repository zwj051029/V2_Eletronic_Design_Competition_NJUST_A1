#include "GetCoord.hpp"
#include "PathPlanner.hpp"
#include <cctype>
#include <cstdio>

GetCoord &get_coord_app = GetCoord::GetInstance();

void GetCoord::Start() {
    bt_.Init(&huart2);
    point_count_ = 0;
    path_ready_ = false;
    // 预设起点 (0,0)
    points_[point_count_++] = Vec2(0.0f, 0.0f);
}

void GetCoord::Update() {
    char line[64];
    if (!bt_.GetLine(line, sizeof(line)))
        return;

    // 如果收到 START 指令，结束接收并置标志
    if (strncmp(line, "START", 5) == 0) {
        path_ready_ = true;
        // 调用路径规划
        PathPlanner &pl = PathPlanner::GetInstance();
        if (pl.Compute(points_, point_count_)) {
            // 通过蓝牙回显全路径和转向表
            char msg[64];
            const Vec2 *path = pl.GetFullPath();
            int len = pl.GetFullPathLength();
            bt_.Send("===== Full Path =====\r\n");
            for (int i = 0; i < len; i++) {
                snprintf(msg, sizeof(msg), "Node%d: (%d,%d)\r\n", i, (int) path[i].x, (int) path[i].y);
                bt_.Send(msg);
            }
            bt_.Send("===== Turn Table =====\r\n");
            for (int i = 0; i < len - 1; i++) {
                TurnDirection act = pl.GetAction(i);
                const char *act_str = "STRAIGHT";
                if (act == TurnDirection::LEFT)
                    act_str = "LEFT";
                if (act == TurnDirection::RIGHT)
                    act_str = "RIGHT";
                if (act == TurnDirection::ARRIVED)
                    act_str = "ARRIVED";
                snprintf(msg, sizeof(msg), "At Node%d(%d,%d): %s\r\n", i + 1, (int) path[i + 1].x, (int) path[i + 1].y,
                         act_str);
                bt_.Send(msg);
            }
        } else {
            bt_.Send("Path computation failed!\r\n");
        }
        System.system_started = true; // 启动系统
        return;
    }

    // 普通坐标行：提取第一个字母，再解析 x,y
    char name = 0;
    int x = -1, y = -1;

    const char *p = line;
    while (*p && isalpha((unsigned char) *p)) {
        if (name == 0)
            name = *p;
        p++;
    }
    if (name == 0)
        return; // 没有找到字母，忽略

    if (sscanf(p, "%d,%d", &x, &y) == 2) {
        // 范围校验（x: 1~3, y: 1~4）
        if (x >= 1 && x <= 3 && y >= 1 && y <= 4) {
            if (point_count_ < MAX_POINTS) {
                points_[point_count_++] = Vec2(static_cast<float>(x), static_cast<float>(y));
            }
            // 发送回显：例如 "A(1,1)\r\n"
            char echo[32];
            snprintf(echo, sizeof(echo), "%c(%d,%d)\r\n", name, x, y);
            bt_.Send(echo);
        }
    }
}