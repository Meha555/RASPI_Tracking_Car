import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)

LED = (13,19,26)

GPIO.setup(LED, GPIO.OUT)

try:
    i = 0
    while True:
        print('LED %s on' % i)
        GPIO.output(LED[i], GPIO.HIGH)
        time.sleep(1.5)

        print('LED %s off' % i)
        GPIO.output(LED[i], GPIO.LOW)
        time.sleep(1.5)
        i = (i + 1) % 3

except KeyboardInterrupt:
    pass

print('pin clean')
GPIO.cleanup()
