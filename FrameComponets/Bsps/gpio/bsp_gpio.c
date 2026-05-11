#include "bsp_gpio.h"
#include "stm32f4xx_hal.h"

/*---------------------------- 内部辅助函数 ------------------------*/

/**
 * @brief 将端口字符转换为 GPIO 寄存器基地址
 * @param port 端口字符（'A' ~ 'I'）
 * @return 对应 GPIO_TypeDef 指针，无效返回 NULL
 */
static GPIO_TypeDef *PortToReg(char port) {
    switch (port) {
    case 'A':
        return GPIOA;
    case 'B':
        return GPIOB;
    case 'C':
        return GPIOC;
    case 'D':
        return GPIOD;
    case 'E':
        return GPIOE;
    case 'F':
        return GPIOF;
    case 'G':
        return GPIOG;
    case 'H':
        return GPIOH;
    case 'I':
        return GPIOI;
    default:
        return NULL;
    }
}

/**
 * @brief 引脚编号转位掩码
 * @param pin 引脚编号 0~15
 * @return 16 位掩码（失败返回 0）
 */
static uint16_t PinToMask(uint8_t pin) {
    if (pin > 15)
        return 0;
    return (uint16_t) (1 << pin);
}

/**
 * @brief 引脚掩码转引脚编号（假定掩码中只有一位为 1）
 * @param mask 引脚掩码
 * @return 引脚编号 0~15
 */
static uint8_t MaskToPinIdx(uint16_t mask) {
    uint8_t idx = 0;
    uint16_t temp = mask;

    while (temp > 1) {
        temp >>= 1;
        idx++;
    }
    return idx;
}

/*---------------------------- 中断回调管理 ------------------------*/

// 存储每个 EXTI 线的用户回调（引脚 0 ~ 15）
static struct {
    BspGpio_IrqCallback callback; // 用户回调函数指针
    void *user_data;              // 用户上下文指针
} exti_callbacks[16] = {0};

/*---------------------------- 公开函数实现 ------------------------*/

/**
 * @brief 注册 GPIO 实例
 * @param inst 指向实例结构体的指针（由用户分配）
 * @param port 端口字符（如 'A' 对应 GPIOA）
 * @param pin  引脚编号 0~15
 * @note 调用前需通过 CubeMX 完成该引脚的硬件初始化
 */
void BspGpio_Register(BspGpio_Instance *inst, char port, uint8_t pin) {
    if (inst == NULL || pin > 15)
        return;

    inst->port = port;
    inst->pin = pin;
}

/**
 * @brief 写引脚电平
 * @param inst  已注册实例
 * @param state 目标电平（BSP_GPIO_HIGH / BSP_GPIO_LOW）
 * @note 对输入模式无效，但不会引起硬件错误
 */
void BspGpio_Write(BspGpio_Instance *inst, BspGpio_PinState state) {
    if (inst == NULL)
        return;

    GPIO_TypeDef *port = PortToReg(inst->port);
    uint16_t mask = PinToMask(inst->pin);

    if (port && mask) {
        HAL_GPIO_WritePin(port, mask, (state == BSP_GPIO_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

/**
 * @brief 读引脚电平
 * @param inst 已注册实例
 * @return 当前引脚电平（输入模式有效，其他模式返回低电平）
 */
BspGpio_PinState BspGpio_Read(BspGpio_Instance *inst) {
    if (inst == NULL)
        return BSP_GPIO_LOW;

    GPIO_TypeDef *port = PortToReg(inst->port);
    uint16_t mask = PinToMask(inst->pin);

    if (port && mask) {
        GPIO_PinState st = HAL_GPIO_ReadPin(port, mask);
        return (st == GPIO_PIN_SET) ? BSP_GPIO_HIGH : BSP_GPIO_LOW;
    }
    return BSP_GPIO_LOW;
}

/**
 * @brief 翻转引脚输出电平
 * @param inst 已注册实例
 */
void BspGpio_Toggle(BspGpio_Instance *inst) {
    if (inst == NULL)
        return;

    GPIO_TypeDef *port = PortToReg(inst->port);
    uint16_t mask = PinToMask(inst->pin);

    if (port && mask) {
        HAL_GPIO_TogglePin(port, mask);
    }
}

/**
 * @brief 注册外部中断回调（仅绑定回调，不配置硬件）
 * @param inst      已注册的 GPIO 实例指针
 * @param trigger   中断触发方式（在 CubeMX 中已配置，此参数暂无实质作用）
 * @param callback  用户中断服务回调
 * @param user_data 传递给回调的参数
 * @note 硬件必须已在 CubeMX 中完成：GPIO 输入模式、EXTI 使能、NVIC 优先级。
 *       本函数仅记录回调，当中断发生时由 HAL_GPIO_EXTI_Callback 调用。
 */
void BspGpio_AttachIrq(BspGpio_Instance *inst, BspGpio_Trigger trigger, BspGpio_IrqCallback callback, void *user_data) {
    (void) trigger; // 触发沿已在 CubeMX 配置，此处保留参数仅为接口统一
    if (inst == NULL || inst->pin > 15 || callback == NULL)
        return;

    uint8_t pin_idx = inst->pin;
    exti_callbacks[pin_idx].callback = callback;
    exti_callbacks[pin_idx].user_data = user_data;
}

/**
 * @brief HAL 库通用 EXTI 回调（重写弱定义）
 * @param GPIO_Pin 产生中断的引脚位掩码
 * @note 由 HAL_GPIO_EXTI_IRQHandler 自动调用，负责分发到用户注册的回调
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint8_t pin_idx = MaskToPinIdx(GPIO_Pin);

    if (pin_idx < 16 && exti_callbacks[pin_idx].callback) {
        exti_callbacks[pin_idx].callback(exti_callbacks[pin_idx].user_data);
    }
}