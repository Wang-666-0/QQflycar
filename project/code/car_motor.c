#include "car_motor.h"

// 左电机：PH 控制方向，EN 输出 PWM
#define LEFT_MOTOR_PH_PIN       E11
#define LEFT_MOTOR_EN_PWM       TIM1_PWM_MAP3_CH1_E9

// 右电机：PH 控制方向，EN 输出 PWM
#define RIGHT_MOTOR_PH_PIN      E13
#define RIGHT_MOTOR_EN_PWM      TIM1_PWM_MAP3_CH4_E14

// 左右电机使用同一个定时器，PWM 频率必须相同
#define MOTOR_PWM_FREQ          17000


// 当前测试程序的最大占空比：100%
#define MOTOR_TEST_DUTY_MAX     10000

// 前进方向电平，实际方向需要根据车轮转向调整
#define LEFT_FORWARD_LEVEL      1
#define RIGHT_FORWARD_LEVEL     0

void car_motor_init(void)
{
    // 初始化左右方向引脚
    gpio_init(
        LEFT_MOTOR_PH_PIN,
        GPO,
        LEFT_FORWARD_LEVEL,
        GPO_PUSH_PULL
    );

    gpio_init(
        RIGHT_MOTOR_PH_PIN,
        GPO,
        RIGHT_FORWARD_LEVEL,
        GPO_PUSH_PULL
    );

    // 初始化左右 EN 的 PWM，占空比先设为 0
    pwm_init(LEFT_MOTOR_EN_PWM, MOTOR_PWM_FREQ, 0);
    pwm_init(RIGHT_MOTOR_EN_PWM, MOTOR_PWM_FREQ, 0);
}

void car_motor_set(uint16 left_duty, uint16 right_duty)
{
    // 限制左电机的最大占空比
    if (left_duty > MOTOR_TEST_DUTY_MAX)
    {
        left_duty = MOTOR_TEST_DUTY_MAX;
    }

    // 限制右电机的最大占空比
    if (right_duty > MOTOR_TEST_DUTY_MAX)
    {
        right_duty = MOTOR_TEST_DUTY_MAX;
    }

    pwm_set_duty(LEFT_MOTOR_EN_PWM, left_duty);
    pwm_set_duty(RIGHT_MOTOR_EN_PWM, right_duty);
}

void car_motor_stop(void)
{
    pwm_set_duty(LEFT_MOTOR_EN_PWM, 0);
    pwm_set_duty(RIGHT_MOTOR_EN_PWM, 0);
}