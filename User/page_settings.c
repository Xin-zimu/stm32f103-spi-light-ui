#include "page_settings.h"
#include "ui_draw.h"

#define PAGE_SETTINGS_ITEM_COUNT   3U      // Animation, brightness, and theme.

static uint8_t g_settings_selected = 0U;
static uint8_t g_settings_animation = 1U;
static uint8_t g_settings_brightness = 3U;
static uint8_t g_settings_theme = 0U;

/*
 * Enter the settings page.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * None. Values are RAM settings retained across page visits.
 */
static void Page_Settings_OnEnter(void)
{
}

/*
 * Change the currently focused setting.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates one RAM setting and requests redraw.
 */
static void Page_Settings_ChangeValue(void)
{
    if (g_settings_selected == 0U)
    {
        g_settings_animation ^= 1U;
    }
    else if (g_settings_selected == 1U)
    {
        g_settings_brightness++;
        if (g_settings_brightness > 5U)
        {
            g_settings_brightness = 1U;
        }
    }
    else
    {
        g_settings_theme++;
        if (g_settings_theme > 2U)
        {
            g_settings_theme = 0U;
        }
    }
    UI_PageRequestRedraw();
}

/*
 * Handle settings page events.
 *
 * Parameters:
 * event: UI event after global routing.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates focus, settings, or active page.
 */
static void Page_Settings_OnEvent(const UI_Event *event)
{
    if (event->type == UI_EVENT_UP)
    {
        g_settings_selected = (g_settings_selected == 0U) ?
            (PAGE_SETTINGS_ITEM_COUNT - 1U) :
            (uint8_t)(g_settings_selected - 1U);
        UI_PageRequestRedraw();
    }
    else if (event->type == UI_EVENT_DOWN)
    {
        g_settings_selected++;
        if (g_settings_selected >= PAGE_SETTINGS_ITEM_COUNT)
        {
            g_settings_selected = 0U;
        }
        UI_PageRequestRedraw();
    }
    else if (event->type == UI_EVENT_LEFT)
    {
        UI_PageBack();
    }
    else if ((event->type == UI_EVENT_RIGHT) || (event->type == UI_EVENT_OK))
    {
        Page_Settings_ChangeValue();
    }
}

/*
 * Draw the settings page.
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
static void Page_Settings_Draw(void)
{
    static const char * const theme_names[3] = {"CYAN", "GOLD", "GREEN"};

    UI_DrawClear(UI_COLOR_BG);
    UI_DrawStatusBar("SETTINGS", UI_COLOR_WARN);
    UI_DrawMenuRow(
        12,
        44,
        216,
        "ANIMATION",
        (g_settings_animation != 0U) ? "ON" : "OFF",
        (g_settings_selected == 0U) ? 1U : 0U
    );
    UI_DrawMenuRow(12, 88, 216, "BRIGHT", 0, (g_settings_selected == 1U) ? 1U : 0U);
    UI_DrawProgressBar(128, 101, 78, g_settings_brightness, 5U);
    UI_DrawMenuRow(
        12,
        132,
        216,
        "THEME",
        theme_names[g_settings_theme],
        (g_settings_selected == 2U) ? 1U : 0U
    );
    UI_DrawToggle(188, 55, g_settings_animation);
    UI_DrawFooter("RIGHT CHANGE LEFT BACK");
}

const UI_PageOps PAGE_SETTINGS_OPS =
{
    Page_Settings_OnEnter,
    Page_Settings_OnEvent,
    Page_Settings_Draw
};
