# campus-g-card-tracker

校赛 G 题小车：**循迹绕圈 + 识别路径旁的扑克牌**。

车沿场地边缘循迹跑圈，K210 摄像头识别路径旁的扑克牌并通过串口把牌面发给主控，
主控在 OLED 上显示识别结果、用蜂鸣器提示圈数完成。

## 功能

- 5 路灰度循迹，差速转向控制
- 编码器计程（MG513X，71 脉冲/cm），一圈 314 cm 自动计圈
- K210 通过 **USART3** 发送牌面，主控缓存最多 5 张
- OLED 显示牌面（v2 拼音 / v3 中文，v3 自带字库）
- 按键切换显示页、蜂鸣器提示

## 硬件

| 模块 | 引脚 |
|---|---|
| 电机方向 | AIN1 PB13 · AIN2 PB12 · BIN1 PB14 · BIN2 PB15 |
| 电机 PWM | PWMA PA11 · PWMB PA8（TIM1） |
| 编码器 | 左 PA0/PA1（TIM2）· 右 PB6/PB7（TIM4） |
| 灰度循迹 | L2 PA5 · L1 PA6 · M PA7 · R1 PB0 · R2 PB1 |
| 视觉串口 | USART3 RX：PB11（接 K230 UART4 TX） |
| 显示 | OLED 128×64，软件 I2C：PB8 SCL / PB9 SDA |
| 按键 | PB3 · PB4（上拉输入，JTAG 已关闭以释放引脚） |
| LED / 蜂鸣器 | LED：PA2 · 蜂鸣器：PB5 |
| 电源 | 12V 输入，LM2596 降到 5V，MCU 3.3V |

> 完整引脚表、定时器分配与关键参数见 [`docs/校赛G题-小车配置.docx`](docs/校赛G题-小车配置.docx)。

## 版本

| 标签 | 内容 | 日期 |
|---|---|---|
| `v1` | 基础版：循迹 + 编码器计圈 | 2026-05-30 |
| `v2` | 加入 K210 牌面接收，OLED 以拼音显示 | 2026-07-01 |
| `v3` | 改为中文显示（引入字库），重写显示与滚动逻辑 | 2026-07-10 |

v2 与 v1 的差异集中在 `main.c`、`uart.c`、`uart.h`、`stm32f10x_it.c`（串口接收）。
v3 相对 v2 改动更大：新增 `OLED_Data.c` / `OLED_Data.h` 字库，`main.c`、`OLED.c`、
`motor.c`、`tim.c` 等 19 个文件均有修改。

## 目录

```
Hardware/        电机、编码器、OLED、循迹、蜂鸣器、按键、串口驱动
User/            main.c、中断与时钟配置
System/          Delay 延时
Library/         STM32F10x 标准外设库
start/           启动文件与 CMSIS
progect.uvprojx  Keil 工程文件
k230-card-recognition/   K230 视觉端（MicroPython，分类模型 55 类）
docs/            赛题原文与引脚配置文档
```

## 赛题

2026 年校赛 **G 题「智能识别小车」**：

- 小车从引导线任意位置出发，自主沿黑色引导线行驶
- 每跑完一圈回到起点发出声光提示，行驶两圈后自动停止
- 引导线上随机放置 1–3 张背面朝上的扑克牌，不影响正常行驶
- 发挥部分：识别标志圆外切线上的 1–5 张扑克牌（正面朝上），显示识别结果
- 有路程与时间显示，并尽量提高一圈速度

原文见 [`docs/G题-智能识别小车-赛题.pdf`](docs/G题-智能识别小车-赛题.pdf)，
引脚与定时器分配见 [`docs/校赛G题-小车配置.docx`](docs/校赛G题-小车配置.docx)。

## 编译

1. Keil MDK 打开根目录的 `progect.uvprojx`
2. 选择目标后 Build，下载到 STM32F103

## 注意

- **源文件为 GBK 编码**（Keil 默认）。用 VS Code 打开中文注释会乱码，
  需把编码切换为 GBK / GB2312，或安装 GBK 插件。
- 仓库不含 `Objects/`、`Listings/` 等编译产物，clone 后需自行编译。
- `Library/` 是 STM32F10x 标准外设库，`start/` 是启动文件，均保留以便直接编译。
- 视觉端代码见 `k230-card-recognition/`，K230 通过 UART4 以 UTF-8 发送牌名，
  接 STM32 的 USART3（PB11）。
