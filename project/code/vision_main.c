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
#define FLYCAR_BASE_SPEED_MPS    0.50f                              // 基础速度，单位 m/s
#define FLYCAR_TURN_SPEED_MPS    0.39f                              // 转弯时的基础速度，单位 m/s
#define FLYCAR_BEND_SPEED_MPS    0.50f                              // 弯道时的基础速度，单位 m/s
#define FLYCAR_RING_SPEED_MPS    0.50f                              // 环岛时的基础速度，单位 m/s

// 每偏移一个像素，改变多少左右轮目标速度
#define FLYCAR_STEER_GAIN        0.0045f                            // 视觉偏差每像素对应的左右轮速度修正量，单位 m/s
#define FLYCAR_STEER_DEADBAND_PX 2                                  // 视觉偏差在 ±2 像素内不修正，避免抖动
#define FLYCAR_STEER_SLEW_MPS    0.05f                              // 视觉偏差修正量的最大变化率，单位 m/s
#define FLYCAR_BEND_SLEW_MPS     0.12f                              // 弯道时的视觉偏差修正量最大变化率，单位 m/s
/* 直角白线短暂钻到车底时，限时低速沿已确认的方向续转。 */
#define FLYCAR_CORNER_RECENT_FRAMES    4
#define FLYCAR_CORNER_RECOVERY_FRAMES 12
#define FLYCAR_CORNER_RECOVERY_SPEED  0.18f                          // 直角丢线续转时的速度，单位 m/s
#define FLYCAR_CORNER_RECOVERY_STEER  0.16f                          // 直角丢线续转时的转向修正量，单位 m/s
/* 图纸六边形每边约 300 mm，共约 1.8 m；编码器按 31 mm 轮径、8192 格/圈估算。
   这是初始值，实车应测一圈编码器计数后修改。 */
#define FLYCAR_RING_PERIMETER_COUNTS 151000u

// 左右轮相对基础速度的最大修正量
#define FLYCAR_MAX_CORRECTION    0.50f                              // 左右轮相对基础速度的最大修正量，单位 m/s

// 视觉处理的前瞻比例，越大越看远处，越小越看近处
#define FLYCAR_MIN_CONFIDENCE    20                             // 视觉结果的置信度低于该值时，立即停车
#define FLYCAR_PREVIEW_PERCENT   50                             // 视觉处理的前瞻比例，越大越看远处，越小越看近处
#define FLYCAR_DISPLAY_DIV        4                             // 每四帧刷新一次屏幕，减少单核图像显示造成的追线延迟
#define FLYCAR_CAMERA_ROTATE_180  1                             // 摄像头倒装时将原始帧旋转 180° 后再识别
#define FLYCAR_IMAGE_WIDTH      128                             // 显示摄像头缩略图的宽度
#define FLYCAR_IMAGE_HEIGHT      80                             // 显示摄像头缩略图的高度

/* 实车若为浅底深线，改成 VISION_LINE_DARK；图纸颜色不能代替实拍确认。 */
#define FLYCAR_LINE_POLARITY VISION_LINE_BRIGHT

static vision_context_t vision;                                              // 视觉处理上下文
static uint8 display_image[FLYCAR_IMAGE_HEIGHT][FLYCAR_IMAGE_WIDTH];         // 用于显示摄像头缩略图的缓冲区
static float last_steer_correction = 0.0f;                                   // 上一帧的视觉偏差修正量，单位 m/s
static vision_feature_t last_steer_feature = VISION_LOST;                    // 上一帧的视觉结果特征类型
static int8_t corner_recovery_side = 0;                                      // 直角丢线续转的方向，-1 左侧，+1 右侧，0 无续转
static uint8 corner_recent_frames = 0;                                       // 近几帧确实追到主线，限制稀疏直角的误触发
static uint8 corner_recovery_frames = 0;                                     // 直角丢线续转的帧数，超过上限就停车，不让车盲走到下一个路口
static uint8 corner_sparse_exhausted = 0;                                    // 稀疏直角是否已耗尽，避免连续误触发

/**
 * 清除直角续转状态
 * @param void
 * @return void
 */
static void clear_corner_recovery(void)
{
    corner_recovery_side = 0;
    corner_recent_frames = 0;
    corner_recovery_frames = 0;
}

/* 只旋转屏幕缩略图，不修改摄像头 DMA 使用的原始帧。 */
static void show_camera_image(void)
{
    uint16 x, y;

    for (y = 0; y < FLYCAR_IMAGE_HEIGHT; ++y)
    {
        // 将显示图像的每一行映射到原始图像的对应行
        uint16 source_y = (uint32)y * MT9V03X_H / FLYCAR_IMAGE_HEIGHT;
        if (FLYCAR_CAMERA_ROTATE_180) source_y = MT9V03X_H - 1 - source_y;
        for (x = 0; x < FLYCAR_IMAGE_WIDTH; ++x)
        {
            uint16 source_x = (uint32)x * MT9V03X_W / FLYCAR_IMAGE_WIDTH;
            if (FLYCAR_CAMERA_ROTATE_180) source_x = MT9V03X_W - 1 - source_x;
            display_image[y][x] = mt9v03x_image[source_y][source_x];
        }
    }
    tft180_show_gray_image(0, 0, display_image[0],
                           FLYCAR_IMAGE_WIDTH, FLYCAR_IMAGE_HEIGHT,
                           FLYCAR_IMAGE_WIDTH, FLYCAR_IMAGE_HEIGHT, 0);
}

static void drive_from_vision(const vision_result_t *r)         // 根据视觉结果计算左右电机占空比并输出 PWM
{
    float base;
    float correction;
    int16_t error_px = r->error;
    uint8 corner_now = r->feature == VISION_TURN_LEFT ||
                      r->feature == VISION_TURN_RIGHT;

    /* 可信直角或近期主线后的稀疏单侧横臂可启动短时续转。 */
    if (!r->sparse_corner && r->feature != VISION_LOST &&
        r->confidence >= FLYCAR_MIN_CONFIDENCE)
        corner_sparse_exhausted = 0;
    if (vision.ring_phase != VISION_RING_IDLE) clear_corner_recovery();
    if (corner_now && (r->confidence >= FLYCAR_MIN_CONFIDENCE ||
                       r->sparse_corner) &&
        (!r->sparse_corner || !corner_sparse_exhausted) &&
        vision.ring_phase == VISION_RING_IDLE &&
        ((r->feature == VISION_TURN_LEFT && error_px <= -12) ||
         (r->feature == VISION_TURN_RIGHT && error_px >= 12)))
    {
        int8_t side = r->feature == VISION_TURN_LEFT ? -1 : 1;
        if (!r->sparse_corner || corner_recovery_side != side)
        {
            corner_recovery_side = side;
            corner_recent_frames = FLYCAR_CORNER_RECENT_FRAMES;
            corner_recovery_frames = 0;
        }
    }

    if (r->feature == VISION_LOST || r->sparse_corner ||
        r->confidence < FLYCAR_MIN_CONFIDENCE)
    {
        if (vision.ring_phase == VISION_RING_IDLE &&
            corner_recovery_side && corner_recent_frames &&
            corner_recovery_frames < FLYCAR_CORNER_RECOVERY_FRAMES)
        {
            /* 此时没有可靠的新线位，只沿刚确认的直角方向低速续转。
               连续丢线超过上限就停车，不让车盲走到下一个路口。 */
            correction = corner_recovery_side * FLYCAR_CORNER_RECOVERY_STEER;
            ++corner_recovery_frames;
            last_steer_correction = correction;
            last_steer_feature = corner_recovery_side < 0 ?
                                 VISION_TURN_LEFT : VISION_TURN_RIGHT;
            control_set_target(FLYCAR_CORNER_RECOVERY_SPEED + correction,
                               FLYCAR_CORNER_RECOVERY_SPEED - correction);
            return;
        }
        if (r->sparse_corner) corner_sparse_exhausted = 1;
        clear_corner_recovery();
        last_steer_correction = 0.0f;
        last_steer_feature = VISION_LOST;
        control_stop();
        return;
    }

    if (!corner_now)                                                                // 直角丢线后，近几帧确实追到主线，限制稀疏直角的误触发
    {
        if (r->feature == VISION_STRAIGHT || r->feature == VISION_BEND)             // 直角丢线后，近几帧确实追到主线，限制稀疏直角的误触发
        {
            if (corner_recent_frames) --corner_recent_frames;
            if (!corner_recent_frames) clear_corner_recovery();
        }
        else clear_corner_recovery();                                               // 直角丢线后，近几帧确实追到主线，限制稀疏直角的误触发
    }

    if (vision.last_ring_hint != 0 ||
        vision.ring_phase == VISION_RING_CIRCLE ||
        vision.ring_phase == VISION_RING_EXIT_READY ||
        r->feature == VISION_RING_LEFT ||
        r->feature == VISION_RING_RIGHT ||
        r->feature == VISION_RING_EXIT)
    {
        base = FLYCAR_RING_SPEED_MPS;                                               // 环岛时的基础速度，单位 m/s
    }
    else if (r->feature == VISION_TURN_LEFT ||
             r->feature == VISION_TURN_RIGHT)
    {
        base = FLYCAR_TURN_SPEED_MPS;                                               // 转弯时的基础速度，单位 m/s
    }
    else if (r->feature == VISION_BEND)
    {
        base = FLYCAR_BEND_SPEED_MPS;                                               // 弯道时的基础速度，单位 m/s
    }
    else
    {
        base = FLYCAR_BASE_SPEED_MPS;                                               // 基础速度，单位 m/s
    }

    if (r->feature == VISION_STRAIGHT &&
        error_px >= -FLYCAR_STEER_DEADBAND_PX &&
        error_px <= FLYCAR_STEER_DEADBAND_PX)
        error_px = 0;
    correction = error_px * FLYCAR_STEER_GAIN;// 根据偏差计算左右轮的速度修正量

    if (correction > FLYCAR_MAX_CORRECTION)
        correction = FLYCAR_MAX_CORRECTION;

    if (correction < -FLYCAR_MAX_CORRECTION)
        correction = -FLYCAR_MAX_CORRECTION;

    /* 直道抑制细小摆动；入弯和连续折角允许更快地改变转向。
       弯后回直线也应较快松开转向，避免继续朝旧方向冲出白线。 */
    if (r->feature == VISION_STRAIGHT || r->feature == VISION_BEND)
    {
        float slew = (r->feature == VISION_STRAIGHT &&
                      last_steer_feature == VISION_STRAIGHT) ?
                     FLYCAR_STEER_SLEW_MPS : FLYCAR_BEND_SLEW_MPS;
        if (last_steer_feature != VISION_LOST)
        {
            if (correction > last_steer_correction + slew)
                correction = last_steer_correction + slew;
            if (correction < last_steer_correction - slew)
                correction = last_steer_correction - slew;
        }
    }
    last_steer_correction = correction;
    last_steer_feature = r->feature;

    // 线在右侧：左轮更快，右轮更慢
    control_set_target(base + correction,
                       base - correction);
}

int main(void)
{
    uint8 display_counter = FLYCAR_DISPLAY_DIV - 1;
    /* MounRiver 工程只编译这个 main；原始 user/src/main.c 在 .cproject 中排除。 */
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

    pit_ms_init(TIM6_PIT, 1);                                    // 1 ms 定时器，定时采样编码器速度

    vision_init(&vision);
    vision_set_line_polarity(&vision, FLYCAR_LINE_POLARITY);                    // 视觉识别黑线或白线
    vision_set_rotate_180(&vision, FLYCAR_CAMERA_ROTATE_180);                   // 摄像头倒装时将原始帧旋转 180° 后再识别
    vision_set_preview(&vision, FLYCAR_PREVIEW_PERCENT);                        // 视觉处理的前瞻比例，越大越看远处，越小越看近处
    vision_set_ring_perimeter(&vision, FLYCAR_RING_PERIMETER_COUNTS);          // 用实测行驶距离判断绕够一圈，防止视觉误数拐角提前出环
    /* 岔路首圈跟随连续可见的分支，第二圈自动选择相反方向。 */

    tft180_show_string(0, 80, "L m/s:");                         // 显示左轮速度
    tft180_show_string(0, 96, "R m/s:");                         // 显示右轮速度
    tft180_show_string(0, 112, "E:");                            // 显示视觉结果的偏差值
    tft180_show_string(80, 112, "H:");                           // 显示当前侧支线提示
    tft180_show_string(0, 128, "P:");                            // 显示环岛识别状态
    tft180_show_string(40, 128, "N:");                           // 显示当前圈数
    tft180_show_string(0, 144, "F");                             // 显示视觉结果的特征类型
    tft180_show_string(32, 144, "C");                            // 显示视觉结果的置信度
    tft180_show_string(80, 144, "T");                            // 显示直角丢线续转帧数

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
        vision_update_odometry(&vision,
                               encoder_get_count(TIM2_ENCOEDER),
                               encoder_get_count(TIM4_ENCOEDER));
        vision_process(&vision, &mt9v03x_image[0][0],
                       MT9V03X_W, MT9V03X_H, MT9V03X_W);
        if (motor_enabled)
        {
            drive_from_vision(&vision.result);
        }
        else
        {
            clear_corner_recovery();
            last_steer_correction = 0.0f;
            last_steer_feature = VISION_LOST;
            control_stop();
        }

        /* 竖屏 128x160；每四帧刷新一次，避免拖慢追线。 */
        if (++display_counter >= FLYCAR_DISPLAY_DIV)
        {
            display_counter = 0;
            show_camera_image();
            tft180_show_float(56, 80, car_speed_left_mps, 2, 3);                        // 显示左轮速度
            tft180_show_float(56, 96, car_speed_right_mps, 2, 3);                       // 显示右轮速度
            tft180_show_int(32, 112, vision.result.error, 5);                           // 显示视觉结果的偏差值
            tft180_show_int(96, 112, vision.last_ring_hint, 2);                         // 显示当前侧支线提示
            tft180_show_int(16, 128, vision.ring_phase, 1);                             // 显示环岛识别状态
            tft180_show_int(56, 128, vision.lap_count, 1);                              // 显示已完成的岔路圈次
            tft180_show_string(80, 128, motor_enabled ? "EN:1" : "EN:0");               // 显示电机使能状态
            tft180_show_int(8, 144, vision.result.feature, 1);                          // 显示视觉结果的特征类型
            tft180_show_uint(40, 144, vision.result.confidence, 3);                     // 显示视觉结果的置信度
            tft180_show_uint(96, 144, corner_recovery_frames, 2);                       // 显示直角丢线续转帧数
            mt9v03x_finish_flag = 0;                                                    // 重置摄像头采集完成标志
        }
    }
}
