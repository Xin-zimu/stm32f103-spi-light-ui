#include "app_st7789_anim.h"
#include "anim_frames.h"
#include "bsp_st7789.h"
#include "timing.h"

static uint32_t g_anim_last_tick = 0U;
static uint8_t g_anim_frame_index = 0U;

/*
 * Initialize the ST7789 GIF playback state and draw the complete first frame.
 *
 * The first frame is stored as a normal packed four-bit indexed image because
 * it establishes every display pixel. Later frames retain unchanged pixels in
 * the controller RAM and apply only their encoded differences.
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
    g_anim_frame_index = (ANIM_FRAME_COUNT > 1U) ? 1U : 0U;
}

/*
 * Advance GIF playback after the non-blocking frame interval expires.
 *
 * Frames 1 through N use offsets into the shared delta stream. When playback
 * wraps, frame zero is drawn completely so the next loop never depends on
 * stale controller RAM. The timestamp is recorded after the synchronous SPI
 * transfer, giving every completed frame its configured visible hold time.
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

    if (g_anim_frame_index == 0U)
    {
        ST7789_ShowIndexed4Image(
            anim_first_frame,
            anim_palette,
            ANIM_FRAME_WIDTH,
            ANIM_FRAME_HEIGHT,
            ANIM_PIXEL_SCALE
        );
    }
    else
    {
        uint32_t delta_start;
        uint32_t delta_end;

        delta_start = anim_delta_offsets[g_anim_frame_index - 1U];
        delta_end = anim_delta_offsets[g_anim_frame_index];
        ST7789_ApplyIndexed4Delta(
            &anim_delta_data[delta_start],
            delta_end - delta_start,
            anim_palette,
            ANIM_FRAME_WIDTH,
            ANIM_FRAME_HEIGHT,
            ANIM_PIXEL_SCALE
        );
    }

    g_anim_last_tick = Timing_GetTick();
    g_anim_frame_index++;
    if (g_anim_frame_index >= ANIM_FRAME_COUNT)
    {
        g_anim_frame_index = 0U;
    }
}
