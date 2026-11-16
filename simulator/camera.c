/* Drop-in replacement for Liumian-SimuEnv/code/camera.c.
 * Copy this file there together with project/code/vision.c and vision.h.
 * The simulator's environment and library files stay untouched. */
#include "camera.h"
#include "vision.h"

int otsu_threshold;
int left_line[LM_IMAGE_H];
int right_line[LM_IMAGE_H];
int mid_line[LM_IMAGE_H];
int edge_count;

static vision_context_t s_vision;

void image_init(void)
{
    vision_init(&s_vision);
}

int image_process(void)
{
    const vision_result_t *r;
    int x, y;
    if (LM_IMAGE_W > VISION_MAX_W || LM_IMAGE_H > VISION_MAX_H) return 0;
    vision_process(&s_vision, &mt9v03x_image[0][0],
                   LM_IMAGE_W, LM_IMAGE_H, LM_IMAGE_W);
    r = &s_vision.result;
    otsu_threshold = r->threshold;
    edge_count = r->valid_rows * 2;
    for (y = 0; y < LM_IMAGE_H; ++y)
    {
        left_line[y] = r->valid[y] ? r->left[y] : -1;
        right_line[y] = r->valid[y] ? r->right[y] : -1;
        mid_line[y] = r->valid[y] ? r->center[y] : -1;
    }
    for (y = 0; y < LM_OUT_IMAGE_H; ++y)
        for (x = 0; x < LM_OUT_IMAGE_W; ++x)
            LM_OutImageSet(x, y,
                vision_binary_get(r, x + LM_OUT_IMAGE_X,
                                     y + LM_OUT_IMAGE_Y) ? 255 : 0);

    LM_Log(LM_LOG_LEVEL_INFO, "feature=%s error=%d confidence=%u threshold=%u",
           vision_feature_name(r->feature), r->error,
           (unsigned)r->confidence, (unsigned)r->threshold);
    LM_Log(LM_LOG_LEVEL_INFO, "completed forks=%u (even=left, odd=right)",
           (unsigned)s_vision.fork_passes);
    return r->feature != VISION_LOST;
}
