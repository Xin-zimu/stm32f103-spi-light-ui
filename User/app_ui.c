#include "app_ui.h"
#include "app_st7789_anim.h"
#include "bsp_st7789.h"
#include "ui_event.h"

#define UI_MAX_EVENTS_PER_TASK     4U      // Events handled per main-loop pass.
#define UI_MENU_COUNT              2U      // GIF player and settings entries.

#define UI_COLOR_BG                0x0841U // Dark neutral background.
#define UI_COLOR_PANEL             0x2104U // Inactive panel color.
#define UI_COLOR_SELECTED          0xFD20U // Selected menu color.
#define UI_COLOR_ACCENT            0x07FFU // Cyan accent.
#define UI_COLOR_OK                0x07E0U // Green status color.
#define UI_COLOR_WARN              0xFBE0U // Yellow status color.

typedef enum
{
    UI_PAGE_HOME = 0,                      // Main menu page.
    UI_PAGE_GIF_PLAYER                     // Existing GIF playback page.
} UI_PageId;

static UI_PageId g_ui_page = UI_PAGE_HOME;
static uint8_t g_ui_menu_index = 0U;
static uint8_t g_ui_redraw_pending = 0U;

/*
 * Draw the first-stage home menu.
 *
 * The UI uses only rectangles so it can run on the existing ST7789 driver
 * without adding font tables or a frame buffer. The top highlighted row enters
 * GIF playback and the lower row opens a settings placeholder.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Replaces the full ST7789 image with the home menu.
 */
static void App_UI_DrawHome(void)
{
    uint16_t top_color;
    uint16_t bottom_color;

    top_color = (g_ui_menu_index == 0U) ? UI_COLOR_SELECTED : UI_COLOR_PANEL;
    bottom_color = (g_ui_menu_index == 1U) ? UI_COLOR_SELECTED : UI_COLOR_PANEL;

    ST7789_Clear(UI_COLOR_BG);
    ST7789_FillRect(0U, 0U, ST7789_WIDTH, 18U, UI_COLOR_ACCENT);
    ST7789_FillRect(24U, 52U, 192U, 36U, top_color);
    ST7789_FillRect(36U, 64U, 112U, 8U, UI_COLOR_OK);
    ST7789_FillRect(24U, 112U, 192U, 36U, bottom_color);
    ST7789_FillRect(36U, 124U, 76U, 8U, UI_COLOR_WARN);
}

/*
 * Return to the home page and request one redraw.
 *
 * This shallow navigation model is intentional for the first hardware test:
 * every child page can be escaped with LEFT or RST, and long RST also routes
 * here before the two-second reset threshold.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Changes page state and marks the display dirty.
 */
static void App_UI_GoHome(void)
{
    g_ui_page = UI_PAGE_HOME;
    g_ui_redraw_pending = 1U;
}

/*
 * Enter the currently selected home item.
 *
 * Selection 0 starts the preserved GIF delta player from its first retained
 * frame. Selection 1 opens the settings placeholder.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * May redraw the display or restart GIF playback.
 */
static void App_UI_EnterSelected(void)
{
    if (g_ui_menu_index == 0U)
    {
        g_ui_page = UI_PAGE_GIF_PLAYER;
        App_ST7789_AnimRestart();
    }
    else
    {
        g_ui_menu_index = 1U;
        g_ui_redraw_pending = 1U;
    }
}

/*
 * Handle one logical UI event.
 *
 * Global reset and home handling runs first. The home page moves focus and
 * enters pages, the GIF page toggles or restarts playback, and settings uses
 * LEFT/RST to return home.
 *
 * Parameters:
 * event: Logical UI event from the fixed queue.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * May change page state, playback state, redraw state, or reset the MCU.
 */
static void App_UI_HandleEvent(const UI_Event *event)
{
    if (event->type == UI_EVENT_SYSTEM_RESET)
    {
        NVIC_SystemReset();
    }
    else if (event->type == UI_EVENT_HOME)
    {
        App_UI_GoHome();
    }
    else if (event->type == UI_EVENT_SETTINGS)
    {
        g_ui_page = UI_PAGE_HOME;
        g_ui_menu_index = 1U;
        g_ui_redraw_pending = 1U;
    }
    else if (g_ui_page == UI_PAGE_HOME)
    {
        if ((event->type == UI_EVENT_UP) || (event->type == UI_EVENT_DOWN))
        {
            g_ui_menu_index ^= 1U;
            g_ui_redraw_pending = 1U;
        }
        else if ((event->type == UI_EVENT_OK) || (event->type == UI_EVENT_RIGHT))
        {
            App_UI_EnterSelected();
        }
        else if ((event->type == UI_EVENT_LEFT) || (event->type == UI_EVENT_BACK))
        {
            g_ui_menu_index = 0U;
            g_ui_redraw_pending = 1U;
        }
    }
    else if (g_ui_page == UI_PAGE_GIF_PLAYER)
    {
        if (event->type == UI_EVENT_OK)
        {
            App_ST7789_AnimTogglePaused();
        }
        else if (event->type == UI_EVENT_RIGHT)
        {
            App_ST7789_AnimRestart();
        }
        else if ((event->type == UI_EVENT_LEFT) || (event->type == UI_EVENT_BACK))
        {
            App_UI_GoHome();
        }
    }
}

/*
 * Draw a pending static UI page.
 *
 * The GIF page is owned by the animation renderer, so this function redraws
 * only static pages and then clears the dirty flag.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * May replace the full ST7789 image.
 */
static void App_UI_FlushRedraw(void)
{
    if (g_ui_redraw_pending == 0U)
    {
        return;
    }

    g_ui_redraw_pending = 0U;
    if (g_ui_page == UI_PAGE_HOME)
    {
        App_UI_DrawHome();
    }
}

/*
 * Initialize the lightweight UI application state.
 *
 * Startup displays HOME instead of auto-playing GIF. GIF playback is still
 * preserved and starts when the first menu item is entered.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Clears UI input state and draws the home page.
 */
void App_UI_Init(void)
{
    UI_EventQueueInit();
    g_ui_page = UI_PAGE_HOME;
    g_ui_menu_index = 0U;
    g_ui_redraw_pending = 1U;
    App_UI_FlushRedraw();
}

/*
 * Service queued UI input and the active page task.
 *
 * Each call handles a bounded number of events, flushes one pending static
 * redraw, and advances GIF playback only while the GIF page is active.
 *
 * Parameters:
 * now: Current Timing_GetTick timestamp.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Handles input, updates display state, and may advance GIF DMA playback.
 */
void App_UI_Task(uint32_t now)
{
    uint8_t processed;
    UI_Event event;

    (void)now;

    for (processed = 0U; processed < UI_MAX_EVENTS_PER_TASK; processed++)
    {
        if (UI_EventPop(&event) == 0U)
        {
            break;
        }
        App_UI_HandleEvent(&event);
    }

    App_UI_FlushRedraw();

    if (g_ui_page == UI_PAGE_GIF_PLAYER)
    {
        App_ST7789_AnimTask();
    }
}
