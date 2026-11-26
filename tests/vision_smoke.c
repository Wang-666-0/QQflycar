#include "../project/code/vision.h"

#include <stdio.h>
#include <string.h>

#define W 188
#define H 120

static uint8_t frame[H][W];
static vision_context_t vision;

static void clear_frame(void)
{
    memset(frame, 35, sizeof(frame));
    vision_init(&vision);
}

static void empty_frame(void)
{
    memset(frame, 35, sizeof(frame));
}

static void rotate_frame_180(void)
{
    int i;
    for (i = 0; i < W * H / 2; ++i)
    {
        uint8_t t = (&frame[0][0])[i];
        (&frame[0][0])[i] = (&frame[0][0])[W * H - 1 - i];
        (&frame[0][0])[W * H - 1 - i] = t;
    }
}

static void plot(int x, int y, int radius)
{
    int dx, dy;
    for (dy = -radius; dy <= radius; ++dy)
        for (dx = -radius; dx <= radius; ++dx)
            if (x + dx >= 0 && x + dx < W && y + dy >= 0 && y + dy < H)
                frame[y + dy][x + dx] = 220;
}

static void line(int x0, int y0, int x1, int y1, int radius)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    for (;;)
    {
        int e2;
        plot(x0, y0, radius);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

static int check(const char *name, vision_feature_t expected, int sign)
{
    const vision_result_t *r;
    vision_process(&vision, &frame[0][0], W, H, W);
    r = &vision.result;
    printf("%-12s feature=%-11s error=%4d heading=%4d curve=%4d preview=%3u confidence=%3u threshold=%3u\n",
           name, vision_feature_name(r->feature), r->error,
           r->heading, r->curvature, (unsigned)r->preview_percent,
           (unsigned)r->confidence, (unsigned)r->threshold);
    if (r->feature != expected || (sign < 0 && r->error >= -5) ||
        (sign > 0 && r->error <= 5) || (sign == 0 &&
         (r->error < -5 || r->error > 5))) return 1;
    return 0;
}

int main(void)
{
    int failed = 0, i, short_error, long_error;
    const int8_t right_route[] = {1};
    clear_frame();
    line(94, 0, 94, 119, 3);
    failed += check("straight", VISION_STRAIGHT, 0);

    clear_frame();
    line(94, 0, 94, 119, 3);
    for (i = 0; i < W * H; ++i)
        (&frame[0][0])[i] = (uint8_t)(255 - (&frame[0][0])[i]);
    vision_set_line_polarity(&vision, VISION_LINE_DARK);
    failed += check("dark line", VISION_STRAIGHT, 0);

    clear_frame();
    line(94, 119, 140, 0, 3);
    failed += check("bend", VISION_BEND, 1);

    /* 赛道视频中的折角含一段 45 度过渡线，而非单个横向直角白条。 */
    clear_frame();
    line(94, 119, 94, 75, 3);
    line(94, 75, 135, 35, 3);
    line(135, 35, 135, 0, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    printf("chamfer      feature=%s error=%d confidence=%u\n",
           vision_feature_name(vision.result.feature), vision.result.error,
           (unsigned)vision.result.confidence);
    if (vision.result.feature == VISION_LOST || vision.result.error <= 5)
        ++failed;

    clear_frame();
    line(94, 119, 94, 60, 3);
    line(94, 60, 172, 60, 3);
    failed += check("right angle", VISION_TURN_RIGHT, 1);
    if (vision.result.confidence < 40) ++failed;

    clear_frame();
    line(94, 119, 94, 60, 3);
    line(94, 60, 16, 60, 3);
    failed += check("left angle", VISION_TURN_LEFT, -1);

    /* 上方残留一小段白线时，单侧横臂仍应优先判为直角。 */
    clear_frame();
    line(94, 119, 94, 60, 3);
    line(94, 60, 172, 60, 3);
    line(94, 57, 94, 38, 3);
    failed += check("turn not cross", VISION_TURN_RIGHT, 1);

    clear_frame();
    line(94, 119, 94, 60, 3);
    line(94, 60, 172, 60, 3);
    rotate_frame_180();
    vision_set_rotate_180(&vision, 1);
    failed += check("rotated turn", VISION_TURN_RIGHT, 1);

    clear_frame();
    line(94, 0, 94, 119, 3);
    line(3, 60, 184, 60, 3);
    failed += check("cross", VISION_CROSS, 0);

    clear_frame();
    line(94, 0, 94, 119, 3);
    for (i = 0; i < 90; ++i)
        frame[(i * 37) % H][(i * 53) % W] = 255;
    failed += check("noise", VISION_STRAIGHT, 0);

    clear_frame();
    failed += check("lost", VISION_LOST, 0);

    clear_frame();
    line(94, 119, 94, 70, 3);
    line(94, 70, 54, 8, 3);
    line(94, 70, 134, 8, 3);
    failed += check("fork", VISION_FORK, -1);

    clear_frame();
    line(94, 119, 94, 70, 3);
    line(94, 70, 54, 8, 3);
    line(94, 70, 134, 8, 3);
    vision_set_fork_route(&vision, right_route, 1);
    failed += check("fork route", VISION_FORK, 1);

    clear_frame();
    line(94, 0, 94, 119, 3);
    line(94, 80, 140, 65, 3);
    line(140, 65, 140, 40, 3);
    line(140, 40, 94, 25, 3);
    /* 连续两帧才确认环岛入口，避免单帧平行线把追线拉偏。 */
    vision_process(&vision, &frame[0][0], W, H, W);
    failed += check("ring hint", VISION_RING_RIGHT, 1);

    clear_frame();
    line(94, 0, 94, 119, 3);
    line(117, 60, 121, 60, 1);
    line(137, 60, 141, 60, 1);
    line(157, 60, 161, 60, 1);
    failed += check("side specks", VISION_STRAIGHT, 0);

    clear_frame();
    line(94, 0, 94, 119, 3);
    line(52, 30, 52, 90, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    printf("parallel     center=%d error=%d feature=%s\n",
           vision.result.center[60], vision.result.error,
           vision_feature_name(vision.result.feature));
    if (!vision.result.valid[60] || vision.result.center[60] < 85 ||
        vision.result.center[60] > 103) ++failed;

    clear_frame();
    line(94, 119, 140, 0, 3);
    vision_set_preview(&vision, 20);
    vision_process(&vision, &frame[0][0], W, H, W);
    short_error = vision.result.error;
    clear_frame();
    line(94, 119, 140, 0, 3);
    vision_set_preview(&vision, 80);
    vision_process(&vision, &frame[0][0], W, H, W);
    long_error = vision.result.error;
    printf("preview      short=%d long=%d\n", short_error, long_error);
    if (long_error <= short_error + 5) ++failed;

    clear_frame();
    line(94, 119, 94, 85, 3);
    line(94, 85, 100, 60, 3);
    line(100, 60, 150, 0, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    printf("curved path  heading=%d curvature=%d preview=%u\n",
           vision.result.heading, vision.result.curvature,
           (unsigned)vision.result.preview_percent);
    if (vision.result.curvature < 6) ++failed;

    clear_frame();
    line(165, 0, 165, 119, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    memset(frame, 35, sizeof(frame));
    line(40, 0, 40, 119, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    printf("reacquire    feature=%s error=%d confidence=%u\n",
           vision_feature_name(vision.result.feature), vision.result.error,
           (unsigned)vision.result.confidence);
    if (vision.result.feature == VISION_LOST || vision.result.error >= -20 ||
        vision.result.confidence < 80) ++failed;

    /* 同一个岔路按跨帧通过次数选择：第一圈左、第二圈右。 */
    clear_frame();
    {
        const int8_t two_laps[] = {-1, 1};
        vision_set_fork_route(&vision, two_laps, 2);
    }
    for (i = 0; i < 2; ++i)
    {
        int j;
        empty_frame();
        line(94, 119, 94, 70, 3);
        line(94, 70, 54, 8, 3);
        line(94, 70, 134, 8, 3);
        if (check(i ? "lap 2 right" : "lap 1 left", VISION_FORK,
                  i ? 1 : -1)) ++failed;
        for (j = 0; j < 4; ++j)
        {
            empty_frame();
            line(94, 0, 94, 119, 3);
            vision_process(&vision, &frame[0][0], W, H, W);
        }
        if (vision.fork_passes != i + 1 || vision.lap_count != i + 1)
            ++failed;
    }

    /* 左进右出：左侧入口入环，绕行足够距离后只从右侧支线退出。 */
    clear_frame();
    for (i = 0; i < 2; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        line(94, 80, 48, 65, 3);
        line(48, 65, 48, 40, 3);
        line(48, 40, 94, 25, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    if (vision.ring_phase != VISION_RING_ENTER) ++failed;
    for (i = 0; i < 5; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    if (vision.ring_phase != VISION_RING_CIRCLE) ++failed;
    for (i = 0; i < 8; ++i)
    {
        int j;
        empty_frame();
        line(94, 119, 140, 0, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
        for (j = 0; j < 3; ++j)
        {
            empty_frame();
            line(94, 0, 94, 119, 3);
            vision_process(&vision, &frame[0][0], W, H, W);
        }
        if (i == 5 && vision.ring_phase != VISION_RING_CIRCLE) ++failed;
    }
    if (vision.ring_corner_count != 8 ||
        vision.ring_phase != VISION_RING_EXIT_READY) ++failed;
    /* 待出口时再次看到左入口，仍应留在环内。 */
    empty_frame();
    line(94, 0, 94, 119, 3);
    line(94, 80, 48, 65, 3);
    line(48, 65, 48, 40, 3);
    line(48, 40, 94, 25, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.ring_exit_seen || vision.result.feature == VISION_RING_EXIT)
        ++failed;
    empty_frame();
    line(94, 0, 94, 119, 3);
    line(94, 80, 140, 65, 3);
    line(140, 65, 140, 40, 3);
    line(140, 40, 94, 25, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.result.feature != VISION_RING_EXIT) ++failed;
    for (i = 0; i < 5; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    if (vision.ring_phase != VISION_RING_COOLDOWN) ++failed;

    /* 低机位可能漏数图像折角：四个以上折角后再看见左入口，
       应提前等待右出口；确认右支线后目标必须向右偏。 */
    clear_frame();
    for (i = 0; i < 2; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        line(94, 80, 48, 65, 3);
        line(48, 65, 48, 40, 3);
        line(48, 40, 94, 25, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    for (i = 0; i < 5; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    for (i = 0; i < 4; ++i)
    {
        int j;
        empty_frame();
        line(94, 119, 140, 0, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
        for (j = 0; j < 3; ++j)
        {
            empty_frame();
            line(94, 0, 94, 119, 3);
            vision_process(&vision, &frame[0][0], W, H, W);
        }
    }
    empty_frame();
    line(94, 0, 94, 119, 3);
    line(94, 80, 48, 65, 3);
    line(48, 65, 48, 40, 3);
    line(48, 40, 94, 25, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.ring_phase != VISION_RING_EXIT_READY ||
        !vision.ring_entry_revisited) ++failed;
    empty_frame();
    line(94, 0, 94, 119, 3);
    line(94, 80, 140, 65, 3);
    line(140, 65, 140, 40, 3);
    line(140, 40, 94, 25, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.result.feature != VISION_RING_EXIT ||
        vision.result.error <= 5) ++failed;

    /* 16 位编码器从 32760 回绕到 -32760，实际只前进 16 格。 */
    clear_frame();
    vision_update_odometry(&vision, 32760, 32760);
    vision_update_odometry(&vision, -32760, -32760);
    if (vision.distance_counts != 16) ++failed;

    /* 标定距离到 1.3 圈后进入待出口阶段，下一岔路才是右出口。 */
    clear_frame();
    vision_set_ring_perimeter(&vision, 100);
    vision_update_odometry(&vision, 0, 0);
    for (i = 0; i < 2; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        line(94, 80, 48, 65, 3);
        line(48, 65, 48, 40, 3);
        line(48, 40, 94, 25, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    for (i = 0; i < 5; ++i)
    {
        empty_frame();
        line(94, 0, 94, 119, 3);
        vision_process(&vision, &frame[0][0], W, H, W);
    }
    vision_update_odometry(&vision, 129, 129);
    empty_frame();
    line(94, 0, 94, 119, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.ring_phase != VISION_RING_CIRCLE) ++failed;
    vision_update_odometry(&vision, 130, 130);
    empty_frame();
    line(94, 0, 94, 119, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.ring_phase != VISION_RING_EXIT_READY) ++failed;

    /* 找不到出口的超时应锁定停车，不能下一帧又继续绕圈。 */
    clear_frame();
    vision.ring_phase = VISION_RING_CIRCLE;
    vision.ring_frames = 500;
    line(94, 0, 94, 119, 3);
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.ring_phase != VISION_RING_ABORT ||
        vision.result.feature != VISION_LOST) ++failed;
    vision_process(&vision, &frame[0][0], W, H, W);
    if (vision.result.feature != VISION_LOST) ++failed;

    if (failed) fprintf(stderr, "%d smoke cases failed\n", failed);
    return failed ? 1 : 0;
}
