#include "page_home.h"
#include "ui_draw.h"

#define PAGE_HOME_ITEM_COUNT       3U      // Player, settings, and info.

static uint8_t g_home_selected = 0U;

/*
 * Enter the home page.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * None. Selection is retained so BACK does not surprise the user.
 */
static void Page_Home_OnEnter(void)
{
}

/*
 * Enter the selected home menu item.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Changes the active page.
 */
static void Page_Home_EnterSelected(void)
{
    if (g_home_selected == 0U)
    {
        UI_PageGoto(UI_PAGE_PLAYER);
    }
    else if (g_home_selected == 1U)
    {
        UI_PageGoto(UI_PAGE_SETTINGS);
    }
    else
    {
        UI_PageGoto(UI_PAGE_INFO);
    }
}

/*
 * Handle home page navigation.
 *
 * Parameters:
 * event: UI event after global routing.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates focus or enters another page.
 */
static void Page_Home_OnEvent(const UI_Event *event)
{
    if (event->type == UI_EVENT_UP)
    {
        g_home_selected = (g_home_selected == 0U) ?
            (PAGE_HOME_ITEM_COUNT - 1U) :
            (uint8_t)(g_home_selected - 1U);
        UI_PageRequestRedraw();
    }
    else if (event->type == UI_EVENT_DOWN)
    {
        g_home_selected++;
        if (g_home_selected >= PAGE_HOME_ITEM_COUNT)
        {
            g_home_selected = 0U;
        }
        UI_PageRequestRedraw();
    }
    else if ((event->type == UI_EVENT_OK) || (event->type == UI_EVENT_RIGHT))
    {
        Page_Home_EnterSelected();
    }
    else if (event->type == UI_EVENT_LEFT)
    {
        g_home_selected = 0U;
        UI_PageRequestRedraw();
    }
}

/*
 * Draw the home page.
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
static void Page_Home_Draw(void)
{
    UI_DrawClear(UI_COLOR_BG);
    UI_DrawStatusBar("UI HOME", UI_COLOR_ACCENT);
    UI_DrawMenuRow(12, 44, 216, "PLAYER", "OFF", (g_home_selected == 0U) ? 1U : 0U);
    UI_DrawMenuRow(12, 88, 216, "SETTINGS", 0, (g_home_selected == 1U) ? 1U : 0U);
    UI_DrawMenuRow(12, 132, 216, "INFO", 0, (g_home_selected == 2U) ? 1U : 0U);
    UI_DrawFooter("UP/DOWN MID OK");
}

const UI_PageOps PAGE_HOME_OPS =
{
    Page_Home_OnEnter,
    Page_Home_OnEvent,
    Page_Home_Draw
};
