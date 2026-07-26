#include "page_player.h"
#include "ui_draw.h"

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
    UI_PageInvalidateXYWH(30, 160, 180, 40);
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

    state_text = "STOPPED";
    if (g_player_state == PLAYER_STATE_PLAYING)
    {
        state_text = "PLAYING";
        g_player_progress = 36U;
    }
    else if (g_player_state == PLAYER_STATE_PAUSED)
    {
        state_text = "PAUSED";
    }

    UI_DrawStatusBar("PLAYER", UI_COLOR_ACCENT);
    UI_DrawRect(18, 42, 204, 112, UI_COLOR_SURFACE);
    UI_DrawFrame(34, 58, 172, 64, UI_COLOR_MUTED);
    UI_DrawText(56, 84, "GIF DISABLED", UI_COLOR_WARN);
    UI_DrawText(30, 164, "STATE", UI_COLOR_MUTED);
    UI_DrawText(92, 164, state_text, UI_COLOR_TEXT);
    UI_DrawProgressBar(30, 188, 180, g_player_progress, 100U);
    UI_DrawFooter("MID PLAY RIGHT RESET");
}

const UI_PageOps PAGE_PLAYER_OPS =
{
    Page_Player_OnEnter,
    Page_Player_OnEvent,
    Page_Player_Draw
};
