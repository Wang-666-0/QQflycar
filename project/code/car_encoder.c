#include "car_encoder.h"

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