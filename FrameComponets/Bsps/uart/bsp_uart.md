# 📡 BSP UART 抽象层使用手册

## 📖 1. 概述

`bsp_uart` 是机器人跨芯片框架中的**板级支持包（BSP）UART 模块**，为上层应用提供了一套**与芯片无关**的通用串口通信接口。
它的核心思想是：**你只管“我要用哪个串口发什么数据”，而不需要关心底层是 STM32 还是 ESP32**。换主控时，只需替换对应的 `bsp_uart.c`，上层代码**纹丝不动**，就像给机器人换个遥控器电池一样简单 🔋➡️🤖。

本模块专注于 **UART 的收发与回调管理**，硬件初始化（引脚、时钟、DMA、中断）依然交给 CubeMX，我们只做逻辑封装，绝不越俎代庖。

---

## ✨ 2. 设计特点

- 🧩 **平台无关**：头文件不暴露任何芯片寄存器或 HAL 库细节，使用 `void*` 句柄隐藏实际硬件。
- 📝 **灵活注册**：支持用户自定义接收缓冲区大小、收发模式，并将 HAL 句柄直接传给实例，编译不依赖未使能的外设。
- 🔔 **回调透传**：通过 `user_data` 可以将任意上下文传递给中断回调，实现同一个回调函数服务多个串口实例，告别全局变量满天飞 ✨。
- ⚡ **高效轻量**：内部使用静态资源池管理控制块，避免动态内存分配，确保嵌入式实时性。
- 🛑 **职责分明**：硬件初始化全部由 CubeMX 负责，BSP 层只负责收发管理和回调分发，层次清晰如刀切豆腐。

---

## 📁 3. 文件说明

| 文件 | 位置 | 说明 |
|------|------|------|
| `bsp_uart.h` | `bsp/common/` | 公共接口头文件（平台无关），上层只需包含此文件 🎮 |
| `bsp_uart.c` | `bsp/<platform>/` | 平台相关实现（如 `stm32f407/bsp_uart.c`），每片芯片的“翻译官” 🌍 |

---

## 📚 4. 接口函数

### 4.1 数据类型

```c
// UART 工作模式
typedef enum {
    BSP_UART_MODE_NORMAL = 0, // 阻塞模式 🐢
    BSP_UART_MODE_IT     = 1, // 中断模式 🐇
    BSP_UART_MODE_DMA    = 2  // DMA 模式 🚀
} BspUart_Mode;
```

```c
// BSP UART 实例结构体（用户持有，轻量透明）
typedef struct {
    void   *priv;      // 私有数据指针（BSP 层使用，用户勿动）🔒
    void   *user_data; // 用户自定义数据（可在回调中获取）🎒
} BspUart_Instance;
```

```c
// 接收回调函数原型
// @param inst  产生事件的 UART 实例
// @param data  接收到的数据缓冲区（不是永久保存，回调返回后可能被覆盖）
// @param len   本次接收到的数据长度
typedef void (*BspUart_RxCallback)(BspUart_Instance *inst, uint8_t *data, uint16_t len);
```

### 4.2 注册实例 📋

```c
void BspUart_Register(BspUart_Instance *inst, void *huart,
                      BspUart_Mode rx_mode, BspUart_Mode tx_mode,
                      uint16_t rx_buf_size, BspUart_RxCallback rx_callback);
```

- **功能**：绑定一个 UART 实例与硬件句柄，指定收发模式、缓冲区大小和接收回调。
- **参数**：
  `inst` - 用户分配的实例指针；
  `huart` - 指向 HAL 句柄的指针（如 `&huart1`），类型为 `void*` 以隐藏平台细节；
  `rx_mode` - 接收模式（阻塞/中断/DMA）；
  `tx_mode` - 发送模式（阻塞/中断/DMA）；
  `rx_buf_size` - 期望接收的字节数（中断模式）或缓冲区大小（DMA 模式），最大 256 字节；
  `rx_callback` - 接收完成回调（中断/DMA 有效，阻塞模式可传 NULL）。
- **注意**：调用前必须通过 CubeMX 完成该 UART 的完整初始化，并确保 `huart` 指针有效。重复注册同一个 `huart` 将被忽略。

### 4.3 发送数据 ✉️

```c
void BspUart_Transmit(BspUart_Instance *inst, uint8_t *data, uint16_t len);
```

- **功能**：通过指定实例发送数据。
- **发送行为取决于注册时的 `tx_mode`**：
  - `NORMAL`：阻塞发送，直到全部发送完成（可能卡住调用线程）
  - `IT`：中断发送，立马返回，后台搬数据
  - `DMA`：DMA 发送，同上，解放 CPU
- **示例**：`BspUart_Transmit(&dbus_uart, tx_buf, 18);`

---

## 🛠️ 5. 使用流程

1. **CubeMX 硬件初始化** 🎛️
   - 选择需要的 UART（如 USART1），配置引脚、波特率等。
   - 根据需要选择 **接收模式**：
        - 若用中断接收 → 在 NVIC 中使能 UART 全局中断
        - 若用 DMA 接收 → 在 DMA Settings 中添加 `RX` 通道，模式选 `Circular`（或配合空闲中断使用 `Normal`）
   - 发送模式类似，但发送 DMA 一般选择 `Normal`。
   - **重要**：如果使用 DMA 空闲中断接收，CubeMX 的 DMA 模式选择 `Normal` 即可，我们会在代码中手动开启空闲中断。

2. **声明实例变量** 🧱
   ```c
   BspUart_Instance dbus_uart;
   ```

3. **包含必要的头文件** 📂
   ```c
   #include "bsp_uart.h"          // BSP 接口
   #include "usart.h"             // CubeMX 生成的 huart 声明（仅应用层需要）
   ```

4. **注册实例并启动接收** 🔗
   ```c
   void MyRxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
       // 处理接收数据...
   }

   BspUart_Register(&dbus_uart, &huart1,
                    BSP_UART_MODE_DMA, BSP_UART_MODE_DMA,
                    128, MyRxCallback);
   ```
   注册后，接收就会自动开始（中断/DMA 模式），无需手动再调用任何接收函数 ✅。

5. **发送数据** 📤
   ```c
   uint8_t hello[] = "Hello from UART!";
   BspUart_Transmit(&dbus_uart, hello, sizeof(hello));
   ```

6. **处理接收数据** 🎒
   在回调 `MyRxCallback` 中处理，或设置标志位，由主循环读取。
   **注意**：回调是在**中断上下文**中执行，应快速完成，不调用耗时函数。

---

## 🧪 6. 完整示例：DMA + 空闲中断收发

```c
#include "bsp_uart.h"
#include "usart.h"   // 提供 huart1

BspUart_Instance comm_uart;

void CommRxHandler(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    // 在此解析一帧数据（DMA 空闲中断会收到不定长的帧）
    // 注意：data 是内部缓冲区的当前帧，会下次覆盖，需要的话请立即拷贝
    if (len > 0) {
        // 例如：设置标志位，通知主循环处理
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();  // CubeMX 生成的初始化

    // 注册：接收用 DMA + 空闲中断，发送用 DMA，缓冲区 128 字节
    BspUart_Register(&comm_uart, &huart1,
                     BSP_UART_MODE_DMA, BSP_UART_MODE_DMA,
                     128, CommRxHandler);

    // 发送欢迎消息
    uint8_t welcome[] = "UART Ready!\r\n";
    BspUart_Transmit(&comm_uart, welcome, sizeof(welcome));

    while (1) {
        // 主循环可以轮询标志位，处理解析后的数据
    }
}
```

> 💡 **DMA 空闲中断** 是接收不定长数据的神器，BSP 层已经帮你处理好了，你只需要在回调里拿到长度和指针。

---

## 🌍 7. 移植到其他芯片

当你把上层 Mods/Apps 迁移到非 STM32 平台（如 ESP32、K210），只需完成：

- 🆕 为新平台编写一个 `bsp_uart.c`，提供完全相同的 `bsp_uart.h` 接口。
- 🔄 实现内部控制块管理（结构体可能不同，但功能等价）。
- 📞 实现该平台 SDK 下的接收回调（如 ESP-IDF 的 `uart_driver_install` 和 `uart_event_callback`），最终仍旧调用用户注册的 `rx_callback`。
- 上层代码**一行不改**，重新编译即可奔跑 🏃💨。

例如 ESP32 平台，你可能会用 `uart_num` 代替 `UART_HandleTypeDef*`，但 BSP 层对外接口完全不变。

---

## ⚠️ 8. 注意事项

| 注意事项 | 说明 |
|----------|------|
| 🔌 **硬件初始化必须提前** | 所有 GPIO、时钟、DMA、中断的初始化由 CubeMX 完成，BSP 层不碰这些寄存器。若未初始化，直接注册会导致未定义行为 |
| 🔁 **重复注册保护** | 不允许同一个 `huart` 被多次注册，重复调用 `BspUart_Register` 会被忽略 |
| 📦 **缓冲区大小限制** | `rx_buf_size` 不能超过 `UART_RX_BUF_SIZE`（默认 256），否则注册失败 |
| ⚡ **中断回调内禁阻塞** | 接收回调在中断上下文中执行，严禁调用 `HAL_Delay` 或耗时操作，建议只置标志位 |
| 🕸️ **DMA 发送未完成时覆盖** | DMA 发送是异步的，如果连续发送需要注意缓冲区生命周期。不建议在发送完成前修改 `data` 内容 |
| 🗃️ **实例生命周期** | `BspUart_Instance` 必须是全局或静态变量，不能在函数内部分配后注册（中断回调需要一直有效） |
| 🧼 **接收数据时效性** | 回调中的 `data` 指针指向内部循环使用的缓冲区，下次中断会覆盖，需要持久化请立即复制 |
| 🧩 **阻塞接收不推荐** | 阻塞模式会让 CPU 死等，一般不推荐在机器人实时控制中使用，除非你很清楚自己在干什么 🐌 |

---

> 🏁 **总结**：`bsp_uart` 是你跨芯片框架的通信基石，无论收发 DBUS 遥控数据、与传感器交互、还是调试日志，它都能优雅地服务。记住“注册一次，到处使用”，换芯片？那不过是换个 `bsp_uart.c` 重新编译的事儿～ 🎉
