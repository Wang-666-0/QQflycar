#include "control.h"
#include "car_motor.h"
#include "car_encoder.h"
// 初步调试时，限制控制器的最大 PWM
#define CONTROL_PWM_MAX 10000.0f
// 目标速度限制在 0～1 m/s，避免调试阶段过快
#define CONTROL_TARGET_MAX_MPS 1.0f
// 一个轮子的速度控制器
typedef struct
{
    volatile float target_mps;      // 目标速度，单位 m/s

    float kp;             // 比例系数
    float ki;             // 积分系数
    float kd;             // 微分系数

    volatile float integral;       // 累计误差
    volatile float last_error;     // 上一次的速度误差
    volatile float output_pwm;     // 计算得到的 PWM
} speed_pid_t;

// 左右轮各自保存控制状态
static speed_pid_t left_pid;
static speed_pid_t right_pid;

void control_init(void)
{
    left_pid.target_mps = 0.0f;

    // 先全部设为 0，后面从 Kp 开始调试
    left_pid.kp = 10000.0f;
    left_pid.ki = 5000.0f;
    left_pid.kd = 0.0f;

    left_pid.integral = 0.0f;
    left_pid.last_error = 0.0f;
    left_pid.output_pwm = 0.0f;

    // 初始值相同，之后两套状态分别更新
    right_pid = left_pid;

    car_motor_stop();
}

// 计算一个轮子的 P 控制输出
static float speed_pid_calculate(speed_pid_t *pid, float measured_mps)
{
    float error;
    float next_integral;
    float next_output;
    float output;

    if (pid->target_mps <= 0.0f)
    {
        pid->integral = 0.0f;
        pid->last_error = 0.0f;
        pid->output_pwm = 0.0f;
        return 0.0f;
    }

    error = pid->target_mps - measured_mps;

    // 每次间隔 1 ms，即 0.001 秒
    next_integral = pid->integral + error * 0.001f;

    next_output = pid->kp * error
                + pid->ki * next_integral;

    // 输出饱和且误差还在推动饱和时，暂停积分
    if (!((next_output > CONTROL_PWM_MAX && error > 0.0f)
       || (next_output < 0.0f && error < 0.0f)))
    {
        pid->integral = next_integral;
    }

    // PI 输出
    output = pid->kp * error
           + pid->ki * pid->integral;

    if (output < 0.0f)
    {
        output = 0.0f;
    }
    else if (output > CONTROL_PWM_MAX)
    {
        output = CONTROL_PWM_MAX;
    }

    pid->last_error = error;
    pid->output_pwm = output;

    return output;
}

// 后续由 1 ms 定时中断调用
void control_update_1ms(void)
{
    float left_pwm;
    float right_pwm;

    left_pwm = speed_pid_calculate(
        &left_pid, car_speed_left_mps);

    right_pwm = speed_pid_calculate(
        &right_pid, car_speed_right_mps);

    car_motor_set((uint16)left_pwm, (uint16)right_pwm);
}

// 从主循环调用，设置左右轮目标速度
void control_set_target(float left_mps, float right_mps)
{
    uint32 irq_token;

    // 将负值或无效数值归零
    if (!(left_mps >= 0.0f))
        left_mps = 0.0f;

    if (!(right_mps >= 0.0f))
        right_mps = 0.0f;

    // 限制调试阶段的目标速度
    if (left_mps > CONTROL_TARGET_MAX_MPS)
        left_mps = CONTROL_TARGET_MAX_MPS;

    if (right_mps > CONTROL_TARGET_MAX_MPS)
        right_mps = CONTROL_TARGET_MAX_MPS;

    // 短暂关闭中断，成对修改左右目标
    irq_token = interrupt_global_disable();

    left_pid.target_mps = left_mps;
    right_pid.target_mps = right_mps;

    interrupt_global_enable(irq_token);
}

// 从主循环调用，撤销目标并关闭驱动输出
void control_stop(void)
{
    uint32 irq_token;

    irq_token = interrupt_global_disable();

    left_pid.target_mps = 0.0f;
    right_pid.target_mps = 0.0f;

    left_pid.integral = 0.0f;
    right_pid.integral = 0.0f;

    left_pid.last_error = 0.0f;
    right_pid.last_error = 0.0f;

    left_pid.output_pwm = 0.0f;
    right_pid.output_pwm = 0.0f;

    car_motor_stop();

    interrupt_global_enable(irq_token);
}
