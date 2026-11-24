/*********************************************************************************************************************
* CH32V307VCT6 Opensourec Library 即（CH32V307VCT6 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是CH32V307VCT6 开源库的一部分
*
* CH32V307VCT6 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MounRiver Studio V1.8.1
* 适用平台          CH32V307VCT6
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期                                      作者                             备注
* 2022-09-15        大W            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "car_encoder.h"
#include "car_motor.h"
#include "vision.h"
#include "control.h"

// 基础速度和转向速度
#define FLYCAR_BASE_SPEED_MPS    0.35f
#define FLYCAR_TURN_SPEED_MPS    0.25f

// 每偏移一个像素，改变多少左右轮目标速度
#define FLYCAR_STEER_GAIN        0.004f

// 左右轮相对基础速度的最大修正量
#define FLYCAR_MAX_CORRECTION    0.40f

// 视觉处理的前瞻比例，越大越看远处，越小越看近处
#define FLYCAR_MIN_CONFIDENCE    20
#define FLYCAR_PREVIEW_PERCENT   60

static vision_context_t vision;                     // 视觉处理上下文

static void drive_from_vision(const vision_result_t *r)// 根据视觉结果计算左右电机占空比并输出 PWM
{
    float base;
    float correction;

    if (r->feature == VISION_LOST ||
        r->confidence < FLYCAR_MIN_CONFIDENCE)
    {
        control_stop();
        return;
    }

    if (r->feature == VISION_TURN_LEFT ||
        r->feature == VISION_TURN_RIGHT ||
        r->feature == VISION_RING_LEFT ||
        r->feature == VISION_RING_RIGHT)
    {
        base = FLYCAR_TURN_SPEED_MPS;
    }
    else
    {
        base = FLYCAR_BASE_SPEED_MPS;
    }

    correction = r->error * FLYCAR_STEER_GAIN;

    if (correction > FLYCAR_MAX_CORRECTION)
        correction = FLYCAR_MAX_CORRECTION;

    if (correction < -FLYCAR_MAX_CORRECTION)
        correction = -FLYCAR_MAX_CORRECTION;

    // 线在右侧：左轮更快，右轮更慢
    control_set_target(base + correction,
                       base - correction);
}

int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);
    debug_init();
    car_motor_init();
    control_init();
    gpio_init(B14, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(B12, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(D8,  GPI, GPIO_HIGH, GPI_PULL_UP);
    mt9v03x_init();
    tft180_init();
    car_encoder_init();
    pit_ms_init(TIM6_PIT, 1);// 1 ms 定时器，定时采样编码器速度
    vision_init(&vision);
    vision_set_preview(&vision, FLYCAR_PREVIEW_PERCENT);

    tft180_show_string(0, 82, "L m/s:");
    tft180_show_string(0, 98, "R m/s:");
    tft180_show_string(0, 114, "E:");

    uint8 motor_enabled = 0;  // 上电默认关闭电机使能
    while (1)
    {
        uint8 key_code;
        key_code = (gpio_get_level(B14) << 2)
                | (gpio_get_level(B12) << 1)
                |  gpio_get_level(D8);
        // KEY2：立即关闭使能，并停止电机
        if (key_code == 0)
        {
            motor_enabled = 0;
            control_stop();
        }
        // KEY1：消抖确认后开启使能
        else if (motor_enabled == 0 && key_code == 3)
        {
            system_delay_ms(20);

            key_code = (gpio_get_level(B14) << 2)
                    | (gpio_get_level(B12) << 1)
                    |  gpio_get_level(D8);

            if (key_code == 3)
            {
                motor_enabled = 1;
            }
        }

        if (!mt9v03x_finish_flag) continue;                 // 等待摄像头采集完成
        mt9v03x_finish_flag = 0;
        vision_process(&vision, &mt9v03x_image[0][0],
                       MT9V03X_W, MT9V03X_H, MT9V03X_W);
        if (motor_enabled)
        {
            drive_from_vision(&vision.result);
        }
        else
        {
            control_stop();
        }

        // 显示图像和调试信息
        tft180_show_gray_image(0, 0, mt9v03x_image[0],MT9V03X_W, MT9V03X_H, 128, 80,0);
        tft180_show_float(56, 82, car_speed_left_mps, 2, 3);
        tft180_show_float(56, 98, car_speed_right_mps, 2, 3);
        tft180_show_int(32, 114, vision.result.error, 5);
        tft180_show_string(0, 130, motor_enabled ? "EN:1" : "EN:0");

        tft180_show_string(40, 130, "F:");
        tft180_show_int(56, 130, vision.result.feature, 1);

        tft180_show_string(80, 130, "C:");
        tft180_show_int(96, 130, vision.result.confidence, 3);
    }
}
