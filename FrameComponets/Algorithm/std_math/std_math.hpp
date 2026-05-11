#ifndef __STD_MATH_HPP__
#define __STD_MATH_HPP__

#include "stm32f4xx_hal.h"

#define SQRT2 1.41421356237f
#define SQRT3 1.73205080757f

typedef uint8_t byte;

class Vec2;
class Vec3;

/**
 * @name 二维向量
 */
class Vec2 {
private:
    /* data */
public:
    float x, y;
    /******      构造函数      ******/
    Vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {
    }

    /******      基本运算函数      ******/
    /// @brief 归一化二维向量
    Vec2 Norm();

    /// @brief 计算二维向量的模
    float Length();

    /// @brief 计算二维向量的角度
    float Angle();

    /// @brief 旋转二维向量
    Vec2 Rotate(float angRad);

    /// @brief 转换为三维向量
    Vec3 ToVec3();

    /******      类型转换函数      ******/
    /// @brief 将Vec2转换入buffer
    void ToBytes(uint8_t *buffer);

    /// @brief 从buffer恢复Vec2
    void FromBytes(const uint8_t *buffer);

    /******      友元函数重载运算符      ******/
    friend Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs);
    friend Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs);
    friend Vec2 operator*(const Vec2 &vec, float scalar);
    friend Vec2 operator*(float scalar, const Vec2 &vec);
    friend Vec2 operator/(const Vec2 &vec, float scalar);
    friend bool operator==(const Vec2 &lhs, const Vec2 &rhs);
    friend bool operator!=(const Vec2 &lhs, const Vec2 &rhs);
};

inline Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs) {
    // 向量加法
    return Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
inline Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs) {
    // 向量减法
    return Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}
inline Vec2 operator*(const Vec2 &vec, float scalar) {
    // 向量数乘（向量在前）
    return Vec2(vec.x * scalar, vec.y * scalar);
}
inline Vec2 operator*(float scalar, const Vec2 &vec) {
    // 向量数乘（标量在前）
    return Vec2(vec.x * scalar, vec.y * scalar);
}
inline Vec2 operator/(const Vec2 &vec, float scalar) {
    // 向量数除
    if (scalar == 0)
        return Vec2(114514, 114514);
    else
        return Vec2(vec.x / scalar, vec.y / scalar);
}
inline bool operator==(const Vec2 &lhs, const Vec2 &rhs) {
    // 向量相等比较
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}
inline bool operator!=(const Vec2 &lhs, const Vec2 &rhs) {
    // 向量不等比较
    return !(lhs == rhs);
}

/**
 * @name 三维向量
 */
class Vec3 {
private:
    /* data */
public:
    float x, y, z;
    /******      构造函数      ******/
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {
    }
    Vec3() : x(0), y(0), z(0) {
    }

    /******      基本运算函数      ******/
    /// @brief 归一化三维向量
    Vec3 Norm();

    /// @brief 计算三维向量的模
    float Length();

    /// @brief 转换为二维向量
    Vec2 ToVec2();

    /******      类型转换函数      ******/
    /// @brief 将Vec3转换入buffer
    void ToBytes(uint8_t *buffer);

    /// @brief 从buffer恢复Vec3
    void FromBytes(const uint8_t *buffer);

    /******      友元函数重载运算符      ******/
    friend Vec3 operator+(const Vec3 &lhs, const Vec3 &rhs);
    friend Vec3 operator-(const Vec3 &lhs, const Vec3 &rhs);
    friend Vec3 operator*(const Vec3 &vec, float scalar);
    friend Vec3 operator*(float scalar, const Vec3 &vec);
    friend Vec3 operator/(const Vec3 &vec, float scalar);
};

inline Vec3 operator+(const Vec3 &lhs, const Vec3 &rhs) {
    return Vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}
inline Vec3 operator-(const Vec3 &lhs, const Vec3 &rhs) {
    return Vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}
inline Vec3 operator*(const Vec3 &vec, float scalar) {
    // 向量数乘（向量在前）
    return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}
inline Vec3 operator*(float scalar, const Vec3 &vec) {
    // 向量数乘（标量在前）
    return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}
inline Vec3 operator/(const Vec3 &vec, float scalar) {
    // 向量数除
    if (scalar == 0)
        return Vec3(114514, 114514, 114514); // 避免除以零
    else
        return Vec3(vec.x / scalar, vec.y / scalar, vec.z / scalar);
}

/**
 * @name 三维向量
 * @warning 颜色分量虽然是float类型，但其适配RGB24方案，取值范围应为0.0f~255.0f
 */
class Color {
private:
    /* data */
public:
    float r, g, b;
    // 构造函数的实现直接放在类定义中
    Color(float r, float g, float b) : r(r), g(g), b(b) {
    }
    Color() : r(0), g(0), b(0) {
    }

    // 预定义颜色
    static Color Red;
    static Color Green;
    static Color Blue;
    static Color White;

    // 友元函数重载运算符
    friend Color operator+(const Color &lhs, const Color &rhs);
    friend Color operator-(const Color &lhs, const Color &rhs);
    friend Color operator*(const Color &vec, float scalar);
    friend Color operator*(const Color &vec, Vec3 vecscalar);
    friend Color operator*(float scalar, const Color &vec);
    friend Color operator/(const Color &vec, float scalar);
};
/*********      运算符重载      **********/
inline Color operator+(const Color &lhs, const Color &rhs) {
    return Color(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b);
}
inline Color operator-(const Color &lhs, const Color &rhs) {
    return Color(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b);
}
inline Color operator*(const Color &vec, float scalar) { // 向量数乘（向量在前）
    return Color(vec.r * scalar, vec.g * scalar, vec.b * scalar);
}
inline Color operator*(const Color &vec, Vec3 vecscalar) { // 颜色哈达玛积
    return Color(vec.r * vecscalar.x, vec.g * vecscalar.y, vec.b * vecscalar.z);
}
inline Color operator*(float scalar, const Color &vec) { // 向量数乘（标量在前）
    return Color(vec.r * scalar, vec.g * scalar, vec.b * scalar);
}
inline Color operator/(const Color &vec, float scalar) { // 向量数除
    if (scalar == 0)
        return Color(114514, 114514, 114514); // 避免除以零
    return Color(vec.r / scalar, vec.g / scalar, vec.b / scalar);
}

/**
 * @name 动态数组
 * @warning 该类仅提供接口定义，具体实现请自行完成
 */
class DynamicArray {
private:
    byte *data;
    size_t size;

public:
    DynamicArray(size_t size) : size(size) {
        (void) size;
        data = new byte[size];
    }
    ~DynamicArray() {
        delete[] data;
    }

    byte &operator[](size_t index) {
        return data[index];
    }
    const byte &operator[](size_t index) const {
        return data[index];
    }
};

/******      常用数学公式      ******/
namespace StdMath {
    /// @brief 转速转弧度速度
    /// @param rpm 转速 (RPM)
    /// @return 弧度速度
    float RpmToRadS(float rpm);

    /// @brief 转速转米每秒速度
    /// @param rpm 转速 (RPM)
    /// @return 米每秒速度
    float RpmToMS(float diameter, float rpm);

    /// @brief 弧度速度转转速
    /// @param rad_s 弧度速度
    /// @return
    float RadSToRpm(float rad_s);

    /// @brief 限幅函数
    /// @param val 目标值
    /// @param limit 限幅值
    float fclamp(float val, float limit);

    /// @brief 非对称限幅函数
    /// @param val 目标值
    /// @param min_limit 负限幅值
    /// @param max_limit 正限幅值
    float fclamp(float val, float min_limit, float max_limit);

    /// @brief 符号函数
    /// @param val 目标值
    /// @return 目标值的符号 + / -
    int signf(float val);
} // namespace StdMath

#endif /* __STD_MATH_HPP__ */
