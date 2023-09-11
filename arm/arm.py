import RPi.GPIO as GPIO
import time
from sshkeyboard import listen_keyboard
from SG90.SG90 import SG90

SG90_PIN = (23,24,25)

def press(key):
    if key == "up":
        print("上")
        SG90S[2].rotation(SG90S[2].get_angles()+20)
    elif key == "down":
        print("下")
        SG90S[2].rotation(SG90S[2].get_angles()-20)
    elif key == "left":
        print("左")
        SG90S[1].rotation(SG90S[1].get_angles()+20)
    elif key == "right":
        print("右")
        SG90S[1].rotation(SG90S[1].get_angles()-20)
    elif key == "[":
        print("松开")
        x = SG90S[0].get_angles()+20
        if(x in range(0,90)):
            SG90S[0].rotation(x)
        else:
            print("爪部角度过大")
    elif key == "]":
        print("夹紧")
        x = SG90S[0].get_angles()-20
        if(x in range(0,90)):
            SG90S[0].rotation(x)
        else:
            print("爪部角度过大")

SG90S = []
GPIO.setmode(GPIO.BCM)

# 创建各自由度的舵机实例
for i in range(len(SG90_PIN)):
    SG90S.append(SG90(SG90_PIN[i]))
try:
    listen_keyboard(on_press=press)
except KeyboardInterrupt:
    pass
    # for sg90 in SG90S:
    #     del sg90
    #     time.sleep(2)
for sg90 in SG90S:
    del sg90
    time.sleep(0.5)
GPIO.cleanup()