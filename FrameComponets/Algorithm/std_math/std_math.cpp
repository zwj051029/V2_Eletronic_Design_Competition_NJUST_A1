#include "std_math.hpp"
#include "arm_math.h"
#include "string.h"

/******      二维向量      ******/
/**
 * @brief 归一化二维向量
 */
Vec2 Vec2::Norm() {
    Vec2 v = *this;
    // 计算向量的模
    float magnitude = sqrt(v.x * v.x + v.y * v.y);
    // 检查模是否为零，如果为零则返回零向量
    if (magnitude == 0) {
        Vec2 zero_vector = {0, 0};
        return zero_vector;
    }
    // 归一化向量
    Vec2 normed_vec;
    normed_vec.x = v.x / magnitude;
    normed_vec.y = v.y / magnitude;

    return normed_vec;
}

/**
 * @brief 计算二维向量的模
 */
float Vec2::Length() {
    return sqrt(this->x * this->x + this->y * this->y);
}

/**
 * @brief 计算二维向量的角度
 * @note 角度单位为弧度，范围为[-π, π]
 */
float Vec2::Angle() {
    // 使用atan2函数计算向量的角度，atan2返回值范围是[-π, π]
    return atan2(this->x, this->y) * 180.0f / 3.1415926f;
}

/**
 * @brief 旋转二维向量
 * @param angRad 旋转角度，单位为弧度
 * @return 旋转后的二维向量
 */
Vec2 Vec2::Rotate(float angRad) {
    Vec2 result;
    float cosTheta = cosf(angRad);
    float sinTheta = sinf(angRad);

    // 旋转公式：x' = x*cosθ - y*sinθ, y' = x*sinθ + y*cosθ
    result.x = this->x * cosTheta - this->y * sinTheta;
    result.y = this->x * sinTheta + this->y * cosTheta;

    return result;
}

/**
 * @brief 将二维向量转换为三维向量
 * @note 转换后z分量为0
 */
Vec3 Vec2::ToVec3() {
    Vec3 v(this->x, this->y, 0);
    return v;
}

/**
 * @brief 将Vec2转换入buffer
 * @note buffer必须至少有8字节空间
 */
void Vec2::ToBytes(uint8_t *buffer) {
    memcpy(buffer, &(this->x), sizeof(float));
    memcpy(buffer + sizeof(float), &(this->y), sizeof(float));
}

/**
 * @brief 从buffer恢复Vec2
 * @note buffer必须至少有8字节空间
 */
void Vec2::FromBytes(const uint8_t *buffer) {
    memcpy(&(this->x), buffer, sizeof(float));
    memcpy(&(this->y), buffer + sizeof(float), sizeof(float));
}

/******      三维向量      ******/
/**
 * @brief 归一化三维向量
 */
Vec3 Vec3::Norm() {
    Vec3 v = *this;
    // 计算向量的模
    float magnitude = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    // 检查模是否为零，如果为零则返回零向量
    if (magnitude == 0) {
        Vec3 zero_vector = {0, 0, 0};
        return zero_vector;
    }

    // 归一化向量
    Vec3 normalized_vector;
    normalized_vector.x = v.x / magnitude;
    normalized_vector.y = v.y / magnitude;
    normalized_vector.z = v.z / magnitude;
    return normalized_vector;
}

/**
 * @brief 计算三维向量的模
 */
float Vec3::Length() {
    // 计算向量的模
    return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}

/**
 * @brief 将三维向量转换为二维向量
 * @note 转换后z分量被丢弃
 */
Vec2 Vec3::ToVec2() {
    Vec2 v(this->x, this->y);
    return v;
}

/**
 * @brief 将Vec3转换入buffer
 * @note buffer必须至少有12字节空间
 */
void Vec3::ToBytes(uint8_t *buffer) {
    memcpy(buffer, &(this->x), sizeof(float));
    memcpy(buffer + sizeof(float), &(this->y), sizeof(float));
    memcpy(buffer + 2 * sizeof(float), &(this->z), sizeof(float));
}

/**
 * @brief 从buffer恢复Vec3
 * @note buffer必须至少有12字节空间
 */
void Vec3::FromBytes(const uint8_t *buffer) {
    memcpy(&(this->x), buffer, sizeof(float));
    memcpy(&(this->y), buffer + sizeof(float), sizeof(float));
    memcpy(&(this->z), buffer + 2 * sizeof(float), sizeof(float));
}

/******      常用数学公式      ******/
/**
 * @brief 转速转弧度速度
 * @param rpm 转速 (RPM)
 * @return 弧度速度
 */
float StdMath::RpmToRadS(float rpm) {
    return rpm * (2.0f * 3.1415926f) / 60.0f;
}

float StdMath::RpmToMS(float diameter, float rpm) {
    return (rpm * diameter * PI) / 6000.0f;
}

/**
 * @brief 转弧度速度转转速
 * @param rad_s 弧度速度
 * @return 转速 (RPM)
 */
float StdMath::RadSToRpm(float rad_s) {
    return rad_s * 60.0f / (2.0f * 3.1415926f);
}

/**
 * @brief 限幅函数
 * @param val 目标值
 * @param limit 限幅值
 */
float StdMath::fclamp(float val, float limit) {
    if (limit <= 0.0f)
        return val; // 0代表不限制
    if (val > limit)
        return limit;
    if (val < -limit)
        return -limit;
    return val;
}

/**
 * @brief 非对称限幅函数
 * @param val 目标值
 * @param min_limit 负限幅值
 * @param max_limit 正限幅值
 * @return 限制在[min_limit, max_limit]范围内的值
 * @note 如果min_limit > max_limit，会自动交换两者
 */
float StdMath::fclamp(float val, float min_limit, float max_limit) {
    // 确保min_limit <= max_limit
    if (min_limit > max_limit) {
        // 交换上下限
        float temp = min_limit;
        min_limit = max_limit;
        max_limit = temp;
    }

    if (val < min_limit)
        return min_limit;
    if (val > max_limit)
        return max_limit;
    return val;
}

/**
 * @brief 符号函数
 * @param val 目标值
 * @return 目标值的符号 + / -
 */
int StdMath::signf(float val) {
    if (val > 0)
        return 1;
    else if (val < 0)
        return -1;
    else
        return 0;
}
