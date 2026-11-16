#include "vision.h"

#include <string.h>

/* Tuning constants for a white 20 mm guide line on dark/blue cloth. */
#define MIN_RUN_PIXELS       2
#define NORMAL_RUN_MAX      38
#define WIDE_RUN_MIN        44
#define BRANCH_GAP_MIN      10
#define BRANCH_ROWS_MIN      5
#define FORK_CLEAR_FRAMES    4
#define RING_HOLD_FRAMES     8

typedef struct
{
    int16_t left;
    int16_t right;
    int16_t center;
    int16_t width;
} run_t;

static int abs_i(int x) { return x < 0 ? -x : x; }
static int clamp_i(int x, int lo, int hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

static void mask_set(vision_result_t *r, int x, int y)
{
    r->mask[y][x >> 3] |= (uint8_t)(1u << (x & 7));
}

uint8_t vision_binary_get(const vision_result_t *r, uint16_t x, uint16_t y)
{
    if (!r || x >= VISION_MAX_W || y >= VISION_MAX_H) return 0;
    return (uint8_t)((r->mask[y][x >> 3] >> (x & 7)) & 1u);
}

static int white(const vision_result_t *r, int x, int y)
{
    return (r->mask[y][x >> 3] >> (x & 7)) & 1u;
}

/* Otsu is computed on a quarter-size sample. This keeps the MCU cost low. */
static uint8_t otsu(const uint8_t *gray, int w, int h, int stride, int *contrast)
{
    uint16_t hist[256] = {0};
    uint32_t sum = 0, sum_b = 0;
    uint32_t total = 0, weight_b = 0;
    float best = -1.0f;
    int low = 255, high = 0, threshold = 110;
    int x, y, i;

    for (y = 0; y < h; y += 2)
        for (x = 0; x < w; x += 2)
        {
            int v = gray[y * stride + x];
            hist[v]++;
            sum += (uint32_t)v;
            total++;
        }
    /* Ignore isolated extreme pixels when estimating image contrast. */
    for (i = 0; i < 256; ++i)
        if (hist[i] >= 3) { low = i; break; }
    for (i = 255; i >= 0; --i)
        if (hist[i] >= 3) { high = i; break; }
    *contrast = high - low;

    for (i = 0; i < 256; ++i)
    {
        float diff, score;
        weight_b += hist[i];
        sum_b += (uint32_t)i * hist[i];
        if (!weight_b || weight_b == total) continue;
        diff = (float)sum_b / weight_b -
               (float)(sum - sum_b) / (total - weight_b);
        score = (float)weight_b * (total - weight_b) * diff * diff;
        if (score > best) { best = score; threshold = i; }
    }
    /* The midpoint handles images with a very small white line fraction. */
    if (threshold < low + 8) threshold = low + 8;
    if (threshold > high - 8) threshold = high - 8;
    return (uint8_t)clamp_i(threshold, 1, 254);
}

/* Three-by-three majority suppresses isolated bright spots and dark holes. */
static void make_mask(vision_result_t *r, const uint8_t *gray,
                      int w, int h, int stride, int threshold)
{
    int x, y, dx, dy;
    memset(r->mask, 0, sizeof(r->mask));
    for (y = 1; y < h - 1; ++y)
        for (x = 1; x < w - 1; ++x)
        {
            int count = 0;
            for (dy = -1; dy <= 1; ++dy)
                for (dx = -1; dx <= 1; ++dx)
                    count += gray[(y + dy) * stride + x + dx] > threshold;
            if (count >= 5) mask_set(r, x, y);
        }
}

static int row_runs(const vision_result_t *r, int w, int y,
                    run_t *runs, int capacity)
{
    int x = 1, n = 0;
    while (x < w - 1)
    {
        int start, end;
        while (x < w - 1 && !white(r, x, y)) ++x;
        start = x;
        while (x < w - 1 && white(r, x, y)) ++x;
        end = x - 1;
        if (end - start + 1 < MIN_RUN_PIXELS) continue;
        if (n < capacity)
        {
            runs[n].left = (int16_t)start;
            runs[n].right = (int16_t)end;
            runs[n].center = (int16_t)((start + end) / 2);
            runs[n].width = (int16_t)(end - start + 1);
            ++n;
        }
    }
    return n;
}

/* Find whether a side branch appears beside a continuing central guide. */
static int ring_hint(const vision_result_t *r, int w, int h)
{
    int left = 0, right = 0, y;
    for (y = h / 4; y < h * 4 / 5; ++y)
    {
        run_t runs[12];
        int n = row_runs(r, w, y, runs, 12);
        int i, central = 0;
        for (i = 0; i < n; ++i)
            if (abs_i(runs[i].center - w / 2) < w / 8 &&
                runs[i].width <= NORMAL_RUN_MAX) central = 1;
        if (!central) continue;
        for (i = 0; i < n; ++i)
        {
            if (runs[i].center < w / 2 - w / 7) ++left;
            if (runs[i].center > w / 2 + w / 7) ++right;
        }
    }
    if (right >= BRANCH_ROWS_MIN && right > left + 2) return 1;
    if (left >= BRANCH_ROWS_MIN && left > right + 2) return -1;
    return 0;
}

static int choose_run(const run_t *runs, int n, int target,
                      int ring_side, int fork_side, int w)
{
    int i, best = -1, best_score = 100000;
    int branch = 0;
    for (i = 0; i < n; ++i)
        if (runs[i].width <= NORMAL_RUN_MAX &&
            abs_i(runs[i].center - target) < w / 3) ++branch;

    for (i = 0; i < n; ++i)
    {
        int score = abs_i(runs[i].center - target) * 4;
        if (runs[i].width > NORMAL_RUN_MAX) score += 50;
        if (runs[i].width > w / 2) score += 100;
        if (branch >= 2 && runs[i].width <= NORMAL_RUN_MAX)
        {
            int side = ring_side ? ring_side : fork_side;
            score -= side * (runs[i].center - target) * 5;
        }
        if (score < best_score) { best_score = score; best = i; }
    }
    return best;
}

void vision_init(vision_context_t *ctx)
{
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->last_center = VISION_MAX_W / 2;
}

void vision_process(vision_context_t *ctx, const uint8_t *gray,
                    uint16_t width, uint16_t height, uint16_t stride)
{
    vision_result_t *r;
    int w = width, h = height, contrast = 0, hint, y;
    int prev, near_sum = 0, near_count = 0, far_sum = 0, far_count = 0;
    int wide_y = -1, wide_left = 0, wide_right = 0;
    int wide_rows = 0, split_rows = 0;
    int valid_rows = 0, target, error, fork_side;

    if (!ctx) return;
    r = &ctx->result;
    memset(r, 0, sizeof(*r));
    if (!gray || w < 16 || w > VISION_MAX_W || h < 16 || h > VISION_MAX_H ||
        stride < width) { r->feature = VISION_LOST; return; }

    r->threshold = otsu(gray, w, h, stride, &contrast);
    if (contrast < 18) { r->feature = VISION_LOST; return; }
    make_mask(r, gray, w, h, stride, r->threshold);
    hint = ring_hint(r, w, h);
    if (hint)
    {
        ctx->ring_side = (int8_t)hint;
        ctx->ring_hold_frames = RING_HOLD_FRAMES;
    }
    else if (ctx->ring_hold_frames && --ctx->ring_hold_frames == 0)
        ctx->ring_side = 0;

    prev = clamp_i(ctx->last_center, 1, w - 2);
    fork_side = (ctx->fork_passes & 1u) ? 1 : -1;
    for (y = h - 2; y >= 1; --y)
    {
        run_t runs[16];
        int n = row_runs(r, w, y, runs, 16);
        int pick, i, nearby = 0;
        for (i = 0; i < n; ++i)
            if (runs[i].width <= NORMAL_RUN_MAX &&
                abs_i(runs[i].center - prev) < w / 3) ++nearby;
        if (nearby >= 2 && y > h / 4 && y < h * 4 / 5) ++split_rows;
        if (!n) continue;
        pick = choose_run(runs, n, prev, ctx->ring_side, fork_side, w);
        if (pick < 0 || abs_i(runs[pick].center - prev) > w / 3)
        {
            /* A horizontal stroke is allowed to contain the tracked line. */
            pick = -1;
            for (i = 0; i < n; ++i)
                if (runs[i].left <= prev && runs[i].right >= prev)
                    { pick = i; break; }
            if (pick < 0) continue;
        }
        r->left[y] = runs[pick].left;
        r->right[y] = runs[pick].right;
        r->center[y] = runs[pick].width >= WIDE_RUN_MIN ?
                       (int16_t)prev : runs[pick].center;
        r->valid[y] = 1;
        prev = r->center[y];
        if (runs[pick].width >= WIDE_RUN_MIN &&
            y > h / 6 && y < h * 5 / 6)
        {
            ++wide_rows;
            if (wide_y < 0)
            {
                wide_y = y;
                wide_left = runs[pick].left;
                wide_right = runs[pick].right;
            }
        }
        if (y >= h / 2 && y <= h * 9 / 10)
        {
            near_sum += prev;
            ++near_count;
        }
        if (y >= h / 4 && y < h / 2)
        {
            far_sum += prev;
            ++far_count;
        }
        ++valid_rows;
    }
    r->valid_rows = (uint8_t)valid_rows;
    r->confidence = (uint8_t)clamp_i(valid_rows * 100 / (h - 2), 0, 100);
    if (valid_rows < h / 7 || !near_count)
    {
        r->feature = VISION_LOST;
        r->error = 0;
        return;
    }

    target = far_count ? (far_sum / far_count * 2 + near_sum / near_count) / 3 :
                         near_sum / near_count;
    r->feature = VISION_STRAIGHT;
    /* A junction is a short horizontal stroke. A broad obstacle can cover
       the guide for many rows and must not be mistaken for a crossing. */
    if (wide_y >= 0 && wide_rows <= 14)
    {
        int above = 0;
        int x = clamp_i(near_sum / near_count, 1, w - 2);
        for (y = wide_y - 5; y >= 1 && y >= wide_y - h / 4; --y)
            if (white(r, x, y)) ++above;
        if (above >= 3)
        {
            /* At a cross, retain the incoming centre and drive straight. */
            r->feature = VISION_CROSS;
            target = near_sum / near_count;
        }
        else if (wide_right - x > x - wide_left + 12)
        {
            r->feature = VISION_TURN_RIGHT;
            target = (x + wide_right) / 2;
        }
        else if (x - wide_left > wide_right - x + 12)
        {
            r->feature = VISION_TURN_LEFT;
            target = (x + wide_left) / 2;
        }
    }
    if (r->feature != VISION_CROSS && r->feature != VISION_TURN_LEFT &&
        r->feature != VISION_TURN_RIGHT)
    {
        if (ctx->ring_side)
            r->feature = ctx->ring_side < 0 ? VISION_RING_LEFT : VISION_RING_RIGHT;
        else if (split_rows >= BRANCH_ROWS_MIN)
            r->feature = VISION_FORK;
        else if (far_count && abs_i(far_sum / far_count - near_sum / near_count) > 8)
            r->feature = VISION_BEND;
    }

    /* One fork is counted only after the branches merge for several frames. */
    if (r->feature == VISION_FORK)
    {
        ctx->fork_active = 1;
        ctx->fork_clear_frames = 0;
    }
    else if (ctx->fork_active && ++ctx->fork_clear_frames >= FORK_CLEAR_FRAMES)
    {
        ++ctx->fork_passes;
        ctx->fork_active = 0;
        ctx->fork_clear_frames = 0;
    }

    error = clamp_i(target - w / 2, -w / 2, w / 2);
    if (r->feature == VISION_STRAIGHT || r->feature == VISION_BEND)
        error = (error * 3 + ctx->last_error) / 4;
    r->error = (int16_t)error;
    ctx->last_error = r->error;
    ctx->last_center = (int16_t)clamp_i(near_sum / near_count, 1, w - 2);
}

const char *vision_feature_name(vision_feature_t feature)
{
    switch (feature)
    {
    case VISION_STRAIGHT: return "straight";
    case VISION_BEND: return "bend";
    case VISION_TURN_LEFT: return "turn left";
    case VISION_TURN_RIGHT: return "turn right";
    case VISION_CROSS: return "cross";
    case VISION_FORK: return "fork";
    case VISION_RING_LEFT: return "ring left";
    case VISION_RING_RIGHT: return "ring right";
    default: return "lost";
    }
}
