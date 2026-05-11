#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------- 类型定义 ----------------------------*/

/// @brief GPIO 引脚电平枚举
typedef enum {
    BSP_GPIO_LOW = 0, // 低电平
    BSP_GPIO_HIGH = 1 // 高电平
} BspGpio_PinState;

/// @brief 外部中断触发方式枚举
typedef enum {
    BSP_GPIO_TRIGGER_RISING = 0,  // 上升沿触发
    BSP_GPIO_TRIGGER_FALLING = 1, // 下降沿触发
    BSP_GPIO_TRIGGER_BOTH = 2     // 双边沿触发
} BspGpio_Trigger;

/// @brief GPIO 实例结构体（用户持有，用于绑定逻辑引脚）
typedef struct {
    char port;   // 端口字符（如 'A', 'B', 'C' ... 平台相关映射）
    uint8_t pin; // 引脚编号 0~15
} BspGpio_Instance;

/// @brief 外部中断回调函数原型
/// @param user_data 用户注册时传入的上下文指针
typedef void (*BspGpio_IrqCallback)(void *user_data);

/*---------------------------- 接口函数 ----------------------------*/

/** @brief 注册一个 GPIO 实例，绑定端口与引脚 */
void BspGpio_Register(BspGpio_Instance *inst, char port, uint8_t pin);

/** @brief 设置指定实例的输出电平 */
void BspGpio_Write(BspGpio_Instance *inst, BspGpio_PinState state);

/** @brief 读取指定实例的输入电平 */
BspGpio_PinState BspGpio_Read(BspGpio_Instance *inst);

/** @brief 翻转指定实例的输出电平 */
void BspGpio_Toggle(BspGpio_Instance *inst);

/** @brief 注册一个外部中断回调函数 */
void BspGpio_AttachIrq(BspGpio_Instance *inst, BspGpio_Trigger trigger, BspGpio_IrqCallback callback, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* BSP_GPIO_H */