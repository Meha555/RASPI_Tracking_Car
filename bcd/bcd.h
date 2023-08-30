#ifndef BCD_H
#define BCD_H

extern void tm1637_init();
extern void bcd_display(int h_shi, int h_ge, int m_shi, int m_ge, int flag);

#endif // BCD_H