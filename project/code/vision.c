#include "vision.h"

#include <string.h>

/* 针对深色赛道上的约 20 mm 白色引导线设置的经验阈值，实车需再标定。 */
#define MIN_RUN_PIXELS       2
#define NORMAL_RUN_MAX      38
#define WIDE_RUN_MIN        44
#define BRANCH_GAP_MIN      10
#define BRANCH_ROWS_MIN      5
#define FORK_CLEAR_FRAMES    4
#define RING_HOLD_FRAMES     8
#define DEFAULT_PREVIEW     60

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
    /* 一字节存 8 个像素，降低单片机内存占用。 */
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

/* 隔行隔列采样（约 1/4 像素）计算 Otsu 阈值，减少单核 MCU 的运算量。 */
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
    /* 估算对比度时忽略只出现一两次的极端亮度，避免噪点误导判断。 */
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
    /* 白线占比很小时，限制阈值不要贴近亮度范围的两端。 */
    if (threshold < low + 8) threshold = low + 8;
    if (threshold > high - 8) threshold = high - 8;
    return (uint8_t)clamp_i(threshold, 1, 254);
}

/* 工程当前不启用编译优化；用宏避免在最内层逐像素调用函数。 */
#define LINE_PIXEL(v) (dark_line ? (v) < threshold : (v) > threshold)

/* 3×3 邻域中过半为赛道线才保留；列和滑动复用，逐像素只新增 3 次比较。 */
static void make_mask(vision_result_t *r, const uint8_t *gray,
                      int w, int h, int stride, int threshold, int dark_line)
{
    int x, y;
    memset(r->mask, 0, sizeof(r->mask));
    for (y = 1; y < h - 1; ++y)
    {
        const uint8_t *top = gray + (y - 1) * stride;
        const uint8_t *mid = gray + y * stride;
        const uint8_t *bottom = gray + (y + 1) * stride;
        int left = LINE_PIXEL(top[0]) + LINE_PIXEL(mid[0]) +
                   LINE_PIXEL(bottom[0]);
        int center = LINE_PIXEL(top[1]) + LINE_PIXEL(mid[1]) +
                     LINE_PIXEL(bottom[1]);
        for (x = 1; x < w - 1; ++x)
        {
            int right = LINE_PIXEL(top[x + 1]) + LINE_PIXEL(mid[x + 1]) +
                        LINE_PIXEL(bottom[x + 1]);
            if (left + center + right >= 5) mask_set(r, x, y);
            left = center;
            center = right;
        }
    }
}

#undef LINE_PIXEL

static int row_runs(const vision_result_t *r, int w, int y,
                    run_t *runs, int capacity)
{
    /* 将一行中的连续白色像素提取成若干线段，记录左右边界和中心。 */
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

/* 中央引导线持续存在时，统计其旁边是否出现稳定的左/右支线，作为环岛提示。 */
static int ring_hint(const vision_result_t *r, int w, int h)
{
    int left = 0, right = 0, y;
    for (y = h / 4; y < h * 4 / 5; ++y)
    {
        run_t runs[12];
        int n = row_runs(r, w, y, runs, 12);
        int i, central = 0, left_in_row = 0, right_in_row = 0;
        for (i = 0; i < n; ++i)
            if (abs_i(runs[i].center - w / 2) < w / 8 &&
                runs[i].width <= NORMAL_RUN_MAX) central = 1;
        if (!central) continue;
        for (i = 0; i < n; ++i)
        {
            if (runs[i].center < w / 2 - w / 7) left_in_row = 1;
            if (runs[i].center > w / 2 + w / 7) right_in_row = 1;
        }
        /* 同一行的多段反光或噪声只算一次，门槛才真正表示持续的行数。 */
        left += left_in_row;
        right += right_in_row;
    }
    if (right >= BRANCH_ROWS_MIN && right > left + 2) return 1;
    if (left >= BRANCH_ROWS_MIN && left > right + 2) return -1;
    return 0;
}

static int choose_run(const run_t *runs, int n, int target,
                      int ring_side, int fork_side, int w)
{
    /* 优先延续上一行的位置；分叉时再按环岛提示或岔路轮换方向选支线。 */
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
    ctx->preview_setting = DEFAULT_PREVIEW;
}

void vision_set_preview(vision_context_t *ctx, uint8_t percent)
{
    if (ctx) ctx->preview_setting = (uint8_t)clamp_i(percent, 15, 90);
}

void vision_set_line_polarity(vision_context_t *ctx, vision_line_polarity_t polarity)
{
    if (ctx) ctx->line_polarity = polarity == VISION_LINE_DARK ?
                                  VISION_LINE_DARK : VISION_LINE_BRIGHT;
}

void vision_set_fork_route(vision_context_t *ctx, const int8_t *sides, uint8_t count)
{
    uint8_t i;
    if (!ctx) return;
    if (!sides || !count)
    {
        ctx->fork_route_len = 0;
        return;
    }
    if (count > VISION_MAX_FORK_ROUTE) count = VISION_MAX_FORK_ROUTE;
    for (i = 0; i < count; ++i)
        if (sides[i] != -1 && sides[i] != 1) return;
    for (i = 0; i < count; ++i)
        ctx->fork_route[i] = sides[i];
    ctx->fork_route_len = count;
}

void vision_process(vision_context_t *ctx, const uint8_t *gray,
                    uint16_t width, uint16_t height, uint16_t stride)
{
    /* 单帧流程：自适应阈值→二值化→逐行追线→识别路况→计算转向偏差。 */
    vision_result_t *r;
    int w = width, h = height, contrast = 0, hint, y;
    int prev, near_sum = 0, near_count = 0;
    int mid_sum = 0, mid_count = 0, far_sum = 0, far_count = 0;
    int wide_y = -1, wide_left = 0, wide_right = 0;
    int wide_rows = 0, split_rows = 0;
    int valid_rows = 0, target, error, fork_side, retry = 0;
    int near_x, mid_x, far_x, preview;

    if (!ctx) return;
    r = &ctx->result;
    memset(r, 0, sizeof(*r));
    /* 输入无效或画面几乎无明暗对比时，直接报告丢线。 */
    if (!gray || w < 16 || w > VISION_MAX_W || h < 16 || h > VISION_MAX_H ||
        stride < width) { r->feature = VISION_LOST; return; }

    r->threshold = otsu(gray, w, h, stride, &contrast);
    if (contrast < 18) { r->feature = VISION_LOST; return; }
    make_mask(r, gray, w, h, stride, r->threshold,
              ctx->line_polarity == VISION_LINE_DARK);
    hint = ring_hint(r, w, h);
    if (hint)
    {
        ctx->ring_side = (int8_t)hint;
        ctx->ring_hold_frames = RING_HOLD_FRAMES;
    }
    else if (ctx->ring_hold_frames && --ctx->ring_hold_frames == 0)
        ctx->ring_side = 0;

    /* 已设路线时按表选岔路；路线用完后回到原先的左右交替策略。 */
    fork_side = ctx->fork_passes < ctx->fork_route_len ?
                ctx->fork_route[ctx->fork_passes] :
                ((ctx->fork_passes & 1u) ? 1 : -1);
track_rows:
    /* 从画面下方向上追踪白线；首次从上一帧中心开始，失败后从画面中心重试。 */
    prev = retry ? w / 2 : clamp_i(ctx->last_center, 1, w - 2);
    near_sum = near_count = mid_sum = mid_count = far_sum = far_count = 0;
    wide_y = -1;
    wide_left = wide_right = wide_rows = split_rows = valid_rows = 0;
    if (retry)
    {
        memset(r->left, 0, sizeof(r->left));
        memset(r->right, 0, sizeof(r->right));
        memset(r->center, 0, sizeof(r->center));
        memset(r->valid, 0, sizeof(r->valid));
    }
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
            /* 横向白条可能覆盖原线：若它跨过预测中心，仍允许继续追踪。 */
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
        if (y >= h * 2 / 3 && y <= h * 9 / 10)
        {
            near_sum += prev;
            ++near_count;
        }
        if (y >= h / 2 && y < h * 2 / 3)
        {
            mid_sum += prev;
            ++mid_count;
        }
        if (y >= h / 4 && y < h / 2)
        {
            far_sum += prev;
            ++far_count;
        }
        ++valid_rows;
    }
    if ((valid_rows < h / 7 || !near_count) && !retry)
    {
        retry = 1;
        goto track_rows;
    }
    r->valid_rows = (uint8_t)valid_rows;
    /* 有效行比例作为置信度；近处没有线时即使远处有线也判为丢线。 */
    r->confidence = (uint8_t)clamp_i(valid_rows * 100 / (h - 2), 0, 100);
    if (valid_rows < h / 7 || !near_count)
    {
        r->feature = VISION_LOST;
        r->error = 0;
        return;
    }

    near_x = near_sum / near_count;
    mid_x = mid_count ? mid_sum / mid_count : near_x;
    far_x = far_count ? far_sum / far_count : mid_x;
    r->heading = (int16_t)(far_x - near_x);
    r->curvature = (int16_t)(far_count && mid_count ?
                             far_x - 2 * mid_x + near_x : 0);
    /* 用近、中、远三段中心估计目标点。curvature 只是图像弯曲趋势，
       不是实际道路半径；弯道明显或可信行较少时缩短前瞻。 */
    preview = ctx->preview_setting;
    preview -= abs_i(r->curvature) * 2;
    if (r->confidence < 50) preview -= 10;
    preview = clamp_i(preview, 15, 90);
    r->preview_percent = (uint8_t)preview;
    target = near_x + (r->heading - 2 * r->curvature) * preview / 100 +
             2 * r->curvature * preview * preview / 10000;
    target = clamp_i(target, 1, w - 2);
    r->feature = VISION_STRAIGHT;
    /* 短的横向宽白条可能是路口；若连续宽行太多，避免把大片障碍误判为十字。 */
    if (wide_y >= 0 && wide_rows <= 14)
    {
        int above = 0;
        int x = clamp_i(near_sum / near_count, 1, w - 2);
        for (y = wide_y - 5; y >= 1 && y >= wide_y - h / 4; --y)
            if (white(r, x, y)) ++above;
        if (above >= 3)
        {
            /* 十字路口沿进入时的中心直行，不追随左右横线。 */
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
        else if (far_count && (abs_i(r->heading) > 8 ||
                              abs_i(r->curvature) > 6))
            r->feature = VISION_BEND;
    }
    /* 岔路直接取所选支线的远处中心，避免插值目标落在两条支线之间的空白区。 */
    if (r->feature == VISION_FORK && far_count)
    {
        target = far_x;
        r->preview_percent = 100;
    }

    r->preview_x = (int16_t)target;

    /* 支线消失并保持数帧后才计为通过一次岔路，避免同一岔路被连续计数。 */
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
    /* 直道和普通弯道对偏差做简单平滑；急转、路口等保留即时响应。 */
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
