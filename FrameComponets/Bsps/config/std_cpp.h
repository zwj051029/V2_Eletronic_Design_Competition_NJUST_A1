#ifndef __STD_CPP__
#define __STD_CPP__

/**
 * @file std_cpp.h
 * @author https://github.com/zwj051029
 * @date 2026-1-17
 * @brief 本库为C和C++代码的接口文件
 * @note 该文件用于在C代码中包含C++代码时，防止名称修饰问题，所有要在C中调用的函数都要放在此文件中
 */
#ifdef __cplusplus
extern "C" {
#endif

/******      RTOS任务相关的函数      ******/

/// @brief 机器人高频控制任务（1000Hz）
void ControlCpp();

/// @brief 机器人状态更新任务（250Hz）
void StateCoreCpp();

/// @brief 机器人应用管理任务（200Hz）
void ApplicationCpp();

/// @brief 机器人系统主任务（200Hz）
void RobotSystemCpp();

/******      主初始化函数      ******/

/// @brief 机器人主初始化函数
void MainInitCpp();

#ifdef __cplusplus
}
#endif

#endif