# ⏱️ BSP DWT 高精度时间库使用手册

## 📖 1. 概述

`bsp_dwt` 是基于 **Cortex-M 内核 DWT（Data Watchpoint and Trace）** 单元的高精度时间库，为你的机器人控制系统提供 **与硬件定时器解耦** 的微秒级时钟服务。
它的核心思想是：**利用 CPU 内部的周期计数器，不受中断抢占、任务调度影响，永远稳定、精准地记录时间流逝** ⏲️。

在整个跨芯片框架中，该模块负责提供统一的 **时间线获取、代码段耗时测量、高精度延时** 等功能，让上层 Mods（如 PID 控制器、运动规划）和 Apps 可以完全信赖一个 **与平台无关** 的时间基准。

---

## ✨ 2. 设计特点

- ⏱️ **硬件级精度**：直接读取 CPU 的 `CYCCNT` 寄存器，分辨率 = 1 / CPU 频率（例如 168MHz 下分辨率 ≈ 5.95ns）。
- 🌐 **全局时间线**：内部维护一个可跨越溢出的 64 位计数器，提供秒、毫秒、微秒三种时间戳，方便不同场景使用。
- 🔁 **自动溢出处理**：DWT 计数器是 32 位，最高计数到 2³²-1 后会回绕。本模块在每次调用更新函数时自动检测并累加溢出次数，确保时间连续不断。
- ⚡ **轻量无依赖**：仅操作 Cortex-M 内核寄存器，不占用任何硬件定时器外设，不与 RTOS 或中断冲突。
- 📏 **代码耗时测量**：通过宏 `TIME_ELAPSE` 可优雅地测量任意代码块执行时间，调试利器。
- 🛑 **与平台无关**：虽然依赖 Cortex-M 内核，但接口封装对上层完全透明。换到其他 Cortex-M 芯片（如 STM32F1、F4、H7）只需调整 CPU 频率参数即可。

---

## 📁 3. 文件说明

| 文件 | 位置 | 说明 |
|------|------|------|
| `bsp_dwt.h` | `bsp/common/` | 公共接口头文件（平台无关），包含宏定义与函数声明 🎛️ |
| `bsp_dwt.c` | `bsp/<platform>/` | 平台相关实现（如 `stm32f407/bsp_dwt.c`），主要涉及内核寄存器操作与频率设定 🧠 |

---

## 📚 4. 接口函数

### 4.1 数据类型

```c
// 时间分解结构体（秒、毫秒、微秒）
typedef struct {
    uint32_t s;    // 秒 🕐
    uint16_t ms;   // 毫秒 🕑
    uint16_t us;   // 微秒 🕒
} BspDwt_TimeType;
```

### 4.2 初始化 ⚙️

```c
void BspDwt_Init(uint32_t CPU_Freq_MHz);
```
- **功能**：使能 DWT 单元，初始化 CYCCNT 计数器，并保存 CPU 主频相关信息。
- **参数**：
  `CPU_Freq_MHz` — CPU 主频，单位 MHz。
  通常从板级宏传入，如 `CPU_HERT_F407_MHZ 168`。
- **注意**：该函数必须在系统时钟配置完成后调用一次，且需在首次使用任何定时功能前执行。

### 4.3 获取系统时间线 🕰️

```c
float    BspDwt_GetTimeline_Sec(void);   // 秒（float）
float    BspDwt_GetTimeline_MSec(void);  // 毫秒（float）
uint64_t BspDwt_GetTimeline_USec(void);  // 微秒（uint64）
```
- **功能**：返回从上电复位到当前时刻的累计时间，内部自动处理 32 位计数器溢出，保证长期连续。
- **用法**：
  - 需要精准时间戳时调用，比如 PID 计算中的 `dt`，或者记录日志时间。
  - 返回的浮点秒/毫秒由于精度问题，在长时间后可能有毫秒以下误差，微秒版本使用 `uint64_t` 保证 **不开裂** 的绝对精度 👍。

### 4.4 高精度延时 ⏲️

```c
void BspDwt_Delay(float Delay);
```
- **功能**：使用 DWT 计数器实现的阻塞延时，不依赖 SysTick。
- **参数**：
  `Delay` — 延时时长，单位秒。
- **特点**：精度可达微秒级，且不受中断影响，但在延时期间 CPU 会一直查询计数器，**不释放控制权**。

### 4.5 测量代码段耗时 📏

```c
#define TIME_ELAPSE(dt, code)                                 \
    do {                                                      \
        float tstart = BspDwt_GetTimeline_Sec();              \
        code;                                                 \
        dt = BspDwt_GetTimeline_Sec() - tstart;               \
        LOGINFO("[DWT] " #dt " = %f s\r\n", dt);              \
    } while (0)
```
- **功能**：这个宏可以测量任意 `code` 块的执行时间（秒），并自动通过日志打印出来。
- **使用示例**：
  ```c
  float elapsed;
  TIME_ELAPSE(elapsed, {
      // 你要测试的代码
      BspUart_Transmit(&dbus, data, 18);
  });
  ```

### 4.6 增量时间（Delta Time）⏱️

```c
float  BspDwt_GetDeltaTime(uint32_t *cnt_last);
double BspDwt_GetDeltaTime64(uint32_t *cnt_last);
```
- **功能**：返回从 `cnt_last` 记录的时刻到当前时刻的时间差（秒），同时把 `*cnt_last` 更新为当前 DWT 计数值。
  适合在循环中计算两次执行之间的时间间隔。
- **参数**：
  `cnt_last` — 指向上一次调用的 DWT 快照的指针（用户需要将其定义为全局或静态变量）。
- **区别**：`_64` 版本使用 `double` 计算，精度更高。

### 4.7 系统时间手动更新 🔄

```c
void BspDwt_SysTimeUpdate(void);   // 更新全局时间结构体
void BspDwt_CntUpdate(void);       // 仅更新溢出轮次
```
- **功能**：通常无需手动调用，获取时间线接口会自动处理。但若你想提前更新全局时间（例如在定时中断中），可单独调用。
  `BspDwt_CntUpdate` 只处理溢出计数，不计算秒/毫秒，更低开销。

---

## 🛠️ 5. 使用流程

1. **系统初始化** ⚙️
   在 `main` 函数中，**先完成时钟配置**（`SystemClock_Config()`），然后调用：
   ```c
   BspDwt_Init(CPU_HERT_F407_MHZ);  // 传入 MCU 主频，如 168
   ```

2. **获取时间戳** ⏰
   在任何需要时间的地方，一行代码即可拿到当前时间的秒/毫秒/微秒：
   ```c
   float now_sec = BspDwt_GetTimeline_Sec();
   uint64_t now_us = BspDwt_GetTimeline_USec();
   ```

3. **延时** ⏱️
   ```c
   BspDwt_Delay(0.001);  // 延时 1ms（百万分之一秒误差级别）
   ```

4. **测量代码时间** 📏
   ```c
   float dt;
   TIME_ELAPSE(dt, {
       MyComplexAlgorithm();
   });
   // 日志会自动输出类似：[DWT] dt = 0.002345 s
   ```

5. **控制循环中计算 dt** 🔁
   ```c
   uint32_t last_tick = 0;
   while (1) {
       float dt = BspDwt_GetDeltaTime(&last_tick);
       // dt 就是距离上次调用经过的秒数
       MyPIDController(dt);
   }
   ```

---

## 🧪 6. 完整示例：闪烁 + 计时

```c
#include "bsp_dwt.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    BspDwt_Init(CPU_HERT_F407_MHZ);   // 初始化 DWT

    uint32_t last_time = 0;
    while (1) {
        float dt = BspDwt_GetDeltaTime(&last_time);
        if (dt > 0.5f) {              // 每 0.5s 执行一次
            BspGpio_Toggle(&led);      // 假设你已注册 led 实例
            last_time = 0;             // 重置（这里为演示，实际请保留 last_time）
        }
    }
}
```

---

## 🌍 7. 移植到其他芯片

`bsp_dwt` 依赖 **Cortex-M 内核** 的 `DWT` 模块，因此：

- ✅ 所有 Cortex-M3/M4/M7/M33 等内核的 MCU **可直接使用**，只需修改 `BspDwt_Init` 中传入的 CPU 频率参数（如 STM32F1 是 72MHz，STM32H7 可能 480MHz）。
- ❌ 对于 **其他内核架构**（如 RISC-V、ESP32 的 Xtensa），需要重新实现底层计时机制（可能使用芯片自带的 64 位硬件定时器）。但上层接口 `BspDwt_GetTimeline_Sec` 等可以完全保持一致，只需改写 `bsp_dwt.c` 即可。
- 💡 建议：为不同架构建立一个条件编译宏，保持接口一致。

---

## ⚠️ 8. 注意事项

| 注意事项 | 说明 |
|----------|------|
| 🔢 **CPU 频率必须正确** | `BspDwt_Init` 的参数必须与实际 CPU 频率匹配，否则所有时间都会出错。最好使用 CubeMX 生成的 `HAL_GetTick()` 对照验证 |
| 🔄 **溢出处理时机** | 如果两次调用 `BspDwt_CntUpdate` 的间隔超过了 2³² / CPU频率（例如 168MHz 下约 25.5 秒），就会丢失溢出计数。因此**必须确保至少每 25 秒调用一次时间更新函数**。本库的 `GetTimeline_xxx` 和 `GetDeltaTime` 内部都会调用更新，正常控制循环绝不会达到这么长时间，尽可放心 ✌️ |
| ⚠️ **中断安全** | `BspDwt_CntUpdate` 内部使用了简单的 `bit_locker` 防重入，但并非真正的原子操作，如果存在两个不同优先级的中断同时调用时间线获取函数，极低概率可能出现竞态。建议在**同一优先级或仅在主循环中**调用时间获取函数 |
| 📉 **浮点精度** | `BspDwt_GetTimeline_Sec` 返回 `float`，存储约 7 位有效数字。长时间运行后（数天）秒数很大，毫秒/微秒可能会丢失。需要长时间绝对计时时请使用 `BspDwt_GetTimeline_USec`（`uint64_t`） |
| 🚫 **不可在中断中长时间延时** | `BspDwt_Delay` 是忙等待，在中断中使用会打断一切，仅建议在初始化或极短延时（微秒级）时使用。主循环中长时间延时也不推荐，推荐使用定时器或事件驱动 |
| 📋 **依赖 CMSIS** | `bsp_dwt.c` 包含了 `main.h`（即 ST 库），并直接操作 `DWT->CYCCNT` 等寄存器。如果你使用其他非 CMSIS 的内核，需要自己定义这些寄存器地址 |

---

> 🏁 **总结**：`bsp_dwt` 是你框架的“心脏” ❤️，为所有模块提供一个精确、统一的时间基准。PID 需要 dt、运动控制需要延迟、数据记录需要时间戳——它都能优雅地胜任。有了它，你再也不用纠结 HAL_Delay 不准、SysTick 中断被抢的问题了。快乐地搬砖吧！ ⏳🤖