#include "page_info.h"
#include "ui_draw.h"

#define TEXT_INFO_TITLE        "\xCF\xB5\xCD\xB3\xD0\xC5\xCF\xA2"
#define TEXT_MCU              "MCU"
#define TEXT_LCD              "LCD"
#define TEXT_KEY              "KEY"
#define TEXT_MEDIA            "MEDIA"
#define TEXT_BUILD            "\xD7\xB4\xCC\xAC"
#define TEXT_NO_GIF           "NO GIF"
#define TEXT_UI_FIRST         "UI FIRST"
#define TEXT_FOOTER_INFO      "\xD7\xF3\xBC\xFC\xB7\xB5\xBB\xD8  SET\xC9\xE8\xD6\xC3"

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
    (void)clip;

    UI_DrawStatusBar(TEXT_INFO_TITLE, UI_COLOR_OK);
    Page_Info_DrawRow(42, TEXT_MCU, "STM32F103C8");
    Page_Info_DrawRow(76, TEXT_LCD, "ST7789");
    Page_Info_DrawRow(110, TEXT_KEY, "PA0-PA6");
    Page_Info_DrawRow(144, TEXT_MEDIA, TEXT_NO_GIF);
    Page_Info_DrawRow(178, TEXT_BUILD, TEXT_UI_FIRST);
    UI_DrawFooter(TEXT_FOOTER_INFO);
}

const UI_PageOps PAGE_INFO_OPS =
{
    Page_Info_OnEnter,
    Page_Info_OnEvent,
    0,
    Page_Info_Draw
};
