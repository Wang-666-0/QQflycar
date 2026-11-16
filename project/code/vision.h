#ifndef FLYCAR_VISION_H
#define FLYCAR_VISION_H

#include <stdint.h>

/* MT9V03X default size. The processor also accepts smaller frames. */
#define VISION_MAX_W 188
#define VISION_MAX_H 120
#define VISION_MASK_STRIDE ((VISION_MAX_W + 7) / 8)

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
    VISION_RING_RIGHT
} vision_feature_t;

typedef struct
{
    uint8_t mask[VISION_MAX_H][VISION_MASK_STRIDE];
    int16_t left[VISION_MAX_H];
    int16_t right[VISION_MAX_H];
    int16_t center[VISION_MAX_H];
    uint8_t valid[VISION_MAX_H];
    int16_t error;             /* signed pixels: positive means steer right */
    uint8_t threshold;
    uint8_t confidence;        /* 0..100 */
    uint8_t valid_rows;
    vision_feature_t feature;
} vision_result_t;

typedef struct
{
    vision_result_t result;
    int16_t last_center;
    int16_t last_error;
    uint8_t fork_passes;       /* alternates left/right after a completed fork */
    uint8_t fork_active;
    uint8_t fork_clear_frames;
    int8_t ring_side;          /* -1 left, +1 right, 0 no ring hint */
    uint8_t ring_hold_frames;
} vision_context_t;

void vision_init(vision_context_t *ctx);
void vision_process(vision_context_t *ctx, const uint8_t *gray,
                    uint16_t width, uint16_t height, uint16_t stride);
uint8_t vision_binary_get(const vision_result_t *result, uint16_t x, uint16_t y);
const char *vision_feature_name(vision_feature_t feature);

#endif
