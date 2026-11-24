#ifndef CAR_ENCODER_H
#define CAR_ENCODER_H

#include "zf_common_headfile.h"

// 初始化左右编码器
void car_encoder_init(void);

extern volatile float car_speed_left_mps;// 左轮速度，单位 m/s
extern volatile float car_speed_right_mps;// 右轮速度，单位 m/s

void car_encoder_sample_1ms(void);
#endif