#include "app_st7789_anim.h"
#include "anim_frames.h"
#include "bsp_st7789.h"
#include "timing.h"

static uint32_t g_anim_last_tick = 0U;
static uint8_t g_anim_transition_index = 0U;

/*
 * Initialize the ST7789 GIF playback state and draw the complete first frame.
 *
 * The first frame is stored as a normal packed four-bit indexed image because
 * it establishes every display pixel. Every later transition, including the
 * last frame back to the first, retains unchanged controller RAM pixels and
 * applies only its encoded differences.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Replaces the full ST7789 image and resets the frame scheduler.
 */
void App_ST7789_AnimInit(void)
{
    ST7789_ShowIndexed4Image(
        anim_first_frame,
        anim_palette,
        ANIM_FRAME_WIDTH,
        ANIM_FRAME_HEIGHT,
        ANIM_PIXEL_SCALE
    );

    g_anim_last_tick = Timing_GetTick();
    g_anim_transition_index = 0U;
}

/*
 * Advance GIF playback after the non-blocking frame interval expires.
 *
 * Each offset pair describes one transition: frame 0 to 1 through the final
 * frame back to frame 0. The loop boundary therefore costs roughly the same
 * as every other update instead of sending a complete 240x240 frame.
 * Advancing the deadline by a fixed interval keeps frame start times uniform;
 * the measured 7 to 12 ms transfer cost does not extend every frame period.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates the ST7789 when one animation frame becomes due.
 */
void App_ST7789_AnimTask(void)
{
    uint32_t now;

    now = Timing_GetTick();
    if ((now - g_anim_last_tick) < ANIM_FRAME_INTERVAL_MS)
    {
        return;
    }

    {
        uint32_t delta_start;
        uint32_t delta_end;

        delta_start = anim_delta_offsets[g_anim_transition_index];
        delta_end = anim_delta_offsets[g_anim_transition_index + 1U];
        ST7789_ApplyIndexed4Delta(
            &anim_delta_data[delta_start],
            delta_end - delta_start,
            anim_palette,
            ANIM_FRAME_WIDTH,
            ANIM_FRAME_HEIGHT,
            ANIM_PIXEL_SCALE
        );
    }

    g_anim_last_tick += ANIM_FRAME_INTERVAL_MS;
    g_anim_transition_index++;
    if (g_anim_transition_index >= ANIM_FRAME_COUNT)
    {
        g_anim_transition_index = 0U;
    }
}
