# K230 扑克牌识别（视觉端）

校赛 G 题的视觉部分：K230 摄像头实时识别路径旁的扑克牌，通过串口把牌名发给 STM32 主控显示。

原工程目录名为「扑克牌k230识别 - 中文」，主程序原文件名 `temp.py`，此处改名为 `card_recognition.py`。

## 识别方式

**图像分类**，不是目标检测。模型输入 224×224，输出 55 类：

| 类别 | 数量 |
|---|---|
| 大王 / 小王 | 2 |
| 黑桃 / 梅花 / 红桃 / 方块 × (2–10, A, J, Q, K) | 52 |
| 背面 / 空白（用于过滤） | 1 |

模型：`can2_10.0s_20260527200114.kmodel`（约 2.6 MB，nncase 2.9.0 量化，uint8 输入输出）。

## 判定逻辑

单帧分类结果不可靠，因此按「连续命中帧数」确认：

```
每帧推理 → 置信度做 softmax → 取最大类
        → num[class]++
        → num[class] > 10 且该牌未上报过  → 判为「识别到」
        → 通过 UART4 发送牌名，LCD 上显示（最多 5 条）
```

- `CARD_CLASS_NUM = 55`，每个类独立计数
- 同一张牌只上报一次（`flag_haddis` 标记）
- 背面 / 空白直接过滤，不发送
- 按键（GPIO53）按下可清零，重新开始识别

## 串口协议

| 项 | 值 |
|---|---|
| 外设 | UART4 |
| 引脚 | GPIO48 = TX，GPIO49 = RX |
| 波特率 | 115200，8N1 |
| 编码 | **UTF-8** |
| 帧格式 | 牌名 + `\n`（如 `红桃A\n`） |

接线到主控：**K230 GPIO48（UART4_TX）→ STM32 PB11（USART3_RX）**。
STM32 端按 `\n` 判帧，`main.c` 里 `uart_rx_complete` 置位后整包缓存。

## 文件

```
card_recognition.py                     主程序（摄像头 + 分类 + 串口发送）
mp_deployment_source/
  deploy_config.json                    模型配置（类别表、阈值、归一化参数）
  can2_10.0s_20260527200114.kmodel     量化后的模型
  cls_image.py                          CanMV 官方分类例程（单图）
  cls_video.py                          CanMV 官方分类例程（视频流）
```

## 部署

1. 把 `mp_deployment_source/` 整个目录拷到 SD 卡根目录
2. `card_recognition.py` 放到 CanMV IDE 运行，或改名为 `main.py` 开机自启
3. 需要 `deploy_config.json` 里的 `kmodel_path` 与实际文件名一致

## 注意

- 分类模型要求牌面**正对镜头、占画面比例足够大**，倾斜或过远时置信度会掉
- 训练集只有 55 类，正面朝下的牌会被判成「背面」，属预期行为
- 发送的是中文牌名，**STM32 端必须有对应字库**（见仓库 `v3` 标签的 `OLED_Data.c`）
