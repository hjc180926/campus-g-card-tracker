import os
import ujson
from media.sensor import *
from media.display import *
from media.media import *
from time import *
import nncase_runtime as nn
import ulab.numpy as np
import time
import image
import _thread
import gc
import array
from machine import UART, Pin, FPIOA

# ====================== 全局配置 ======================
display_mode = "lcd"
if display_mode == "lcd":
    DISPLAY_WIDTH = ALIGN_UP(800, 16)
    DISPLAY_HEIGHT = 480
else:
    DISPLAY_WIDTH = ALIGN_UP(1920, 16)
    DISPLAY_HEIGHT = 1080
OUT_RGB888P_WIDTH = ALIGN_UP(1280, 16)
OUT_RGB888P_HEIGH = 720
CARD_CLASS_NUM = 55

# ====================== 硬件初始化 ======================
fpioa = FPIOA()
# 按键 GPIO53
fpioa.set_function(53, FPIOA.GPIO53)
button = Pin(53, Pin.IN, Pin.PULL_DOWN)
debounce_delay = 20
last_press_time = 0
button_last_state = 0

# UART4 串口：GPIO48=TX，GPIO49=RX，波特率 115200
fpioa.set_function(48, FPIOA.UART4_TXD)
fpioa.set_function(49, FPIOA.UART4_RXD)
uart4 = UART(4, 115200)

# 线程锁，保护全局数据
data_lock = _thread.allocate_lock()

# ====================== 全局识别缓存 ======================
num = array.array('H', [0] * CARD_CLASS_NUM)
flag_haddis = array.array('H', [0] * CARD_CLASS_NUM)
dis = array.array('H', [0] * CARD_CLASS_NUM)
result_count = 0
labels = []
osd_img = image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, image.ARGB8888)
root_path = "/sdcard/mp_deployment_source/"
config_path = root_path + "deploy_config.json"
debug_mode = 1

# ====================== 串口发送函数（原生 UTF-8） ======================
def send_card_uart4(chinese_name):
    # 兜底过滤：空内容、无类别 一律不发送
    clean_name = str(chinese_name).strip().replace("\r", "").replace("\n", "")
    if not clean_name or clean_name == "无":
        return
    # 默认 encode() 为 UTF-8，末尾加换行符作为帧结束标志
    uart4.write((clean_name + "\n").encode())
    print("串口4已发送牌名:", clean_name)
    time.sleep_ms(10)

# ====================== 按键清零线程 ======================
def key_action():
    global result_count, dis, flag_haddis, num, last_press_time, button_last_state
    while True:
        button_state = button.value()
        current_time = time.ticks_ms()
        if button_state == 1 and button_last_state == 0:
            if current_time - last_press_time > debounce_delay:
                last_press_time = current_time
                with data_lock:
                    result_count = 0
                    print("按键清零，所有识别记录重置")
                    for i in range(CARD_CLASS_NUM):
                        dis[i] = 0
                        flag_haddis[i] = 0
                        num[i] = 0
        button_last_state = button_state

# ====================== 识别结果处理 ======================
def result_dis():
    global num, labels, osd_img, result_count, flag_haddis
    with data_lock:
        for i in range(len(num)):
            # 连续识别超过10次判定稳定，且过滤掉「无」背景类别
            if num[i] > 10 and flag_haddis[i] == 0 and labels[i] != "无":
                flag_haddis[i] = 1
                if result_count < len(dis):
                    dis[result_count] = i
                    result_count += 1
                    send_card_uart4(labels[i])
    # LCD 屏幕绘制已识别卡牌列表
    for i in range(result_count):
        if i < 5:
            osd_img.draw_string_advanced(5, 70 + i * 70, 64, labels[dis[i]], color=(255,105,180))

# ====================== 工具函数 ======================
def read_deploy_config(config_path):
    with open(config_path, 'r') as json_file:
        config = ujson.load(json_file)
    return config

def softmax(x):
    exp_x = np.exp(x - np.max(x))
    return exp_x / np.sum(exp_x)

def sigmoid(x):
    return 1 / (1 + np.exp(-x))

# ====================== AI 识别主流程 ======================
def classification():
    global labels
    print("=== 扑克牌识别启动（串口UTF-8输出）===")
    deploy_conf = read_deploy_config(config_path)
    kmodel_name = deploy_conf["kmodel_path"]
    labels = deploy_conf["categories"]
    confidence_threshold = deploy_conf["confidence_threshold"]
    img_size = deploy_conf["img_size"]
    num_classes = deploy_conf["num_classes"]

    # KPU、AI2D 硬件初始化
    kpu = nn.kpu()
    ai2d = nn.ai2d()
    kpu.load_kmodel(root_path + kmodel_name)
    ai2d.set_dtype(nn.ai2d_format.NCHW_FMT,
                   nn.ai2d_format.NCHW_FMT,
                   np.uint8, np.uint8)
    ai2d.set_resize_param(True, nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
    ai2d_builder = ai2d.build([1,3,OUT_RGB888P_HEIGH,OUT_RGB888P_WIDTH], [1,3,img_size[0],img_size[1]])

    # 摄像头初始化
    sensor = Sensor()
    sensor.reset()
    sensor.set_hmirror(False)
    sensor.set_vflip(False)
    sensor.set_framesize(width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT)
    sensor.set_pixformat(PIXEL_FORMAT_YUV_SEMIPLANAR_420)
    sensor.set_framesize(width=OUT_RGB888P_WIDTH, height=OUT_RGB888P_HEIGH, chn=CAM_CHN_ID_2)
    sensor.set_pixformat(PIXEL_FORMAT_RGB_888_PLANAR, chn=CAM_CHN_ID_2)
    sensor_bind_info = sensor.bind_info(x=0, y=0, chn=CAM_CHN_ID_0)
    Display.bind_layer(**sensor_bind_info, layer=Display.LAYER_VIDEO1)

    # 屏幕初始化
    if display_mode == "lcd":
        Display.init(Display.ST7701, to_ide=True)
    else:
        Display.init(Display.LT9611, to_ide=True)

    try:
        MediaManager.init()
        sensor.run()
        rgb888p_img = None
        ai2d_input_tensor = None
        data = np.ones((1,3,img_size[0],img_size[1]), dtype=np.uint8)
        ai2d_output_tensor = nn.from_numpy(data)

        while True:
            cls_idx = -1
            score = 0.0
            rgb888p_img = sensor.snapshot(chn=CAM_CHN_ID_2)
            if rgb888p_img == -1:
                continue
            if rgb888p_img.format() == image.RGBP888:
                ai2d_input = rgb888p_img.to_numpy_ref()
                ai2d_input_tensor = nn.from_numpy(ai2d_input)
                ai2d_builder.run(ai2d_input_tensor, ai2d_output_tensor)
                kpu.set_input_tensor(0, ai2d_output_tensor)
                kpu.run()
                results = []
                for i in range(kpu.outputs_size()):
                    output_data = kpu.get_output_tensor(i)
                    result = output_data.to_numpy()
                    del output_data
                    results.append(result)
                # 多分类模型
                if num_classes > 2:
                    softmax_res = softmax(results[0][0])
                    cls_idx = np.argmax(softmax_res)
                    if softmax_res[cls_idx] > confidence_threshold:
                        score = softmax_res[cls_idx]
                    else:
                        cls_idx = -1
                # 二分类模型
                else:
                    sigmoid_res = sigmoid(results[0][0][0])
                    if sigmoid_res > confidence_threshold:
                        cls_idx = 1
                        score = sigmoid_res
                    else:
                        cls_idx = 0
                        score = 1 - sigmoid_res
            osd_img.clear()
            # 关键修改：只有识别到有效牌（非「无」类别）才显示、才计数
            if cls_idx >= 0 and labels[cls_idx] != "无":
                osd_img.draw_string_advanced(5, 5, 64, labels[cls_idx], color=(0,255,255))
                with data_lock:
                    num[cls_idx] += 1
            result_dis()
            Display.show_image(osd_img, 0, 0, Display.LAYER_OSD3)
            rgb888p_img = None
            gc.collect()
    except Exception as e:
        print("运行异常:", e)
    finally:
        os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
        sensor.stop()
        Display.deinit()
        MediaManager.deinit()
        gc.collect()
        time.sleep(1)
        nn.shrink_memory_pool()
    print("程序退出")

# ====================== 程序入口 ======================
if __name__ == "__main__":
    _thread.start_new_thread(key_action, ())
    classification()
