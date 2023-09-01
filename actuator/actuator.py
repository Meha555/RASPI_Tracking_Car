import RPi.GPIO as GPIO
import time

# 设置GPIO模式为BCM编码方式
GPIO.setmode(GPIO.BCM)

# 设置GPIO引脚4为输出模式
servo_pin = 4
GPIO.setup(servo_pin, GPIO.OUT)

# 创建PWM对象，设置频率为100Hz（舵机内是20ms的周期）
pwm = GPIO.PWM(servo_pin,100)

# 启动PWM信号输出
pwm.start(0)

# [-90,0] -> [0.5ms,1.5ms]
# (0,90] -> [1.5ms,2.5ms]
def get_duty_cycle(angle):
    return angle / 9 + 15 # 可以发现角度和占空比的对应是线性函数

try:
    while True:
        for angle in range(-90, 91, 1):
            print(f"舵机左转{angle}度")
            pwm.ChangeDutyCycle(get_duty_cycle(angle))
            time.sleep(0.02)
        for angle in range(90, -90, -1):
            print(f"舵机右转{angle}度")
            pwm.ChangeDutyCycle(get_duty_cycle(angle))
            time.sleep(0.02)
            
except KeyboardInterrupt:
    pwm.ChangeDutyCycle(get_duty_cycle(0)) # 回正（0度）
    time.sleep(0.7) # 回正需要时间
    # 停止PWM信号输出
    pwm.stop()
    # 清除GPIO引脚状态
    GPIO.cleanup()
