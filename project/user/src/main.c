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

int main(void)
{
    // 初始化主控时钟和调试接口
    clock_init(SYSTEM_CLOCK_120M);
    debug_init();

    // 初始化电机，初始占空比为 0
    car_motor_init();
    car_motor_stop();

    // 左右电机持续输出约 30% 的 PWM
    car_motor_set(3000, 3000);

    // 初始化摄像头和屏幕
    mt9v03x_init();
    tft180_init();

    // 初始化左右编码器
    car_encoder_init();

    // 在屏幕下半部分显示固定文字
    tft180_show_string(0, 88, "L:");
    tft180_show_string(0, 112, "R:");

    while (1)
    {
        // 屏幕上半部分显示摄像头画面
        tft180_show_gray_image(
            0,
            0,
            mt9v03x_image[0],
            MT9V03X_W,
            MT9V03X_H,
            128,
            80,
            0
        );

        // 显示左编码器计数
        tft180_show_int(32, 88, car_encoder_get_left(), 5);

        // 显示右编码器计数
        tft180_show_int(32, 112, car_encoder_get_right(), 5);
    }
}

