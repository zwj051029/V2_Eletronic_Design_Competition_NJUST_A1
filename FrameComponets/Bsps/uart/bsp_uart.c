#include "bsp_uart.h"
#include "stm32f4xx_hal.h"

/*---------------------------- 内部配置 ----------------------------*/

#define MAX_UART_INSTANCES 6 // 最大支持 6 个 UART 实例
#define UART_RX_BUF_SIZE 256 // 内部接收缓冲区大小

/*---------------------------- 内部类型 ----------------------------*/

typedef struct {
    UART_HandleTypeDef *huart;        // HAL 句柄（由注册时传入）
    BspUart_Mode rx_mode;             // 接收模式
    BspUart_Mode tx_mode;             // 发送模式
    uint8_t rx_buf[UART_RX_BUF_SIZE]; // 接收缓冲区
    uint16_t rx_buf_size;             // 用户指定的缓冲区有效大小
    BspUart_RxCallback rx_callback;   // 接收回调
    BspUart_Instance *inst;           // 指回用户实例
    bool used;                        // 是否已使用
} uart_ctrl_t;

static uart_ctrl_t ctrl_pool[MAX_UART_INSTANCES] = {0};
static bool pool_initialized = false;

/*---------------------------- 内部辅助函数 ------------------------*/

/**
 * @brief 初始化控制池（首次调用时执行）
 */
static void PoolInit(void) {
    for (int i = 0; i < MAX_UART_INSTANCES; i++) {
        ctrl_pool[i].used = false;
    }
    pool_initialized = true;
}

/**
 * @brief 分配一个空闲的控制块
 * @return 控制块指针，无空闲则返回 NULL
 */
static uart_ctrl_t *AllocCtrl(void) {
    for (int i = 0; i < MAX_UART_INSTANCES; i++) {
        if (!ctrl_pool[i].used) {
            ctrl_pool[i].used = true;
            return &ctrl_pool[i];
        }
    }
    return NULL;
}

/**
 * @brief 根据 HAL 句柄查找对应的控制块
 * @param huart HAL 句柄
 * @return 控制块指针，未找到返回 NULL
 */
static uart_ctrl_t *FindCtrlByHuart(UART_HandleTypeDef *huart) {
    for (int i = 0; i < MAX_UART_INSTANCES; i++) {
        if (ctrl_pool[i].used && ctrl_pool[i].huart == huart) {
            return &ctrl_pool[i];
        }
    }
    return NULL;
}

/**
 * @brief 启动接收
 * @param ctrl 控制块指针
 */
static void StartReceive(uart_ctrl_t *ctrl) {
    if (ctrl->rx_mode == BSP_UART_MODE_IT) {
        HAL_UART_Receive_IT(ctrl->huart, ctrl->rx_buf, ctrl->rx_buf_size);
    } else if (ctrl->rx_mode == BSP_UART_MODE_DMA) {
        HAL_UARTEx_ReceiveToIdle_DMA(ctrl->huart, ctrl->rx_buf, ctrl->rx_buf_size);
        __HAL_DMA_DISABLE_IT(ctrl->huart->hdmarx, DMA_IT_HT);
    }
}

/*---------------------------- 公开函数实现 ------------------------*/

/**
 * @brief 注册 UART 实例
 * @param inst         用户分配的实例指针
 * @param huart        指向 HAL UART 句柄的指针（如 &huart1）
 * @param rx_mode      接收工作模式
 * @param tx_mode      发送工作模式
 * @param rx_buf_size  期望接收的长度（IT模式）或缓冲区大小（DMA模式），不得大于 UART_RX_BUF_SIZE
 * @param rx_callback  接收回调函数（中断/DMA模式下有效）
 * @note 调用前必须通过 CubeMX 完成该 UART 的硬件配置（引脚、时钟、DMA/中断等），
 *       并确保 huart 指针有效。此函数会启动接收。
 */
void BspUart_Register(BspUart_Instance *inst, void *huart, BspUart_Mode rx_mode, BspUart_Mode tx_mode,
                      uint16_t rx_buf_size, BspUart_RxCallback rx_callback) {
    if (inst == NULL || huart == NULL)
        return;
    if (!pool_initialized)
        PoolInit();
    if (rx_buf_size > UART_RX_BUF_SIZE)
        return;

    UART_HandleTypeDef *h = (UART_HandleTypeDef *) huart;

    // 检查是否重复注册同一 UART
    for (int i = 0; i < MAX_UART_INSTANCES; i++) {
        if (ctrl_pool[i].used && ctrl_pool[i].huart == h)
            return;
    }

    uart_ctrl_t *ctrl = AllocCtrl();
    if (ctrl == NULL)
        return;

    ctrl->huart = h;
    ctrl->rx_mode = rx_mode;
    ctrl->tx_mode = tx_mode;
    ctrl->rx_buf_size = rx_buf_size;
    ctrl->rx_callback = rx_callback;
    ctrl->inst = inst;

    inst->priv = ctrl;

    StartReceive(ctrl);
}

/**
 * @brief 发送数据
 * @param inst 已注册的实例
 * @param data 待发送的数据缓冲区
 * @param len  数据长度（字节）
 */
void BspUart_Transmit(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    if (inst == NULL || data == NULL || len == 0)
        return;
    uart_ctrl_t *ctrl = (uart_ctrl_t *) inst->priv;
    if (ctrl == NULL || !ctrl->used)
        return;

    switch (ctrl->tx_mode) {
    case BSP_UART_MODE_NORMAL:
        HAL_UART_Transmit(ctrl->huart, data, len, HAL_MAX_DELAY);
        break;
    case BSP_UART_MODE_IT:
        HAL_UART_Transmit_IT(ctrl->huart, data, len);
        break;
    case BSP_UART_MODE_DMA:
        HAL_UART_Transmit_DMA(ctrl->huart, data, len);
        break;
    default:
        break;
    }
}

/*---------------------------- HAL 回调函数实现 --------------------*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    uart_ctrl_t *ctrl = FindCtrlByHuart(huart);
    if (ctrl && ctrl->rx_callback) {
        ctrl->rx_callback(ctrl->inst, ctrl->rx_buf, ctrl->rx_buf_size);
    }
    if (ctrl && ctrl->rx_mode == BSP_UART_MODE_IT) {
        HAL_UART_Receive_IT(ctrl->huart, ctrl->rx_buf, ctrl->rx_buf_size);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    uart_ctrl_t *ctrl = FindCtrlByHuart(huart);
    if (ctrl && ctrl->rx_callback) {
        ctrl->rx_callback(ctrl->inst, ctrl->rx_buf, Size);
    }
    if (ctrl && ctrl->rx_mode == BSP_UART_MODE_DMA) {
        HAL_UARTEx_ReceiveToIdle_DMA(ctrl->huart, ctrl->rx_buf, ctrl->rx_buf_size);
        __HAL_DMA_DISABLE_IT(ctrl->huart->hdmarx, DMA_IT_HT);
    }
}