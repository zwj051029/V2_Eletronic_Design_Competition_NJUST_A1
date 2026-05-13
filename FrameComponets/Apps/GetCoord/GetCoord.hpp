#ifndef APP_GETCOORD_HPP_
#define APP_GETCOORD_HPP_

#include "SysDefs.hpp"
#include "System.hpp"
#include "bluetooth.hpp"
#include "std_math.hpp"

class GetCoord : public Application {
    friend class MainStateMachine;

    SINGLETON(GetCoord) : Application("GetCoord") {
        prescaler = 5; // 40 Hz
    };
    APPLICATION_OVERRIDE; // 自动声明 Start, Update, GetType

public:
    static constexpr int MAX_POINTS = 16;

    bool IsPathReady() const {
        return path_ready_;
    }
    const Vec2 *GetPoints() const {
        return points_;
    }
    int GetPointCount() const {
        return point_count_;
    }

    App::Status GetStatus() override {
        return App::Normal;
    }

private:
    Bluetooth bt_; // 非单例蓝牙对象
    Vec2 points_[MAX_POINTS];
    int point_count_ = 0;
    bool path_ready_ = false;

    void ParseLine(const char *line);
};

extern GetCoord &get_coord_app;

#endif