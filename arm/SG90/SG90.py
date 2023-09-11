import RPi.GPIO as GPIO
import time

class SG90:
    angle = 90
    # 构造函数
    def __init__(self, pin:int):
        self.__pin = pin
        GPIO.setup(pin, GPIO.OUT, initial = GPIO.LOW) #初始化引脚
        self.__pwm = GPIO.PWM(pin, 50)#生成pwm控制对象
        self.__pwm.start(0)#pwm启动
    #析构函数
    def __del__(self):
        self.rotation(90)
        time.sleep(0.5)
        self.__pwm.stop()
    #舵机旋转角度
    def rotation(self, angle):
        if(angle not in range(0,181)):
            print("角度过大")
            return
        self.angle = angle
        duty = angle / 18 + 2
        GPIO.output(self.__pin, True)
        self.__pwm.ChangeDutyCycle(duty)
        time.sleep(0.2)
        GPIO.output(self.__pin, False)
        self.__pwm.ChangeDutyCycle(0)
    #获取角度
    def get_angles(self):
        return self.angle

if __name__ == "__main__":
    GPIO.setmode(GPIO.BCM)
    s = SG90(25)
    for x in range(0,181,10):
        s.rotation(x)
    gpio.cleanup()