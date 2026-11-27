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
#include "control.h"

#define PID_TEST_TARGET_MPS 0.50f

int main(void)
{
    uint8 start_ready = 0;
    uint8 motor_running = 0;

    clock_init(SYSTEM_CLOCK_120M);
    debug_init();

    car_motor_init();
    control_init();
    tft180_init();
    car_encoder_init();

    gpio_init(B14, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(B12, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(D8,  GPI, GPIO_HIGH, GPI_PULL_UP);

    tft180_show_string(0, 72, "KEY:");
    tft180_show_string(0, 0, "CTRL TEST");
    tft180_show_string(0, 24, "L:");
    tft180_show_string(0, 48, "R:");

    // TIM6 每 1 ms 先采样编码器，再更新速度 PID。
    pit_ms_init(TIM6_PIT, 1);

    while (1)
    {
        uint8 key_code;
        uint8 run_requested;
        uint32 irq_token;
        float left_speed_mps;
        float right_speed_mps;

        key_code = (gpio_get_level(B14) << 2)
                 | (gpio_get_level(B12) << 1)
                 |  gpio_get_level(D8);

        // 上电先检测到按键释放，再允许测试。
        if (key_code == 7)
        {
            start_ready = 1;
        }

        run_requested = (start_ready && key_code == 3);
        if (run_requested != motor_running)
        {
            if (run_requested)
            {
                control_set_target(PID_TEST_TARGET_MPS, PID_TEST_TARGET_MPS);
            }
            else
            {
                control_stop();
            }
            motor_running = run_requested;
        }

        // 取同一控制周期的两轮速度，供屏幕和串口共用。
        irq_token = interrupt_global_disable();
        left_speed_mps = car_speed_left_mps;
        right_speed_mps = car_speed_right_mps;
        interrupt_global_enable(irq_token);

        tft180_show_int(40, 72, key_code, 1);
        tft180_show_float(24, 24, left_speed_mps, 2, 3);
        tft180_show_float(24, 48, right_speed_mps, 2, 3);

        // FireWater：左轮实测、右轮实测、共同目标，单位 m/s。
        printf("%.3f,%.3f,%.3f\r\n",
               (double)left_speed_mps,
               (double)right_speed_mps,
               motor_running ? (double)PID_TEST_TARGET_MPS : 0.0);
        fflush(stdout);

        system_delay_ms(10);
    }
}
