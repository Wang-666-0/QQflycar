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
    clear_frame();
    line(94, 0, 94, 119, 3);
    failed += check("straight", VISION_STRAIGHT, 0);

    clear_frame();
    line(94, 119, 140, 0, 3);
    failed += check("bend", VISION_BEND, 1);

    clear_frame();
    line(94, 119, 94, 60, 3);
    line(94, 60, 172, 60, 3);
    failed += check("right angle", VISION_TURN_RIGHT, 1);

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
    line(94, 0, 94, 119, 3);
    line(94, 80, 140, 65, 3);
    line(140, 65, 140, 40, 3);
    line(140, 40, 94, 25, 3);
    failed += check("ring hint", VISION_RING_RIGHT, 1);

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

    if (failed) fprintf(stderr, "%d smoke cases failed\n", failed);
    return failed ? 1 : 0;
}
