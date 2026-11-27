#include "control.h"
#include "car_motor.h"
#include "car_encoder.h"
// 初步调试时，限制控制器的最大 PWM
#define CONTROL_PWM_MAX 10000.0f
// 目标速度限制在 0～1 m/s，避免调试阶段过快
#define CONTROL_TARGET_MAX_MPS 1.0f

#define CONTROL_DT_S        0.001f                                                      // 控制周期：1 ms
#define CONTROL_D_TAU_S     0.010f                                                      // D 支路滤波时间常数：10 ms
#define CONTROL_D_ALPHA \
    (CONTROL_DT_S / (CONTROL_D_TAU_S + CONTROL_DT_S))                                   // D 支路滤波系数
// 一个轮子的速度控制器
typedef struct
{
    volatile float target_mps;      // 目标速度，单位 m/s

    float kp;             // 比例系数
    float ki;             // 积分系数
    float kd;             // 微分系数

    volatile float integral;                                // 累计误差
    volatile float last_error;                              // 上一次的速度误差
    volatile float output_pwm;                              // 计算得到的 PWM
    volatile float last_measured_mps;                       // 上一次实测速度
    volatile float d_filtered;                              // 滤波后的速度变化率
    volatile uint8 d_ready;                                 // 是否已有上一拍测量
    uint8 reverse_active;                                   // 是否正在反向驱动
    uint8 near_stop_ms;                                     // 目标速度为零时，连续多少毫秒后才允许反向驱动
} speed_pid_t;

// 左右轮各自保存控制状态
static speed_pid_t left_pid;
static speed_pid_t right_pid;

static void speed_pid_reset_state(speed_pid_t *pid)// 重置控制器状态
{
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output_pwm = 0.0f;

    pid->last_measured_mps = 0.0f;
    pid->d_filtered = 0.0f;
    pid->d_ready = 0;
}

void control_init(void)// 初始化控制器，设置 PID 参数
{
    left_pid.target_mps = 0.0f;

    // 先全部设为 0，后面从 Kp 开始调试
    left_pid.kp = 23000.0f;
    left_pid.ki = 1875.0f;
    left_pid.kd = 10.0f;

    speed_pid_reset_state(&left_pid);

    // 初始值相同，之后两套状态分别更新
    right_pid = left_pid;

    car_motor_stop();
}

/**
 * @brief 计算一个轮子的 P 控制输出
 * @param pid 指向速度 PID 控制器的指针
 * @param measured_mps 实测速度，单位 m/s
 * @param ph_pin 使能引脚
 * @param forward_level 正转电平
 * @return PWM 输出值
 */
static float speed_pid_calculate(
    speed_pid_t *pid, float measured_mps,gpio_pin_enum ph_pin, uint8 forward_level)
{
    float error;                                            // 速度误差
    float p_term;                                           // P 项
    float d_raw;                                            // D 项原始值
    float d_term;                                           // D 项
    float next_integral;                                    // 积分项的下一拍值
    float next_output;                                      // 下一拍输出值
    float output;                                           // 输出值

    float target = pid->target_mps;
    uint8 reverse = target < 0.0f;

    if (target == 0.0f)                                             // 目标速度为 0 时，立即清除积分并关闭输出
    {
        pid->near_stop_ms = 0;
        speed_pid_reset_state(pid);
        return 0.0f;
    }

    if (reverse != pid->reverse_active)                             // 目标方向改变时，立即清除积分并关闭输出
    {
        speed_pid_reset_state(pid);

        if (measured_mps < -0.03f || measured_mps > 0.03f)          // 速度仍然不为零，先反向制动
        {
            pid->near_stop_ms = 0;
        }
        else if (++pid->near_stop_ms >= 2)
        {
            gpio_set_level(ph_pin, forward_level ^ reverse);
            pid->reverse_active = reverse;
            pid->near_stop_ms = 0;
        }
        return 0.0f;
    }

    pid->near_stop_ms = 0;
    if (reverse)
    {
        target = -target;
        measured_mps = -measured_mps;
    }
    error = target - measured_mps;


    p_term = pid->kp * error;

    // 首次启动只记录当前速度，避免产生虚假的微分冲击
    if (pid->d_ready == 0)
    {
        pid->d_filtered = 0.0f;
        pid->d_ready = 1;
    }
    else
    {
        // 对实测速度求微分
        d_raw = (measured_mps - pid->last_measured_mps)
              / CONTROL_DT_S;

        // 对速度变化率进行一阶低通滤波
        pid->d_filtered += CONTROL_D_ALPHA
                         * (d_raw - pid->d_filtered);
    }

    pid->last_measured_mps = measured_mps;

    // 注意负号：速度快速增加时，减小驱动输出
    d_term = -pid->kd * pid->d_filtered;

    next_integral = pid->integral + error * CONTROL_DT_S;

    // 积分防饱和判断也必须包含 D 项
    next_output = p_term
                + pid->ki * next_integral
                + d_term;

    if (!((next_output > CONTROL_PWM_MAX && error > 0.0f)
       || (next_output < 0.0f && error < 0.0f)))
    {
        pid->integral = next_integral;
    }

    output = p_term
           + pid->ki * pid->integral
           + d_term;

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

    left_pwm = speed_pid_calculate(&left_pid, car_speed_left_mps, E11, 1);
    right_pwm = speed_pid_calculate(&right_pid, car_speed_right_mps, E13, 0);

    car_motor_set((uint16)left_pwm, (uint16)right_pwm);
}

// 从主循环调用，设置左右轮目标速度
void control_set_target(float left_mps, float right_mps)
{
    uint32 irq_token;

    // 处理 NaN 值
    if (left_mps != left_mps) left_mps = 0.0f;
    if (right_mps != right_mps) right_mps = 0.0f;

    if (left_mps < -CONTROL_TARGET_MAX_MPS) left_mps = -CONTROL_TARGET_MAX_MPS;
    if (right_mps < -CONTROL_TARGET_MAX_MPS) right_mps = -CONTROL_TARGET_MAX_MPS;

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

    speed_pid_reset_state(&left_pid);
    speed_pid_reset_state(&right_pid);

    car_motor_stop();

    interrupt_global_enable(irq_token);
}
