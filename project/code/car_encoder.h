#ifndef CAR_ENCODER_H
#define CAR_ENCODER_H

#include "zf_common_headfile.h"

// 初始化左右编码器
void car_encoder_init(void);

// 获取左编码器当前计数
int32 car_encoder_get_left(void);

// 获取右编码器当前计数
int32 car_encoder_get_right(void);

extern volatile float car_speed_left_mps;//
extern volatile float car_speed_right_mps;

void car_encoder_sample_1ms(void);
#endif