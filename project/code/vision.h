#ifndef FLYCAR_VISION_H
#define FLYCAR_VISION_H

#include <stdint.h>

/* 摄像头默认帧为 188×120；处理函数也接受不超过此尺寸的小图。 */
#define VISION_MAX_W 188
#define VISION_MAX_H 120
#define VISION_MASK_STRIDE ((VISION_MAX_W + 7) / 8)
#define VISION_MAX_FORK_ROUTE 16

/* 图纸颜色不等于实物颜色：按摄像头实际画面选择亮线或暗线。 */
typedef enum
{
    VISION_LINE_BRIGHT = 0,
    VISION_LINE_DARK
} vision_line_polarity_t;

/* 路况分类：丢线、直道、普通弯道、直角、十字、岔路及环岛进出。 */
typedef enum
{
    VISION_LOST = 0,
    VISION_STRAIGHT,
    VISION_BEND,
    VISION_TURN_LEFT,
    VISION_TURN_RIGHT,
    VISION_CROSS,
    VISION_FORK,
    VISION_RING_LEFT,
    VISION_RING_RIGHT,
    VISION_RING_EXIT
} vision_feature_t;

typedef enum
{
    VISION_RING_IDLE = 0,
    VISION_RING_ENTER,
    VISION_RING_CIRCLE,
    VISION_RING_EXIT_READY,
    VISION_RING_COOLDOWN,
    VISION_RING_ABORT
} vision_ring_phase_t;

/* 单帧输出：逐行白线位置、转向目标与识别可信度。 */
typedef struct
{
    uint8_t mask[VISION_MAX_H][VISION_MASK_STRIDE];
    int16_t left[VISION_MAX_H];
    int16_t right[VISION_MAX_H];
    int16_t center[VISION_MAX_H];
    uint8_t valid[VISION_MAX_H];
    int16_t error;             /* 目标点相对图像中心的像素偏差；正值表示线在右侧 */
    int16_t heading;           /* 远处中心减近处中心，单位为像素 */
    int16_t curvature;         /* 远 - 2×中 + 近；仅为图像内的弯曲趋势 */
    int16_t preview_x;         /* 本帧选出的目标横坐标 */
    uint8_t preview_percent;   /* 常规前瞻 15～90；岔路时直接取远处中心，记为 100 */
    uint8_t threshold;
    uint8_t confidence;        /* 下方 3/4 视野的有效扫描行占比，范围 0～100 */
    uint8_t valid_rows;
    vision_feature_t feature;
} vision_result_t;

/* 跨帧状态：保留上次线位、岔路次数和短时环岛提示。 */
typedef struct
{
    vision_result_t result;
    int16_t last_center;
    int16_t last_error;
    uint8_t fork_passes;       /* 完整通过岔路的次数，用奇偶决定下次选左/右 */
    uint8_t fork_active;
    uint8_t fork_clear_frames;
    int8_t ring_side;          /* -1 左侧支线，+1 右侧支线，0 无环岛提示 */
    uint8_t preview_setting;   /* 用户设定的前瞻比例，限制在 15～90 */
    vision_line_polarity_t line_polarity;
    int8_t fork_route[VISION_MAX_FORK_ROUTE]; /* -1 左，+1 右；按通过岔路的次数索引 */
    uint8_t fork_route_len;     /* 为 0 时沿用原来的左右交替策略 */
    uint8_t rotate_180;        /* 摄像头倒装时将原始帧旋转 180° 后再识别 */
    uint8_t lap_count;         /* 已完成的岔路圈次；上电初始化为 0 */
    vision_ring_phase_t ring_phase;
    uint8_t ring_hint_frames;
    uint8_t ring_clear_frames;
    uint8_t ring_corner_count;
    uint8_t ring_bend_active;
    uint8_t ring_straight_frames;
    uint8_t ring_cooldown_frames;
    uint16_t ring_frames;
    int8_t ring_entry_side;
    uint8_t ring_exit_seen;
    uint32_t distance_counts;  /* 左右编码器增量绝对值的平均累计 */
    uint32_t ring_entry_counts;
    uint32_t ring_perimeter_counts; /* 六边形一圈计数；0 时按转角数判断 */
    int32_t odom_last_left;
    int32_t odom_last_right;
    uint8_t odom_valid;
} vision_context_t;

/* 每次上电或从起点重新出发时初始化一次；其中包含跨帧跟踪状态。 */
void vision_init(vision_context_t *ctx);
/* 调整前瞻比例。数值越大，目标点越偏向图像远处。 */
void vision_set_preview(vision_context_t *ctx, uint8_t percent);
/* 默认亮线；暗线赛道需在初始化后设为 VISION_LINE_DARK。 */
void vision_set_line_polarity(vision_context_t *ctx, vision_line_polarity_t polarity);
void vision_set_rotate_180(vision_context_t *ctx, uint8_t enabled);
/* 六边形一圈的编码器计数，需按实车测量；0 时用转角数退化判断。 */
void vision_set_ring_perimeter(vision_context_t *ctx, uint32_t counts);
void vision_update_odometry(vision_context_t *ctx, int32_t left, int32_t right);
/* 预置最多 16 次岔路选择；每项 -1 为左、+1 为右，传 NULL 或 count=0 可清除。 */
void vision_set_fork_route(vision_context_t *ctx, const int8_t *sides, uint8_t count);
/* 输入一帧 8 位灰度图；stride 是相邻两行起点间的字节数。 */
void vision_process(vision_context_t *ctx, const uint8_t *gray,
                    uint16_t width, uint16_t height, uint16_t stride);
/* 读取二值化结果：1 为白线像素，0 为背景。 */
uint8_t vision_binary_get(const vision_result_t *result, uint16_t x, uint16_t y);
/* 将识别结果转成英文短名称，便于调试输出。 */
const char *vision_feature_name(vision_feature_t feature);

#endif
