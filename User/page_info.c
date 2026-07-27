#include "page_info.h"
#include "ui_dirty.h"
#include "ui_draw.h"
#include "ui_renderer.h"

#define TEXT_INFO_TITLE        "\xCF\xB5\xCD\xB3\xD0\xC5\xCF\xA2"
#define TEXT_MCU              "MCU"
#define TEXT_LCD              "LCD"
#define TEXT_STRIP            "STRIP"
#define TEXT_BUSY             "BUSY"
#define TEXT_DIRTY            "DIRTY"
#define TEXT_FOOTER_INFO      "\xD7\xF3\xBC\xFC\xB7\xB5\xBB\xD8  SET\xC9\xE8\xD6\xC3"

/*
 * Convert a 32-bit counter to compact decimal ASCII.
 *
 * INFO draws renderer counters without using formatted stdio. When a counter
 * exceeds the display budget, the low digits are still useful for confirming
 * that the value is changing during manual tests.
 *
 * Parameters:
 * value: Counter value to format.
 * buffer: Destination buffer.
 * buffer_size: Number of bytes in buffer.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Writes buffer.
 */
static void Page_Info_FormatU32(uint32_t value, char *buffer, uint8_t buffer_size)
{
    char digits[10];
    uint8_t count;
    uint8_t index;

    if ((buffer == 0) || (buffer_size < 2U))
    {
        return;
    }

    count = 0U;
    do
    {
        digits[count] = (char)('0' + (value % 10U));
        value /= 10U;
        count++;
    } while ((value != 0U) && (count < sizeof(digits)));

    if (count >= buffer_size)
    {
        count = (uint8_t)(buffer_size - 1U);
    }

    for (index = 0U; index < count; index++)
    {
        buffer[index] = digits[count - 1U - index];
    }
    buffer[count] = '\0';
}

/*
 * Format dirty queue occupancy as max/current.
 *
 * Parameters:
 * stats: Dirty queue statistics snapshot.
 * buffer: Destination buffer.
 * buffer_size: Number of bytes in buffer.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Writes buffer.
 */
static void Page_Info_FormatDirty(const UI_DirtyStats *stats, char *buffer, uint8_t buffer_size)
{
    if ((stats == 0) || (buffer == 0) || (buffer_size < 4U))
    {
        return;
    }

    buffer[0] = (char)('0' + stats->max_pending_rects);
    buffer[1] = '/';
    buffer[2] = (char)('0' + stats->pending_rects);
    buffer[3] = '\0';
}

/*
 * Enter the information page.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * None.
 */
static void Page_Info_OnEnter(void)
{
}

/*
 * Handle information page events.
 *
 * Parameters:
 * event: UI event after global routing.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * LEFT returns to HOME.
 */
static void Page_Info_OnEvent(const UI_Event *event)
{
    if (event->type == UI_EVENT_LEFT)
    {
        UI_PageBack();
    }
}

/*
 * Draw one labeled information row.
 *
 * Parameters:
 * y: Top coordinate.
 * label: Left label.
 * value: Right value.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws one row.
 */
static void Page_Info_DrawRow(int16_t y, const char *label, const char *value)
{
    UI_DrawInfoRowCN(y, label, value);
}

/*
 * Draw the system information page within the requested clip.
 *
 * Parameters:
 * clip: Dirty rectangle currently being repainted.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Repaints the ST7789 area intersecting clip.
 */
static void Page_Info_Draw(const UI_Rect *clip)
{
    UI_RendererStats renderer_stats;
    UI_DirtyStats dirty_stats;
    char strip_text[8];
    char busy_text[8];
    char dirty_text[5];

    (void)clip;

    UI_RendererGetStats(&renderer_stats);
    UI_DirtyGetStats(&dirty_stats);
    Page_Info_FormatU32(renderer_stats.strips_submitted, strip_text, (uint8_t)sizeof(strip_text));
    Page_Info_FormatU32(renderer_stats.busy_returns, busy_text, (uint8_t)sizeof(busy_text));
    Page_Info_FormatDirty(&dirty_stats, dirty_text, (uint8_t)sizeof(dirty_text));

    UI_DrawStatusBar(TEXT_INFO_TITLE, UI_COLOR_OK);
    Page_Info_DrawRow(42, TEXT_MCU, "STM32F103C8");
    Page_Info_DrawRow(76, TEXT_LCD, "ST7789");
    Page_Info_DrawRow(110, TEXT_STRIP, strip_text);
    Page_Info_DrawRow(144, TEXT_BUSY, busy_text);
    Page_Info_DrawRow(178, TEXT_DIRTY, dirty_text);
    UI_DrawFooter(TEXT_FOOTER_INFO);
}

const UI_PageOps PAGE_INFO_OPS =
{
    Page_Info_OnEnter,
    Page_Info_OnEvent,
    0,
    Page_Info_Draw
};
