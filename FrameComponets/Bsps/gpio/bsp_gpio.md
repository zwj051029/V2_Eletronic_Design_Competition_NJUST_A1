# 🚀 BSP GPIO 抽象层使用手册

## 📖 1. 概述

`bsp_gpio` 是机器人跨芯片框架中的**板级支持包（BSP）GPIO 模块**，为上层应用提供了一套**与芯片无关**的通用 GPIO 操作接口。
它的核心思想是：**你只管“我要操作哪个引脚”，而不用管底层是 STM32、ESP32 还是其他芯片**。换主控时，只需替换对应的 `bsp_gpio.c` 实现，上层代码纹丝不动，就像换双鞋一样轻松 👟➡️👞。

本模块只提供操作和回调管理，**硬件初始化全部交给 CubeMX**，真正做到分层清晰、职责分明。

---

## ✨ 2. 设计特点

- 🧩 **平台无关**：头文件不包含任何芯片相关的寄存器或库，使用字符端口标识（如 `'A'` 代表 GPIOA），读起来就像看标签一样直观。
- 📝 **注册机制**：通过 `BspGpio_Register()` 将逻辑实例与物理引脚绑定，一行代码完成“认领”。
- 🔔 **中断回调透传**：支持注册用户回调函数，并可通过 `user_data` 传入自定义上下文，让同一个回调服务多个实例，告别全局变量满天飞。
- ⚡ **轻量高效**：全部使用静态存储，不碰 `malloc`，完全满足嵌入式对实时性和确定性的苛求。
- 🛑 **职责分离**：硬件初始化（模式、时钟、中断线等）归 CubeMX 管，BSP 层只做操作封装，绝不越权操作寄存器——这界限划得比楚河汉界还清楚！

---

## 📁 3. 文件说明

| 文件 | 位置 | 说明 |
|------|------|------|
| `bsp_gpio.h` | `bsp/common/` | 公共接口头文件（平台无关），上层只需包含这个文件，它就是我们的“万能遥控器” 🎮 |
| `bsp_gpio.c` | `bsp/<platform>/` | 平台相关实现（如 `stm32f407/bsp_gpio.c`），每片芯片的“翻译官” 🌍 |

---

## 📚 4. 接口函数

### 4.1 数据类型

```c
typedef enum {
    BSP_GPIO_LOW  = 0,   // 低电平 🌑
    BSP_GPIO_HIGH = 1    // 高电平 🌕
} BspGpio_PinState;
```

```c
typedef enum {
    BSP_GPIO_TRIGGER_RISING  = 0,   // 上升沿触发 📈
    BSP_GPIO_TRIGGER_FALLING = 1,   // 下降沿触发 📉
    BSP_GPIO_TRIGGER_BOTH    = 2    // 双边沿触发 ⚡
} BspGpio_Trigger;
```

```c
typedef struct {
    char port;    // 端口字符，如 'A' 表示 GPIOA 🏷️
    uint8_t pin;  // 引脚编号 0~15 📌
} BspGpio_Instance;
```

```c
typedef void (*BspGpio_IrqCallback)(void *user_data);  // 中断回调函数指针 📞
```

### 4.2 注册实例 📋

```c
void BspGpio_Register(BspGpio_Instance *inst, char port, uint8_t pin);
```

- **功能**：将一个用户自定义的实例结构体与实际的端口、引脚绑定。
- **参数**：
  `inst` - 指向用户分配的 `BspGpio_Instance` 结构体；
  `port` - 端口字符（`'A'` ~ `'I'`）；
  `pin` - 引脚编号（0~15）。
- **注意**：调用前必须通过 CubeMX 完成该引脚的硬件初始化（模式、速度、上下拉等）。**CubeMX 是总导演，我们只是场记** 🎬。

### 4.3 写电平 ✏️

```c
void BspGpio_Write(BspGpio_Instance *inst, BspGpio_PinState state);
```

- **功能**：设置引脚输出高或低电平。
- **示例**：`BspGpio_Write(&led, BSP_GPIO_HIGH);` —— 点亮 LED（要看极性哦） 💡

### 4.4 读电平 📖

```c
BspGpio_PinState BspGpio_Read(BspGpio_Instance *inst);
```

- **功能**：读取引脚当前输入电平，返回 `BSP_GPIO_HIGH` 或 `BSP_GPIO_LOW`。

### 4.5 翻转电平 🔁

```c
void BspGpio_Toggle(BspGpio_Instance *inst);
```

- **功能**：将当前输出电平翻转（高→低，低→高），LED 闪烁的必备利器 ✨。

### 4.6 注册外部中断回调 🔔

```c
void BspGpio_AttachIrq(BspGpio_Instance *inst, BspGpio_Trigger trigger,
                       BspGpio_IrqCallback callback, void *user_data);
```

- **功能**：将一个用户回调函数绑定到指定 GPIO 的外部中断上。
- **参数**：
  `inst` - 已注册的实例（引脚必须在 CubeMX 中配置为输入且使能 EXTI 中断）；
  `trigger` - 触发方式（本参数目前仅作接口统一，实际触发沿已在 CubeMX 中配置）；
  `callback` - 中断服务函数指针（在中断上下文中执行，**务必简短快速** ⏱️）；
  `user_data` - 传递给回调的用户自定义数据（可以是任意指针，用于区分实例）。
- **注意**：该函数**不配置硬件**，只记录回调。务必确保 CubeMX 中已开启对应的 EXTI 中断线并设置好 NVIC 优先级，否则按键按烂也没反应 🤯。

---

## 🛠️ 5. 使用流程

1. **CubeMX 初始化硬件** 🎛️
   - 设置目标引脚为所需的输出/输入/EXTI 模式，配置好上下拉、速度等。
   - 若需外部中断，选择 `GPIO_EXTIx` 模式，**务必在 NVIC 中使能对应中断线** ⚠️。
   - 生成代码，确认 `stm32f4xx_it.c` 中有对应的中断服务函数（如 `EXTI15_10_IRQHandler`）并调用了 `HAL_GPIO_EXTI_IRQHandler`。
   > **💡 小贴士**：如果用了 PA15/PB3/PB4 等 JTAG 引脚，记得在 SYS 里把 Debug 改成 `Serial Wire`，否则它们会赖在调试接口不走 😅。

2. **声明实例变量** 🧱
   ```c
   BspGpio_Instance led;
   BspGpio_Instance key;
   ```

3. **注册实例** 🖇️
   ```c
   BspGpio_Register(&led, 'A', 6);    // PA6
   BspGpio_Register(&key, 'E', 3);    // PE3
   ```

4. **读写操作** 🌗
   ```c
   BspGpio_Write(&led, BSP_GPIO_LOW);   // 点亮 LED（视极性而定）
   if (BspGpio_Read(&key) == BSP_GPIO_LOW) { ... }
   ```

5. **注册中断（可选）** 🔗
   ```c
   void KeyCallback(void *data) {
       BspGpio_Instance *led = (BspGpio_Instance *)data;
       BspGpio_Toggle(led);
   }
   BspGpio_AttachIrq(&key, BSP_GPIO_TRIGGER_FALLING, KeyCallback, &led);
   ```
   > ✅ **注意**：`AttachIrq` 只需在初始化时调用一次，别放在 `while(1)` 里循环注册，那不是勤劳，是徒劳 🐝。

---

## 🧪 6. 完整示例：按键翻转 LED

### 6.1 CubeMX 配置
- **LED**：PB6，推挽输出（`GPIO_Output`）
- **按键**：PA15，输入上拉，`GPIO_EXTI15` 模式
- **NVIC**：使能 `EXTI line[15:10] interrupts`
- **调试接口**：`SYS → Debug` 选 `Serial Wire`（释放 PA15）

### 6.2 代码 📝

```c
#include "bsp_gpio.h"

BspGpio_Instance led;
BspGpio_Instance key;

void KeyCallback(void *user_data) {
    // 简单防抖可加个时间戳判断，这里省略
    BspGpio_Toggle((BspGpio_Instance *)user_data);
}

int main(void) {
    // HAL 初始化（CubeMX 生成）
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    // 注册实例
    BspGpio_Register(&led, 'B', 6);
    BspGpio_Register(&key, 'A', 15);

    // 绑定中断（仅一次）
    BspGpio_AttachIrq(&key, BSP_GPIO_TRIGGER_FALLING, KeyCallback, &led);

    while (1) {
        // 主循环可处理其他任务
        // 放空就好，中断会自动翻转 LED ~
    }
}
```

---

## 🌍 7. 移植到其他芯片

当你想把上层代码（Mods/Apps）迁移到 ESP32、K210 等非 STM32 平台时，只需要为新平台写一个 `bsp_gpio.c`，提供完全相同的 `bsp_gpio.h` 接口。你需要实现：

- 🔄 内部映射：将字符端口映射到新芯片的 GPIO 编号（替换 `PortToReg` 函数的逻辑）
- 📟 写/读/翻转操作（调用新平台的 HAL 或直接操作寄存器）
- 🔔 中断回调管理：在新平台的中断入口函数中，调用我们统一的 `HAL_GPIO_EXTI_Callback`（或自定义分发函数），以触发用户回调

上层代码**一个字都不用改**，重新编译就能跑，就像把乐高火车头从轨道换到太空场景，连接器一模一样 🚂➡️🚀。

---

## ⚠️ 8. 注意事项

| 注意事项 | 说明 |
|----------|------|
| 🔤 **端口字符大小写** | 当前实现仅支持大写字母（`'A'`~`'I'`），输入小写会返回 `NULL`，请保持大写习惯 |
| 🔧 **JTAG 占用** | PA15、PB3、PB4 等默认是 JTAG 调试引脚，普通 GPIO 需要先在 CubeMX 中切换到 SWD 或关闭调试 |
| 🕰️ **中断防抖** | 机械按键类信号务必须在中回调中加入消抖（如记录时间戳，忽略 50ms 内重复触发），否则 LED 可能会抽风 🤪 |
| ⚡ **中断回调速度** | 中断回调应快速完成，禁止调用 `HAL_Delay`、复杂打印等耗时操作，建议只置标志位，具体工作在 `main` 循环处理 |
| 🧳 **实例生命周期** | `BspGpio_Instance` 变量必须是全局或静态的，下班（函数返回）后不能让别人找不到它 🔍 |
| 🤝 **与 CubeMX 的配合** | BSP 层绝对不碰硬件配置寄存器，所有模式、时钟、中断使能均由 CubeMX 负责。如果功能不正常，请先检查 CubeMX 配置是否到位 |

---

> 🏁 **总结**：`bsp_gpio` 是你跨芯片框架的第一块积木，以后的电机控制、传感器读取、通信协议都会站在它的肩膀上。一旦习惯这种“一次注册，随处使用”的模式，你会发现：比赛换芯片？那不过是换个 BSP 文件，重新编译一下的事儿～ 🎉

---
