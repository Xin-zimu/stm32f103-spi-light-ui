#include "page_info.h"
#include "ui_draw.h"

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
    UI_DrawRect(12, y, 216, 26, UI_COLOR_SURFACE);
    UI_DrawText(22, (int16_t)(y + 8), label, UI_COLOR_MUTED);
    UI_DrawText(88, (int16_t)(y + 8), value, UI_COLOR_TEXT);
}

/*
 * Draw the system information page.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Replaces the visible ST7789 image.
 */
static void Page_Info_Draw(void)
{
    UI_DrawClear(UI_COLOR_BG);
    UI_DrawStatusBar("INFO", UI_COLOR_OK);
    Page_Info_DrawRow(42, "MCU", "STM32F103C8");
    Page_Info_DrawRow(72, "LCD", "ST7789 240");
    Page_Info_DrawRow(102, "KEY", "PA0-PA6");
    Page_Info_DrawRow(132, "GIF", "DISABLED");
    Page_Info_DrawRow(162, "BUILD", "UI FIRST");
    UI_DrawFooter("RST BACK SET MENU");
}

const UI_PageOps PAGE_INFO_OPS =
{
    Page_Info_OnEnter,
    Page_Info_OnEvent,
    Page_Info_Draw
};
