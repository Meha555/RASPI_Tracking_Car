#ifndef DHT11_H
#define DHT11_H

#define DATA_PIN 1

extern void dht11_reset();
extern unsigned char read_bit();
extern unsigned char read_byte();
extern int dht11_read_data(unsigned char buff[]);

#endif  // DHT11_H