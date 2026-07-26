#include "page_home.h"
#include "ui_draw.h"

#define PAGE_HOME_ITEM_COUNT       3U      // Player, settings, and info.

static uint8_t g_home_selected = 0U;

/*
 * Build the repaint rectangle for one home menu row.
 *
 * Parameters:
 * index: Menu item index.
 *
 * Return value:
 * Rectangle covering the full row.
 *
 * Side effects:
 * None.
 */
static UI_Rect Page_Home_GetRowRect(uint8_t index)
{
    UI_Rect rect;

    rect.x = 12;
    rect.y = (int16_t)(44 + ((int16_t)index * 44));
    rect.w = 216;
    rect.h = (int16_t)UI_ROW_H;

    return rect;
}

/*
 * Mark one home menu row dirty.
 *
 * Parameters:
 * index: Menu item index.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Queues a local repaint for the row.
 */
static void Page_Home_InvalidateRow(uint8_t index)
{
    UI_Rect rect;

    rect = Page_Home_GetRowRect(index);
    UI_PageInvalidate(&rect);
}

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
    uint8_t old_selected;

    if (event->type == UI_EVENT_UP)
    {
        old_selected = g_home_selected;
        g_home_selected = (g_home_selected == 0U) ?
            (PAGE_HOME_ITEM_COUNT - 1U) :
            (uint8_t)(g_home_selected - 1U);
        Page_Home_InvalidateRow(old_selected);
        Page_Home_InvalidateRow(g_home_selected);
    }
    else if (event->type == UI_EVENT_DOWN)
    {
        old_selected = g_home_selected;
        g_home_selected++;
        if (g_home_selected >= PAGE_HOME_ITEM_COUNT)
        {
            g_home_selected = 0U;
        }
        Page_Home_InvalidateRow(old_selected);
        Page_Home_InvalidateRow(g_home_selected);
    }
    else if ((event->type == UI_EVENT_OK) || (event->type == UI_EVENT_RIGHT))
    {
        Page_Home_EnterSelected();
    }
    else if (event->type == UI_EVENT_LEFT)
    {
        old_selected = g_home_selected;
        g_home_selected = 0U;
        Page_Home_InvalidateRow(old_selected);
        Page_Home_InvalidateRow(g_home_selected);
    }
}

/*
 * Draw the home page within the requested clip.
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
static void Page_Home_Draw(const UI_Rect *clip)
{
    (void)clip;

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
