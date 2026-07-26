#include "app_ui.h"
#include "bsp_st7789.h"
#include "ui_event.h"

#define UI_MAX_EVENTS_PER_TASK     4U      // Events handled per main-loop pass.
#define UI_MENU_COUNT              3U      // Player, settings, and info entries.

#define UI_COLOR_BG                0x0841U // Dark neutral background.
#define UI_COLOR_SURFACE           0x2104U // Card and panel color.
#define UI_COLOR_SURFACE_2         0x3186U // Secondary panel color.
#define UI_COLOR_SELECTED          0xFD20U // Focus color.
#define UI_COLOR_TEXT              0xFFFFU // Primary text color.
#define UI_COLOR_MUTED             0xA514U // Secondary text color.
#define UI_COLOR_ACCENT            0x07FFU // Cyan accent.
#define UI_COLOR_OK                0x07E0U // Green status color.
#define UI_COLOR_WARN              0xFBE0U // Yellow status color.
#define UI_COLOR_DANGER            0xF800U // Red status color.

typedef enum
{
    UI_PAGE_HOME = 0,                      // Main menu page.
    UI_PAGE_PLAYER,                        // UI-only player mock page.
    UI_PAGE_SETTINGS,                      // UI-only settings page.
    UI_PAGE_INFO                           // UI-only system info page.
} UI_PageId;

static UI_PageId g_ui_page = UI_PAGE_HOME;
static uint8_t g_ui_menu_index = 0U;
static uint8_t g_ui_redraw_pending = 0U;
static uint8_t g_ui_player_paused = 0U;
static uint8_t g_ui_setting_index = 0U;
static uint8_t g_ui_anim_enabled = 1U;
static uint8_t g_ui_brightness = 3U;

/*
 * Return a compact 5x7 uppercase glyph column bitmap.
 *
 * The UI-first build keeps text readable without linking a large font table.
 * Only the letters and digits used by the current English UI labels are
 * encoded. Missing characters render as spaces.
 *
 * Parameters:
 * ch: ASCII character to draw.
 * column: Glyph column from 0 to 4.
 *
 * Return value:
 * Seven-bit vertical column bitmap, bit 0 at the top.
 *
 * Side effects:
 * None.
 */
static uint8_t App_UI_GlyphColumn(char ch, uint8_t column)
{
    static const uint8_t digits[10][5] =
    {
        {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU},
        {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U},
        {0x62U, 0x51U, 0x49U, 0x49U, 0x46U},
        {0x22U, 0x49U, 0x49U, 0x49U, 0x36U},
        {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U},
        {0x2FU, 0x49U, 0x49U, 0x49U, 0x31U},
        {0x3EU, 0x49U, 0x49U, 0x49U, 0x32U},
        {0x01U, 0x71U, 0x09U, 0x05U, 0x03U},
        {0x36U, 0x49U, 0x49U, 0x49U, 0x36U},
        {0x26U, 0x49U, 0x49U, 0x49U, 0x3EU}
    };
    static const uint8_t letters[26][5] =
    {
        {0x7EU, 0x09U, 0x09U, 0x09U, 0x7EU},
        {0x7FU, 0x49U, 0x49U, 0x49U, 0x36U},
        {0x3EU, 0x41U, 0x41U, 0x41U, 0x22U},
        {0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU},
        {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U},
        {0x7FU, 0x09U, 0x09U, 0x09U, 0x01U},
        {0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU},
        {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU},
        {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U},
        {0x20U, 0x40U, 0x41U, 0x3FU, 0x01U},
        {0x7FU, 0x08U, 0x14U, 0x22U, 0x41U},
        {0x7FU, 0x40U, 0x40U, 0x40U, 0x40U},
        {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU},
        {0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU},
        {0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU},
        {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U},
        {0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU},
        {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U},
        {0x46U, 0x49U, 0x49U, 0x49U, 0x31U},
        {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U},
        {0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU},
        {0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU},
        {0x7FU, 0x20U, 0x18U, 0x20U, 0x7FU},
        {0x63U, 0x14U, 0x08U, 0x14U, 0x63U},
        {0x07U, 0x08U, 0x70U, 0x08U, 0x07U},
        {0x61U, 0x51U, 0x49U, 0x45U, 0x43U}
    };

    if (column >= 5U)
    {
        return 0U;
    }
    if ((ch >= '0') && (ch <= '9'))
    {
        return digits[ch - '0'][column];
    }
    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch = (char)(ch - 'a' + 'A');
    }
    if ((ch >= 'A') && (ch <= 'Z'))
    {
        return letters[ch - 'A'][column];
    }
    if (ch == '-')
    {
        return (column == 2U) ? 0x08U : 0U;
    }
    if (ch == ':')
    {
        return (column == 2U) ? 0x14U : 0U;
    }
    return 0U;
}

/*
 * Draw one scaled 5x7 ASCII character with rectangle pixels.
 *
 * The renderer trades speed for tiny Flash use and uses the existing
 * `ST7789_FillRect` primitive. Text is meant for static UI pages, not for
 * high-frequency animation.
 *
 * Parameters:
 * x: Left pixel coordinate.
 * y: Top pixel coordinate.
 * ch: ASCII character to draw.
 * color: RGB565 foreground color.
 * scale: Pixel scale, normally 2 for page labels.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws foreground glyph pixels on the ST7789.
 */
static void App_UI_DrawChar(
    uint16_t x,
    uint16_t y,
    char ch,
    uint16_t color,
    uint8_t scale
)
{
    uint8_t column;
    uint8_t row;

    for (column = 0U; column < 5U; column++)
    {
        uint8_t bits;

        bits = App_UI_GlyphColumn(ch, column);
        for (row = 0U; row < 7U; row++)
        {
            if ((bits & (uint8_t)(1U << row)) != 0U)
            {
                ST7789_FillRect(
                    (uint16_t)(x + ((uint16_t)column * scale)),
                    (uint16_t)(y + ((uint16_t)row * scale)),
                    scale,
                    scale,
                    color
                );
            }
        }
    }
}

/*
 * Draw a null-terminated ASCII string.
 *
 * Parameters:
 * x: Left pixel coordinate.
 * y: Top pixel coordinate.
 * text: Null-terminated text.
 * color: RGB565 foreground color.
 * scale: Pixel scale.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws text on the ST7789.
 */
static void App_UI_DrawText(
    uint16_t x,
    uint16_t y,
    const char *text,
    uint16_t color,
    uint8_t scale
)
{
    while ((text != 0) && (*text != '\0'))
    {
        App_UI_DrawChar(x, y, *text, color, scale);
        x = (uint16_t)(x + (6U * scale));
        text++;
    }
}

/*
 * Draw one home menu row.
 *
 * Parameters:
 * index: Menu item index.
 * y: Top coordinate.
 * label: Row label.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws the row background and label.
 */
static void App_UI_DrawMenuRow(uint8_t index, uint16_t y, const char *label)
{
    uint16_t color;

    color = (g_ui_menu_index == index) ? UI_COLOR_SELECTED : UI_COLOR_SURFACE;
    ST7789_FillRect(18U, y, 204U, 36U, color);
    ST7789_FillRect(18U, y, 4U, 36U, UI_COLOR_ACCENT);
    App_UI_DrawText(34U, (uint16_t)(y + 11U), label, UI_COLOR_TEXT, 2U);
}

/*
 * Draw the UI-first home menu.
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
    ST7789_Clear(UI_COLOR_BG);
    ST7789_FillRect(0U, 0U, ST7789_WIDTH, 24U, UI_COLOR_ACCENT);
    App_UI_DrawText(18U, 6U, "UI HOME", UI_COLOR_BG, 2U);
    App_UI_DrawMenuRow(0U, 48U, "PLAYER");
    App_UI_DrawMenuRow(1U, 94U, "SETTINGS");
    App_UI_DrawMenuRow(2U, 140U, "INFO");
    App_UI_DrawText(22U, 214U, "UP DOWN MID RST", UI_COLOR_MUTED, 1U);
}

/*
 * Draw the UI-only player page.
 *
 * The GIF data is deliberately excluded from this build, so this page focuses
 * on validating layout, navigation, and play/pause state feedback.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Replaces the full ST7789 image with the player page.
 */
static void App_UI_DrawPlayer(void)
{
    uint16_t state_color;

    state_color = (g_ui_player_paused == 0U) ? UI_COLOR_OK : UI_COLOR_WARN;
    ST7789_Clear(UI_COLOR_BG);
    ST7789_FillRect(0U, 0U, ST7789_WIDTH, 24U, UI_COLOR_ACCENT);
    App_UI_DrawText(18U, 6U, "PLAYER", UI_COLOR_BG, 2U);
    ST7789_FillRect(26U, 48U, 188U, 112U, UI_COLOR_SURFACE);
    ST7789_FillRect(44U, 72U, 152U, 56U, UI_COLOR_SURFACE_2);
    ST7789_FillRect(44U, 142U, 152U, 8U, state_color);
    App_UI_DrawText(58U, 88U, (g_ui_player_paused == 0U) ? "RUNNING" : "PAUSED", UI_COLOR_TEXT, 2U);
    App_UI_DrawText(22U, 188U, "MID PAUSE", UI_COLOR_MUTED, 1U);
    App_UI_DrawText(22U, 204U, "LEFT BACK", UI_COLOR_MUTED, 1U);
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
 * Replaces the full ST7789 image with the settings page.
 */
static void App_UI_DrawSettings(void)
{
    uint16_t first;
    uint16_t second;

    first = (g_ui_setting_index == 0U) ? UI_COLOR_SELECTED : UI_COLOR_SURFACE;
    second = (g_ui_setting_index == 1U) ? UI_COLOR_SELECTED : UI_COLOR_SURFACE;
    ST7789_Clear(UI_COLOR_BG);
    ST7789_FillRect(0U, 0U, ST7789_WIDTH, 24U, UI_COLOR_WARN);
    App_UI_DrawText(18U, 6U, "SETTINGS", UI_COLOR_BG, 2U);
    ST7789_FillRect(18U, 54U, 204U, 36U, first);
    App_UI_DrawText(34U, 65U, "ANIM", UI_COLOR_TEXT, 2U);
    App_UI_DrawText(154U, 65U, (g_ui_anim_enabled != 0U) ? "ON" : "OFF", UI_COLOR_TEXT, 2U);
    ST7789_FillRect(18U, 108U, 204U, 36U, second);
    App_UI_DrawText(34U, 119U, "LIGHT", UI_COLOR_TEXT, 2U);
    ST7789_FillRect(140U, 122U, (uint16_t)(g_ui_brightness * 14U), 10U, UI_COLOR_ACCENT);
    App_UI_DrawText(22U, 204U, "RIGHT CHANGE RST BACK", UI_COLOR_MUTED, 1U);
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
 * Replaces the full ST7789 image with the info page.
 */
static void App_UI_DrawInfo(void)
{
    ST7789_Clear(UI_COLOR_BG);
    ST7789_FillRect(0U, 0U, ST7789_WIDTH, 24U, UI_COLOR_OK);
    App_UI_DrawText(18U, 6U, "INFO", UI_COLOR_BG, 2U);
    App_UI_DrawText(28U, 52U, "STM32F103C8", UI_COLOR_TEXT, 2U);
    App_UI_DrawText(28U, 82U, "LCD 240X240", UI_COLOR_TEXT, 2U);
    App_UI_DrawText(28U, 112U, "KEY PA0-PA6", UI_COLOR_TEXT, 2U);
    App_UI_DrawText(28U, 142U, "GIF OFF", UI_COLOR_WARN, 2U);
    ST7789_FillRect(28U, 178U, 132U, 10U, UI_COLOR_ACCENT);
    App_UI_DrawText(22U, 212U, "RST BACK", UI_COLOR_MUTED, 1U);
}

/*
 * Request a redraw of the active page.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Marks the UI dirty.
 */
static void App_UI_RequestRedraw(void)
{
    g_ui_redraw_pending = 1U;
}

/*
 * Return to the home page.
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
    App_UI_RequestRedraw();
}

/*
 * Enter the currently selected home item.
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
static void App_UI_EnterSelected(void)
{
    switch (g_ui_menu_index)
    {
        case 0U:
            g_ui_page = UI_PAGE_PLAYER;
            break;

        case 1U:
            g_ui_page = UI_PAGE_SETTINGS;
            break;

        default:
            g_ui_page = UI_PAGE_INFO;
            break;
    }
    App_UI_RequestRedraw();
}

/*
 * Handle one settings-page event.
 *
 * Parameters:
 * event: Logical UI event from the queue.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates local setting values and marks the page dirty.
 */
static void App_UI_HandleSettingsEvent(const UI_Event *event)
{
    if ((event->type == UI_EVENT_UP) || (event->type == UI_EVENT_DOWN))
    {
        g_ui_setting_index ^= 1U;
        App_UI_RequestRedraw();
    }
    else if ((event->type == UI_EVENT_RIGHT) || (event->type == UI_EVENT_LEFT) ||
             (event->type == UI_EVENT_OK))
    {
        if (g_ui_setting_index == 0U)
        {
            g_ui_anim_enabled ^= 1U;
        }
        else
        {
            g_ui_brightness++;
            if (g_ui_brightness > 5U)
            {
                g_ui_brightness = 1U;
            }
        }
        App_UI_RequestRedraw();
    }
}

/*
 * Handle one logical UI event.
 *
 * Parameters:
 * event: Logical UI event from the fixed queue.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * May change page state, redraw state, or reset the MCU.
 */
static void App_UI_HandleEvent(const UI_Event *event)
{
    switch (event->type)
    {
        case UI_EVENT_SYSTEM_RESET:
            NVIC_SystemReset();
            break;

        case UI_EVENT_HOME:
            App_UI_GoHome();
            break;

        case UI_EVENT_SETTINGS:
            g_ui_page = UI_PAGE_SETTINGS;
            App_UI_RequestRedraw();
            break;

        case UI_EVENT_BACK:
        case UI_EVENT_LEFT:
            if (g_ui_page == UI_PAGE_HOME)
            {
                g_ui_menu_index = 0U;
                App_UI_RequestRedraw();
            }
            else
            {
                App_UI_GoHome();
            }
            break;

        default:
            if (g_ui_page == UI_PAGE_HOME)
            {
                if (event->type == UI_EVENT_UP)
                {
                    g_ui_menu_index = (g_ui_menu_index == 0U) ?
                        (UI_MENU_COUNT - 1U) :
                        (uint8_t)(g_ui_menu_index - 1U);
                    App_UI_RequestRedraw();
                }
                if (event->type == UI_EVENT_DOWN)
                {
                    g_ui_menu_index++;
                    if (g_ui_menu_index >= UI_MENU_COUNT)
                    {
                        g_ui_menu_index = 0U;
                    }
                    App_UI_RequestRedraw();
                }
                if ((event->type == UI_EVENT_OK) || (event->type == UI_EVENT_RIGHT))
                {
                    App_UI_EnterSelected();
                }
            }
            if (g_ui_page == UI_PAGE_PLAYER)
            {
                if (event->type == UI_EVENT_OK)
                {
                    g_ui_player_paused ^= 1U;
                    App_UI_RequestRedraw();
                }
                if (event->type == UI_EVENT_RIGHT)
                {
                    g_ui_player_paused = 0U;
                    App_UI_RequestRedraw();
                }
            }
            if (g_ui_page == UI_PAGE_SETTINGS)
            {
                App_UI_HandleSettingsEvent(event);
            }
            break;
    }
}

/*
 * Draw a pending UI page.
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
    switch (g_ui_page)
    {
        case UI_PAGE_HOME:
            App_UI_DrawHome();
            break;

        case UI_PAGE_PLAYER:
            App_UI_DrawPlayer();
            break;

        case UI_PAGE_SETTINGS:
            App_UI_DrawSettings();
            break;

        default:
            App_UI_DrawInfo();
            break;
    }
}

/*
 * Initialize the UI-first application state.
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
 * Service queued UI input.
 *
 * Parameters:
 * now: Current Timing_GetTick timestamp.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Handles input and redraws changed UI pages.
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
}
