import time
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(13, GPIO.OUT)
  
p = GPIO.PWM(13, 50)  # 创建PWM实例，引脚18，频率50Hz

p.start(0) # 启动PWM，初始为灭，占空比为
try:
    while True:
        for dc in range(0, 101, 5):
            p.ChangeDutyCycle(dc)
            time.sleep(0.1)
        for dc in range(100, -1, -5):
            p.ChangeDutyCycle(dc)
            time.sleep(0.1)
except KeyboardInterrupt:
    pass

p.stop()
print('Pin clean')
GPIO.cleanup()