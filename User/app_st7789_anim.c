#include "app_st7789_anim.h"
#include "anim_frames.h"
#include "bsp_st7789.h"
#include "timing.h"

static uint32_t g_anim_last_tick = 0U;
static uint8_t g_anim_frame_index = 0U;

/*
 * Initialize the ST7789 animation playback state.
 *
 * The first indexed frame is expanded from 120x120 to 240x240 immediately.
 * Frame data and the RGB565 palette remain in Flash; no frame buffer or
 * dynamic memory is used.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Writes the first frame to the ST7789 and resets animation timing.
 */
void App_ST7789_AnimInit(void)
{
    ST7789_ShowIndexed4Image(
        anim_frames[0],
        anim_palette,
        ANIM_FRAME_WIDTH,
        ANIM_FRAME_HEIGHT,
        ANIM_PIXEL_SCALE
    );

    g_anim_last_tick = Timing_GetTick();
    g_anim_frame_index = (ANIM_FRAME_COUNT > 1U) ? 1U : 0U;
}

/*
 * Advance the ST7789 animation when the configured frame interval expires.
 *
 * Unsigned tick subtraction remains valid across the system tick wraparound.
 * A full frame transfer is synchronous, while the interval wait itself is
 * non-blocking so other main-loop tasks may run between refreshes.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Streams one indexed frame to the ST7789 when it becomes due.
 */
void App_ST7789_AnimTask(void)
{
    uint32_t now;

    now = Timing_GetTick();
    if ((now - g_anim_last_tick) < ANIM_FRAME_INTERVAL_MS)
    {
        return;
    }

    ST7789_ShowIndexed4Image(
        anim_frames[g_anim_frame_index],
        anim_palette,
        ANIM_FRAME_WIDTH,
        ANIM_FRAME_HEIGHT,
        ANIM_PIXEL_SCALE
    );

    g_anim_last_tick = now;
    g_anim_frame_index++;
    if (g_anim_frame_index >= ANIM_FRAME_COUNT)
    {
        g_anim_frame_index = 0U;
    }
}
