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

/* 实车调参起点：基础占空比、转弯占空比、转向增益与最低识别置信度。 */
#define FLYCAR_BASE_DUTY       2300
#define FLYCAR_BEND_DUTY       1900
#define FLYCAR_TURN_DUTY       1700
#define FLYCAR_STEER_GAIN        14
#define FLYCAR_MAX_CORRECTION  1000
#define FLYCAR_CORRECTION_STEP  350
#define FLYCAR_TURN_STEP        700
#define FLYCAR_MIN_CONFIDENCE    20
#define FLYCAR_PREVIEW_PERCENT   60
#define FLYCAR_DISPLAY_DIV        4
/* 实车若为浅底深线，改成 VISION_LINE_DARK；图纸颜色不能代替实拍确认。 */
#define FLYCAR_LINE_POLARITY VISION_LINE_BRIGHT

static vision_context_t vision;
static int last_correction;

static int limit_correction(int requested, int step)
{
    if (requested > FLYCAR_MAX_CORRECTION) requested = FLYCAR_MAX_CORRECTION;
    if (requested < -FLYCAR_MAX_CORRECTION) requested = -FLYCAR_MAX_CORRECTION;
    /* 偏差换向时先清除旧方向，避免限速期间仍向反方向打轮。 */
    if ((requested > 0 && last_correction < 0) ||
        (requested < 0 && last_correction > 0))
        last_correction = 0;
    if (requested > last_correction + step)
        requested = last_correction + step;
    if (requested < last_correction - step)
        requested = last_correction - step;
    last_correction = requested;
    return requested;
}

static uint16_t motor_duty(int value)
{
    /* 电机接口的有效占空比限制在 0～5000，避免校正量把输出推到范围外。 */
    if (value < 0) return 0;
    if (value > 5000) return 5000;
    return (uint16_t)value;
}

static void drive_from_vision(const vision_result_t *r)
{
    int base, correction, sharp_turn;
    if (r->feature == VISION_LOST || r->confidence < FLYCAR_MIN_CONFIDENCE)
    {
        /* 丢线或识别不可靠时先停车，避免单核程序继续按旧方向行驶。 */
        last_correction = 0;
        car_motor_stop();
        return;
    }
    sharp_turn = r->feature == VISION_TURN_LEFT ||
                 r->feature == VISION_TURN_RIGHT ||
                 r->feature == VISION_RING_LEFT ||
                 r->feature == VISION_RING_RIGHT ||
                 r->feature == VISION_FORK;
    if (sharp_turn)
        base = FLYCAR_TURN_DUTY;
    else if (r->feature == VISION_BEND || r->feature == VISION_CROSS)
        base = FLYCAR_BEND_DUTY;
    else
        base = FLYCAR_BASE_DUTY;
    /* 近中心的小幅波动不转向；每帧限制校正量变化，避免左右轮占空比猛跳。 */
    correction = (r->error >= -2 && r->error <= 2) ? 0 :
                 r->error * FLYCAR_STEER_GAIN;
    correction = limit_correction(correction, sharp_turn ?
                                  FLYCAR_TURN_STEP : FLYCAR_CORRECTION_STEP);
    /* 正偏差表示白线在画面右侧：左轮加速、右轮减速，使车向右转。 */
    car_motor_set(motor_duty(base + correction),
                  motor_duty(base - correction));
}

int main(void)
{
    uint8 display_counter = FLYCAR_DISPLAY_DIV - 1;
    /* MounRiver 工程只编译这个 main；原始 user/src/main.c 在 .cproject 中排除。 */
    clock_init(SYSTEM_CLOCK_120M);
    debug_init();
    car_motor_init();
    car_motor_stop();
    mt9v03x_init();
    tft180_init();
    car_encoder_init();
    vision_init(&vision);
    vision_set_line_polarity(&vision, FLYCAR_LINE_POLARITY);
    vision_set_preview(&vision, FLYCAR_PREVIEW_PERCENT);

    tft180_show_string(0, 80, "L:");
    tft180_show_string(0, 96, "R:");
    tft180_show_string(0, 112, "E:");
    tft180_show_string(80, 80, "F:");
    tft180_show_string(80, 96, "C:");
    tft180_show_string(80, 112, "T:");

    while (1)
    {
        /* 只在摄像头完成一帧后处理；主循环顺序执行，没有额外核或线程。 */
        if (!mt9v03x_finish_flag) continue;
        mt9v03x_finish_flag = 0;
        vision_process(&vision, &mt9v03x_image[0][0],
                       MT9V03X_W, MT9V03X_H, MT9V03X_W);
        drive_from_vision(&vision.result);

        /* TFT 走 SPI，逐帧写入 128×80 像素会拖慢下一帧控制；仅每四帧刷新。 */
        if (++display_counter >= FLYCAR_DISPLAY_DIV)
        {
            display_counter = 0;
            tft180_show_gray_image(0, 0, mt9v03x_image[0],
                                   MT9V03X_W, MT9V03X_H, 128, 80, 0);
            tft180_show_int(32, 80, car_encoder_get_left(), 5);
            tft180_show_int(32, 96, car_encoder_get_right(), 5);
            tft180_show_int(32, 112, vision.result.error, 5);
            /* F=路况枚举(0为丢线)，C=有效行百分比，T=二值化阈值。 */
            tft180_show_int(104, 80, vision.result.feature, 2);
            tft180_show_int(104, 96, vision.result.confidence, 3);
            tft180_show_int(104, 112, vision.result.threshold, 3);
            /* 显示期间可能已完成新帧；丢弃旧标志，等下一次完整帧再追线。 */
            mt9v03x_finish_flag = 0;
        }
    }
}
