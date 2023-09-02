#ifndef RAY_H
#define RAY_H

extern void initial_tcrt5000();
extern void tracking();

struct TCRT5000 {
    int line_l;   // 左边循迹灯
    int line_r;   // 右边循迹灯
    int line_m1;  // 中间循迹灯左
    int line_m2;  // 中间循迹灯右
};

extern struct TCRT5000 tracker;

#endif  // RAY_H