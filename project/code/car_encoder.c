#include "car_encoder.h"
// 装好硅胶胎后的整轮外径，暂按模型的 31 mm 估算
#define WHEEL_DIAMETER_MM 31.0f

// 适用于 1 ms 采样，计算出的速度单位为 m/s
#define ENCODER_SPEED_FACTOR \
    (3.1415926f * WHEEL_DIAMETER_MM / 8192.0f)

volatile float car_speed_left_mps = 0.0f;
volatile float car_speed_right_mps = 0.0f;

static uint16 previous_left_count = 0;
static uint16 previous_right_count = 0;
/*
 * 初始化左右编码器
 *
 * 左编码器：
 * PA0 -> TIM2 CH1
 * PA1 -> TIM2 CH2
 *
 * 右编码器：
 * PD12 -> TIM4 CH1
 * PD13 -> TIM4 CH2
 */
void car_encoder_init(void)
{
    // 提前打开 AFIO 时钟，供 TIM4 引脚重映射使用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // 初始化左编码器：TIM2，PA0 和 PA1
    encoder_quad_init(
        TIM2_ENCOEDER,
        TIM2_ENCOEDER_MAP0_CH1_A0,
        TIM2_ENCOEDER_MAP0_CH2_A1
    );

    // 初始化右编码器：TIM4，PD12 和 PD13
    encoder_quad_init(
        TIM4_ENCOEDER,
        TIM4_ENCOEDER_MAP1_CH1_D12,
        TIM4_ENCOEDER_MAP1_CH2_D13
    );

    // 初始化完成后将计数清零
    encoder_clear_count(TIM2_ENCOEDER);
    encoder_clear_count(TIM4_ENCOEDER);

    previous_left_count = 0;
    previous_right_count = 0;

    car_speed_left_mps = 0.0f;
    car_speed_right_mps = 0.0f;
}

/*
 * 获取左编码器当前计数
 */
int32 car_encoder_get_left(void)
{
    return -(int32)encoder_get_count(TIM2_ENCOEDER);
}

/*
 * 获取右编码器当前计数
 */
int32 car_encoder_get_right(void)
{
    return (int32)encoder_get_count(TIM4_ENCOEDER);
}
/*
 * 1 ms 定时采样，计算左右轮速度
 */
void car_encoder_sample_1ms(void)
{
    uint16 current_left =
        (uint16)encoder_get_count(TIM2_ENCOEDER);

    uint16 current_right =
        (uint16)encoder_get_count(TIM4_ENCOEDER);

    // 按 16 位求差，处理计数器回绕，再解释为带方向的增量
    int16 delta_left =
        (int16)(uint16)(current_left - previous_left_count);

    int16 delta_right =
        (int16)(uint16)(current_right - previous_right_count);

    previous_left_count = current_left;
    previous_right_count = current_right;

    car_speed_left_mps =
        -(float)delta_left * ENCODER_SPEED_FACTOR;

    car_speed_right_mps =
        (float)delta_right * ENCODER_SPEED_FACTOR;
}