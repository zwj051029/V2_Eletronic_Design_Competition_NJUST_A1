#ifndef BSP_UART_H
#define BSP_UART_H

#include "usart.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------- 类型定义 ----------------------------*/

// UART 工作模式
typedef enum {
    BSP_UART_MODE_NORMAL = 0, // 阻塞模式
    BSP_UART_MODE_IT = 1,     // 中断模式
    BSP_UART_MODE_DMA = 2     // DMA 模式
} BspUart_Mode;

// BSP UART 实例结构体（用户持有）
typedef struct {
    void *priv;      // 私有数据指针（BSP 层使用，用户勿动）
    void *user_data; // 用户自定义数据（可在回调中获取）
} BspUart_Instance;

// 接收回调函数原型
typedef void (*BspUart_RxCallback)(BspUart_Instance *inst, uint8_t *data, uint16_t len);

/*---------------------------- 接口函数 ----------------------------*/

/** @brief 注册一个 UART 实例并指定收发模式及回调 */
void BspUart_Register(BspUart_Instance *inst, void *huart, BspUart_Mode rx_mode, BspUart_Mode tx_mode,
                      uint16_t rx_buf_size, BspUart_RxCallback rx_callback);

/** @brief 通过 UART 实例发送数据 */
void BspUart_Transmit(BspUart_Instance *inst, uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_UART_H */