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

/* Starting points for track tuning. Both motor duties are clamped below 5000. */
#define FLYCAR_BASE_DUTY       2600
#define FLYCAR_TURN_DUTY       2200
#define FLYCAR_STEER_GAIN        22
#define FLYCAR_MIN_CONFIDENCE    20

static vision_context_t vision;

static uint16_t motor_duty(int value)
{
    if (value < 0) return 0;
    if (value > 5000) return 5000;
    return (uint16_t)value;
}

static void drive_from_vision(const vision_result_t *r)
{
    int base, correction;
    if (r->feature == VISION_LOST || r->confidence < FLYCAR_MIN_CONFIDENCE)
    {
        car_motor_stop();
        return;
    }
    base = (r->feature == VISION_TURN_LEFT || r->feature == VISION_TURN_RIGHT ||
            r->feature == VISION_RING_LEFT || r->feature == VISION_RING_RIGHT) ?
           FLYCAR_TURN_DUTY : FLYCAR_BASE_DUTY;
    correction = r->error * FLYCAR_STEER_GAIN;
    /* Positive image error means the line lies to the right. */
    car_motor_set(motor_duty(base + correction),
                  motor_duty(base - correction));
}

int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);
    debug_init();
    car_motor_init();
    car_motor_stop();
    mt9v03x_init();
    tft180_init();
    car_encoder_init();
    vision_init(&vision);

    tft180_show_string(0, 82, "L:");
    tft180_show_string(0, 98, "R:");
    tft180_show_string(0, 114, "E:");

    while (1)
    {
        if (!mt9v03x_finish_flag) continue;
        mt9v03x_finish_flag = 0;
        vision_process(&vision, &mt9v03x_image[0][0],
                       MT9V03X_W, MT9V03X_H, MT9V03X_W);
        drive_from_vision(&vision.result);

        /* Display only on a new frame; the image also supports on-car debugging. */
        tft180_show_gray_image(0, 0, mt9v03x_image[0],
                               MT9V03X_W, MT9V03X_H, 128, 80, 0);
        tft180_show_int(32, 82, car_encoder_get_left(), 5);
        tft180_show_int(32, 98, car_encoder_get_right(), 5);
        tft180_show_int(32, 114, vision.result.error, 5);
    }
}
