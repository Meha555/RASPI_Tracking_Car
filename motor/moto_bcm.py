import RPi.GPIO as gpio
import time

wheelL = (28,29)#左轮
wheelR = (26,27)#右轮

gpio.setwarnings(False)#屏蔽警告
gpio.setmode(gpio.BCM)#设置引脚模式
gpio.setup(wheelL ,gpio.OUT)# 设置引脚m1方向输出
gpio.setup(wheelR, gpio.OUT)# 设置引脚m2方向输出
for i in range(2):
    PWL = gpio.PWM(wheelL[i] ,500) # 左轮设置为PWM控制
    PWR = gpio.PWM(wheelR[i],500) # 右轮设置为PWM控制
    PWL.start(0)
    PWR.start(0)

try:
    while True:
        cmd = input("请输入ac加速/dc减速:")
        if cmd == 'ac': # 加速
            gpio.output (wheelR, gpio.LOW)#
            PWL.ChangeDutyCycle(0)#
            for dc in range(0,100, 5):
                PWL.ChangeDutyCycle(dc)#
                time.sleep(1)
        elif cmd == 'dc':# 减速
            gpio.output (wheelR , gpio.LOW)
            PWL.ChangeDutyCycle(100)
            for dc in range(100,0,-5):
                PWL.ChangeDutyCycle(dc)
                time.sleep(1)
        elif cmd == 'p':
            gpio.output(wheelL , gpio.HIGH)#
            gpio.output(wheelR , gpio.HIGH)#
            gpio.setup(wheelL , gpio.IN)#设置引脚m1方向输入
            gpio.setup(wheelR, gpio.IN)#设置引脚m2方向输入
        break
        #捕获ctr1+C，并停止电机转动
except KeyboardInterrupt:
    gpio.output(wheelL, gpio.HIGH)#
    gpio.output(wheelR, gpio.HIGH)#
    gpio.setup(wheelL ,gpio.IN)#设置引脚m1方向输入
    gpio.setup(wheelR ,gpio.IN)#设置引脚m2方向输入

print('Pin clean')
gpio.cleanup()