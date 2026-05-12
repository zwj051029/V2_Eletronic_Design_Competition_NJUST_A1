#include "GetCoord.hpp"
#include "PathPlanner.hpp"
#include <cctype>
#include <cstdio>

GetCoord &get_coord_app = GetCoord::GetInstance();

void GetCoord::Start() {
    // 初始化蓝牙，传入 CubeMX 生成的 huart2
    bt_.Init(&huart1); // 假设 huart2 在 main.h 或 usart.h 中声明
    point_count_ = 0;
    path_ready_ = false;
}

void GetCoord::Update() {
    char line[64];
    if (!bt_.GetLine(line, sizeof(line)))
        return;

    // 如果收到 START 指令，结束接收并置标志
    if (strncmp(line, "START", 5) == 0) {
        path_ready_ = true;
        // 后续可在此调用路径规划
        // PathPlanner::GetInstance().Compute(points_, point_count_);
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