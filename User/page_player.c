#include "page_player.h"
#include "ui_draw.h"

#define TEXT_PLAYER_TITLE       "\xB2\xA5\xB7\xC5"
#define TEXT_GIF_DISABLED      "GIF \xBD\xFB\xD3\xC3\xD6\xD0"
#define TEXT_STATE             "\xD7\xB4\xCC\xAC"
#define TEXT_PLAYING           "\xB2\xA5\xB7\xC5"
#define TEXT_PAUSED            "\xD4\xDD\xCD\xA3"
#define TEXT_STOPPED           "\xCD\xA3\xD6\xB9"
#define TEXT_FOOTER_PLAYER     "\xC8\xB7\xC8\xCF\xB2\xA5\xB7\xC5  \xD7\xF3\xBC\xFC\xB7\xB5\xBB\xD8"

typedef enum
{
    PLAYER_STATE_STOPPED = 0,             // Placeholder is stopped.
    PLAYER_STATE_PLAYING,                 // Simulated playback is running.
    PLAYER_STATE_PAUSED                   // Simulated playback is paused.
} PlayerState;

static PlayerState g_player_state = PLAYER_STATE_STOPPED;
static uint8_t g_player_progress = 0U;

/*
 * Mark the dynamic player state area dirty.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Queues a local repaint covering state text and the progress bar.
 */
static void Page_Player_InvalidateState(void)
{
    UI_PageInvalidateXYWH(24, 158, 192, 44);
}

/*
 * Enter the player placeholder page.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Resets the simulated player state.
 */
static void Page_Player_OnEnter(void)
{
    g_player_state = PLAYER_STATE_STOPPED;
    g_player_progress = 0U;
}

/*
 * Handle player placeholder events.
 *
 * Parameters:
 * event: UI event after global routing.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates simulated playback state and requests local repaint.
 */
static void Page_Player_OnEvent(const UI_Event *event)
{
    if (event->type == UI_EVENT_LEFT)
    {
        UI_PageBack();
    }
    else if (event->type == UI_EVENT_OK)
    {
        if (g_player_state == PLAYER_STATE_PLAYING)
        {
            g_player_state = PLAYER_STATE_PAUSED;
        }
        else
        {
            g_player_state = PLAYER_STATE_PLAYING;
        }
        Page_Player_InvalidateState();
    }
    else if (event->type == UI_EVENT_RIGHT)
    {
        g_player_state = PLAYER_STATE_PLAYING;
        g_player_progress = 0U;
        Page_Player_InvalidateState();
    }
}

/*
 * Draw the player placeholder page within the requested clip.
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
static void Page_Player_Draw(const UI_Rect *clip)
{
    const char *state_text;

    (void)clip;

    state_text = TEXT_STOPPED;
    if (g_player_state == PLAYER_STATE_PLAYING)
    {
        state_text = TEXT_PLAYING;
        g_player_progress = 36U;
    }
    else if (g_player_state == PLAYER_STATE_PAUSED)
    {
        state_text = TEXT_PAUSED;
    }

    UI_DrawStatusBar(TEXT_PLAYER_TITLE, UI_COLOR_ACCENT);
    UI_DrawRect(18, 44, 204, 104, UI_COLOR_SURFACE);
    UI_DrawFrame(34, 60, 172, 56, UI_COLOR_MUTED);
    UI_DrawTextCN(48, 80, TEXT_GIF_DISABLED, UI_COLOR_WARN);
    UI_DrawTextCN(30, 164, TEXT_STATE, UI_COLOR_MUTED);
    UI_DrawTextCN(94, 164, state_text, UI_COLOR_TEXT);
    UI_DrawProgressBar(30, 190, 180, g_player_progress, 100U);
    UI_DrawFooter(TEXT_FOOTER_PLAYER);
}

const UI_PageOps PAGE_PLAYER_OPS =
{
    Page_Player_OnEnter,
    Page_Player_OnEvent,
    Page_Player_Draw
};
