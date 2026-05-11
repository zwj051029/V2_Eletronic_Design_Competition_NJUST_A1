#pragma once

#include "arm_math.h"
#include "std_math.hpp"
#include "stm32f4xx_hal.h"
#include <initializer_list>


// 确保定义 CMSIS DSP 宏，如果项目中未定义，需要在此处或者工程设置中定义
// #define ARM_MATH_CM4

template <uint8_t rows, uint8_t cols>
class Matrix {
public:
    Matrix() {
        // 显式初始化为0
        memset(data, 0, sizeof(data));
    };

    /**
     * @brief 可变参数构造函数，解决部分编译器无法自动推导 initializer_list 的问题
     * @details 允许 Matrix<2,2> m = {1.0f, 2.0f, 3.0f, 4.0f}; 形式
     *          Vscode IntelliSense 在同时存在 initializer_list 构造函数时可能会报错，
     *          因此优先使用此变参模板来处理列表初始化。
     */
    template <typename... Args>
    Matrix(Args... args) {
        // 静态断言：参数数量不能超过矩阵容量
        static_assert(sizeof...(Args) <= rows * cols, "Matrix initialization: Too many arguments");
        // 静态断言：确保不是默认构造（空参数由默认构造函数处理）
        static_assert(sizeof...(Args) > 0, "Use default constructor for empty initialization");

        // 先清零
        memset(data, 0, sizeof(data));

        // 将参数解包到临时数组，并逐个转换为 float
        // 使用逗号表达式展开参数包
        float temp[] = {static_cast<float>(args)...};

        // 拷贝数据
        size_t count = sizeof...(Args);
        // data 是二维数组，但在内存中连续，可以用 flat 方式拷贝
        memcpy(data, temp, count * sizeof(float));
    }

    /**
     * @brief 列表赋值重载，允许 m = {1, 2, 3, 4};
     */
    Matrix<rows, cols> &operator=(std::initializer_list<float> list) {
        // 安全检查
        size_t count = list.size();
        if (count > rows * cols) {
            count = rows * cols;
        }

        if (count > 0) {
            memcpy(data, list.begin(), count * sizeof(float));
        }
        // 如果列表数据比矩阵小，剩余部分保持原样或可选清零，这里选择只覆盖前面的
        return *this;
    }

    /**
     * @brief 同尺寸矩阵赋值重载 A = B;
     * @note  显式利用 memcpy 实现高速拷贝，确保在所有优化等级下都使用块拷贝指令
     */
    Matrix<rows, cols> &operator=(const Matrix<rows, cols> &other) {
        // 防止自赋值 (A = A)
        if (this != &other) {
            // 安全且高速的内存块拷贝，sizeof(data) 在编译期确定
            // 相比逐元素拷贝，memcpy 在 ARM 上通常会被优化为 LDM/STM 或 NEON 指令
            memcpy(data, other.data, sizeof(data));
        }
        return *this;
    }

    // 使用一维连续内存布局，保持与 float data[rows][cols] 的内存兼容性
    // 二维数组在内存中也是连续的，可以直接强转用于 arm_math
    float data[rows][cols];

    // 数据访问重载
    float &operator()(size_t r, size_t c) {
        return data[r][c];
    }
    const float &operator()(size_t r, size_t c) const {
        return data[r][c];
    }

    // 获取 arm_matrix_instance_f32 实例的辅助函数
    // 注意：arm_math 的 pData 指针不是 const 的，所以需要 const_cast
    void get_arm_matrix(arm_matrix_instance_f32 *pMat) const {
        arm_mat_init_f32(pMat, rows, cols, (float32_t *) data);
    }

    // 矩阵加法
    Matrix<rows, cols> operator+(const Matrix<rows, cols> &other) const {
        Matrix<rows, cols> result;

        arm_matrix_instance_f32 matA, matB, matDst;
        this->get_arm_matrix(&matA);
        other.get_arm_matrix(&matB);
        result.get_arm_matrix(&matDst);

        arm_mat_add_f32(&matA, &matB, &matDst);

        return result;
    }

    // 矩阵减法
    Matrix<rows, cols> operator-(const Matrix<rows, cols> &other) const {
        Matrix<rows, cols> result;

        arm_matrix_instance_f32 matA, matB, matDst;
        this->get_arm_matrix(&matA);
        other.get_arm_matrix(&matB);
        result.get_arm_matrix(&matDst);

        arm_mat_sub_f32(&matA, &matB, &matDst);

        return result;
    }

    // 矩阵乘法
    template <uint8_t new_cols>
    Matrix<rows, new_cols> operator*(const Matrix<cols, new_cols> &other) const {
        Matrix<rows, new_cols> result;

        arm_matrix_instance_f32 matA, matB, matDst;
        this->get_arm_matrix(&matA);
        // 这里手动初始化 other 的 arm_matrix，因为维度不同，不能直接调用 other.get_arm_matrix
        arm_mat_init_f32(&matB, cols, new_cols, (float32_t *) other.data);
        result.get_arm_matrix(&matDst); // 结果矩阵维度 rows x new_cols

        arm_mat_mult_f32(&matA, &matB, &matDst);

        return result;
    }

    // 数乘
    Matrix<rows, cols> operator*(float scalar) const {
        Matrix<rows, cols> result;

        arm_matrix_instance_f32 matA, matDst;
        this->get_arm_matrix(&matA);
        result.get_arm_matrix(&matDst);

        arm_mat_scale_f32(&matA, scalar, &matDst);

        return result;
    }

    // ---------------------------------------------------------
    // 高级功能
    // ---------------------------------------------------------

    // 1. 转置 (Transpose)
    Matrix<cols, rows> transpose() const {
        Matrix<cols, rows> result;

        arm_matrix_instance_f32 matSrc, matDst;
        this->get_arm_matrix(&matSrc);
        // 目标矩阵维度反转
        arm_mat_init_f32(&matDst, cols, rows, (float32_t *) result.data);

        arm_mat_trans_f32(&matSrc, &matDst);

        return result;
    }

    // 2. 生成单位矩阵 (静态方法)
    static Matrix<rows, cols> identity() {
        static_assert(rows == cols, "Identity matrix must be square");
        Matrix<rows, cols> res;
        for (uint8_t i = 0; i < rows; ++i)
            res(i, i) = 1.0f;
        return res;
    }

    // 3. 求逆 (Inverse)
    // 返回值：是否成功（true=成功，false=奇异矩阵不可逆）
    bool inverse(Matrix<rows, cols> &out) const {
        static_assert(rows == cols, "Inverse matrix must be square");

        arm_matrix_instance_f32 matSrc, matDst;
        this->get_arm_matrix(&matSrc);
        out.get_arm_matrix(&matDst);

        arm_status status = arm_mat_inverse_f32(&matSrc, &matDst);

        return (status == ARM_MATH_SUCCESS);
    }
};