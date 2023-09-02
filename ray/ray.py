import RPi.GPIO as GPIO
import time

LINE_L = 26 # 左边循迹灯
LINE_R = 6 # 右边循迹灯
LINE_M1 = 19 # 中间循迹灯左
LINE_M2 = 13 # 中间循迹灯右

GPIO.setmode(GPIO.BCM)

GPIO.setup(LINE_L,GPIO.IN,GPIO.PUD_UP)
GPIO.setup(LINE_R,GPIO.IN,GPIO.PUD_UP)
GPIO.setup(LINE_M1,GPIO.IN,GPIO.PUD_UP)
GPIO.setup(LINE_M2,GPIO.IN,GPIO.PUD_UP)

# 碰到黑带是高电平，碰到地板是低电平

try:
    while True:
        # 中间都低电平，说明没黑带，可以转弯
        if(GPIO.input(LINE_M1) == GPIO.LOW and GPIO.input(LINE_M2) == GPIO.LOW):
            # 左侧高电平，右侧低电平，说明左侧碰到黑带，应该左转
            if(GPIO.input(LINE_L) == GPIO.HIGH and GPIO.input(LINE_R) == GPIO.LOW):
                print("左转")
            # 右侧高电平，左侧低电平，说明右侧碰到黑带，应该右转
            elif(GPIO.input(LINE_R) == GPIO.HIGH and GPIO.input(LINE_L) == GPIO.LOW):
                print("右转")
        # 中间至少有一个为高电平，说明有黑带，可以直行
        elif(GPIO.input(LINE_M1) == GPIO.HIGH or GPIO.input(LINE_M2) == GPIO.HIGH):
            # 左右两侧都是低电平，说明可以直走
            if(GPIO.input(LINE_L) == GPIO.LOW and GPIO.input(LINE_R) == GPIO.LOW):
                print("直行")
        time.sleep(0.02) # 行进3s

        # # 左右两侧都是低电平，说明可以直走
        # if(GPIO.input(LINE_L) == GPIO.LOW and GPIO.input(LINE_R) == GPIO.LOW):
        #     print("直行")
        # # 左侧高电平，右侧低电平，说明左侧碰到黑带，应该左转
        # elif(GPIO.input(LINE_L) == GPIO.HIGH and GPIO.input(LINE_R) == GPIO.LOW):
        #     print("左转")
        # # 右侧高电平，左侧低电平，说明右侧碰到黑带，应该右转
        # elif(GPIO.input(LINE_R) == GPIO.HIGH and GPIO.input(LINE_L) == GPIO.LOW):
        #     print("右转")
        # time.sleep(3) # 行进3s
except KeyboardInterrupt:
    GPIO.cleanup()