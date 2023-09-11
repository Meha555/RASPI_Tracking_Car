# RASPI_Tracking_Car

#### 介绍

树莓派循迹小车，包含红外线检测路线、超声波避障、温度传感器、数码管显示、QT上位机等

#### TODO

- [x] 完善读写锁（针对`TcpParam`的读写）
- [x] 添加QT上位机
- [x] 为最终的可执行文件添加命令行参数控制启动模块

#### 软件架构

主程序main.c放在根目录，其他各文件夹内是支持主程序的功能代码以及对应各模块的测试代码。

arm文件下内是机械臂的代码。

#### 硬件说明

- 树莓派3B
- 直流电机+L29N电机驱动模块
- 数码管+TM1637驱动芯片
- 超声波传感器HC-SR04
- 红外线传感器TCRT5000
- 有源蜂鸣器
- 温度传感器DHT11
- 舵机SG90

#### 使用说明

- 需要安装wiringPi库和RPI.GPIO库
- 需要使用全部模块，需要取消`COMMAND_LINE`宏

#### 存在问题

- 由于（舵机+键盘控制）是一组，（循迹）是单独一组，在使用QT上位机切换两组功能时，需要手动开关蜂鸣器或按一个键来触发TCP通信，解决方案是在协议中添加特定分组来在每次切换控制模式后刷新数据。
