#ifndef CONTROL_H
#define CONTROL_H

void control_init(void);// 初始化控制器
void control_set_target(float left_mps, float right_mps);// 设置目标速度，单位 m/s
void control_stop(void);// 停止控制器
void control_update_1ms(void);// 1 ms 定时更新控制器，计算输出占空比

#endif