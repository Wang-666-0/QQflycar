#ifndef CAR_MOTOR_H
#define CAR_MOTOR_H

#include "zf_common_headfile.h"

// 初始化左右电机
void car_motor_init(void);

// 设置左右电机的 PWM 占空比
void car_motor_set(uint16 left_duty, uint16 right_duty);

// 停止左右电机
void car_motor_stop(void);

#endif