import RPi.GPIO as GPIO  
import time  
  
GPIO.setmode(GPIO.BCM)  

BUTTON_PIN = 5
GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)  

# 长按时间设置（单位：秒）  
LONG_PRESS_TIME = 3  
  
try:  
    while True:  
        # 检测按钮是否被按下 (按下是高电平，弹起是低电平) 
        if GPIO.input(BUTTON_PIN) == GPIO.HIGH:  # 初次检测
            time.sleep(0.02) # 延迟20ms防抖
            if GPIO.input(BUTTON_PIN) == GPIO.HIGH: # 再次检测
                print("按钮长按超过3秒，将关闭树莓派...")  
                time.sleep(3)  
                if GPIO.input(BUTTON_PIN) == GPIO.HIGH: # 如果按钮已经弹起
                    GPIO.cleanup()
                    # os.system("shutdown -h now")
                    print("关机~")  
                else:  
                    print("未达到长按时间")  
except KeyboardInterrupt:  
    print("收到键盘中断，关闭树莓派...")  
    # 清理GPIO设置  
    GPIO.cleanup()