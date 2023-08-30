import RPi.GPIO as GPIO
import time
import datetime 
import pytz
  
GPIO.setmode(GPIO.BCM)

DIO_PIN = 22
CLK_PIN = 27

class tm1637:
    __segdata = (63,6,91,79,102,109,125,7,127,111) #0-9
    def __init__ (self,CLK:int,DIO:int) -> None:
        self.__CLK = CLK
        self.__DIO = DIO
        GPIO.setup(self.__DIO,GPIO.OUT)
        GPIO.setup(self.__CLK,GPIO.OUT)

    def __start(self):
        GPIO.output(self.__CLK, GPIO.HIGH)# CLK引脚提到高电平,接受开始信号
        time.sleep(0.000140)
        #开始信号:self._DIO引脚由高电平变为低电平
        GPIO.output(self.__DIO, GPIO.HIGH)
        time.sleep(0.000140)
        GPIO.output(self.__DIO, GPIO.LOW)
        time.sleep(0.000140)
        GPIO.output(self.__CLK,GPIO.LOW) #CLK引脚降到低电平,准备后续的数据/命令接收
        time.sleep(0.000140)

    def __stop(self):
        GPIO.output(self.__CLK,GPIO.LOW) # CLK引脚降到低电平,保证self.__DIO变为低电平时不会触发开始信号
        time.sleep(0.000140)
        GPIO. output(self.__DIO,GPIO.LOW) # 拉低DIO
        time.sleep(0.000140)
        GPIO.output(self.__CLK,GPIO.HIGH) # CLK引脚提到高电平,接受结束信号
        time.sleep(0.000140)
        #结束信号:self.__DIO引脚由低电平变为高电平
        GPIO.output(self.__DIO, GPIO.HIGH)
        time.sleep(0.000140)

    def __write_bit(self, bit:int):
        GPIO.output(self.__CLK,GPIO.LOW) # 保险：确保输入时，CLK引脚在低电平时
        time.sleep(0.000140)
        GPIO.output(self.__DIO,bit)
        time.sleep(0.000140)
        GPIO.output(self.__CLK, GPIO.HIGH) # CLK引脚拉高，结束当前bit的传输
        time.sleep(0.000140)

    def __write_byte(self,byte:int):
        for x in range(0,8):
            self.__write_bit((byte>>x)&1)
        # 处理第九个CLK的ACK信号
        GPIO.output(self.__CLK,GPIO.LOW)
        time.sleep(0.000140)
        GPIO.output(self.__DIO,GPIO.HIGH)
        time.sleep(0.000140)
        GPIO.output(self.__CLK,GPIO.HIGH)
        time.sleep(0.000140)
        """
        临时设置数据线为输入模式，从而检测数据线的电平
        等待数据线变为低电平，表示芯片已经接收完所有的数据，然后将数据线重新设置为输出模式。
        """
        GPIO.setup(self.__DIO,GPIO.IN)
        while(GPIO.input(self.__DIO)==GPIO.HIGH):
            pass
        GPIO.setup(self.__DIO,GPIO.OUT)

    def write_command(self,cmd:int):
        self.__start()
        self.__write_byte(cmd)
        self.__stop()

    def write_data(self,addr:int,data:int):
        self.__start()
        self.__write_byte(addr)
        self.__write_byte(data)
        self.__stop()

    def set_num_display(self,n1:int,n2:int,n3:int,n4:int,dp:bool):
        self.write_command(64)# 命令：数据写入寄存器
        self.write_command(68)# 命令：固定地址设置法
    # 设置数据
        self.write_data(192,self.__segdata[n1])
        self.write_data(193,self.__segdata[n2]+dp*128)
        self.write_data(194,self.__segdata[n3])
        self.write_data(195,self.__segdata[n4])
        self.write_command(136)#显示打开

if __name__ == '__main__':
    bcd = tm1637(CLK_PIN,DIO_PIN)
    timezone = pytz.timezone('Asia/Shanghai')
    try:  
        while True:
            utc_time = datetime.datetime.now(pytz.utc)
            local_time = utc_time.astimezone(timezone)
            bcd.set_num_display(int(local_time.hour/10),int(local_time.hour%10),int(local_time.minute/10),int(local_time.minute%10),True)
            time.sleep(0.01)
    except KeyboardInterrupt:  
        GPIO.cleanup()
