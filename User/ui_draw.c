#include "ui_draw.h"
#include "bsp_st7789.h"
#include "ui_font.h"

static UI_Rect g_ui_draw_clip =
{
    0,
    0,
    (int16_t)UI_SCREEN_W,
    (int16_t)UI_SCREEN_H
};

/*
 * Clear the full visible display.
 *
 * Parameters:
 * color: RGB565 fill color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Replaces the full ST7789 visible area.
 */
void UI_DrawClear(uint16_t color)
{
    ST7789_Clear(color);
}

/*
 * Set the current drawing clip rectangle.
 *
 * All later UI drawing primitives intersect their target area with this clip.
 * Passing NULL restores a full-screen clip. The clip is intended for dirty
 * rectangle redraw, not for arbitrary nested clipping.
 *
 * Parameters:
 * clip: Rectangle to constrain drawing, or NULL for full screen.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Updates module-local drawing state.
 */
void UI_DrawSetClip(const UI_Rect *clip)
{
    if (clip == 0)
    {
        g_ui_draw_clip.x = 0;
        g_ui_draw_clip.y = 0;
        g_ui_draw_clip.w = (int16_t)UI_SCREEN_W;
        g_ui_draw_clip.h = (int16_t)UI_SCREEN_H;
        return;
    }

    g_ui_draw_clip = *clip;
}

/*
 * Clear only the current clip rectangle.
 *
 * Parameters:
 * color: RGB565 fill color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Repaints the active clipped area.
 */
void UI_DrawClearClip(uint16_t color)
{
    UI_DrawRect(g_ui_draw_clip.x, g_ui_draw_clip.y, g_ui_draw_clip.w, g_ui_draw_clip.h, color);
}

/*
 * Draw a clipped filled rectangle.
 *
 * Parameters:
 * x: Left coordinate.
 * y: Top coordinate.
 * w: Width in pixels.
 * h: Height in pixels.
 * color: RGB565 fill color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws within the ST7789 visible area.
 */
void UI_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    int16_t right;
    int16_t bottom;
    int16_t clip_right;
    int16_t clip_bottom;

    if ((w <= 0) || (h <= 0) || (x >= (int16_t)UI_SCREEN_W) || (y >= (int16_t)UI_SCREEN_H))
    {
        return;
    }
    if (x < 0)
    {
        w = (int16_t)(w + x);
        x = 0;
    }
    if (y < 0)
    {
        h = (int16_t)(h + y);
        y = 0;
    }
    if ((x + w) > (int16_t)UI_SCREEN_W)
    {
        w = (int16_t)((int16_t)UI_SCREEN_W - x);
    }
    if ((y + h) > (int16_t)UI_SCREEN_H)
    {
        h = (int16_t)((int16_t)UI_SCREEN_H - y);
    }

    right = (int16_t)(x + w);
    bottom = (int16_t)(y + h);
    clip_right = (int16_t)(g_ui_draw_clip.x + g_ui_draw_clip.w);
    clip_bottom = (int16_t)(g_ui_draw_clip.y + g_ui_draw_clip.h);
    if ((right <= g_ui_draw_clip.x) || (bottom <= g_ui_draw_clip.y) ||
        (x >= clip_right) || (y >= clip_bottom))
    {
        return;
    }

    if (x < g_ui_draw_clip.x)
    {
        x = g_ui_draw_clip.x;
    }
    if (y < g_ui_draw_clip.y)
    {
        y = g_ui_draw_clip.y;
    }
    if (right > clip_right)
    {
        right = clip_right;
    }
    if (bottom > clip_bottom)
    {
        bottom = clip_bottom;
    }
    w = (int16_t)(right - x);
    h = (int16_t)(bottom - y);

    if ((w > 0) && (h > 0))
    {
        ST7789_FillRect((uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
    }
}

/*
 * Draw a one-pixel rectangle frame.
 *
 * Parameters:
 * x: Left coordinate.
 * y: Top coordinate.
 * w: Width in pixels.
 * h: Height in pixels.
 * color: RGB565 frame color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws four clipped rectangle edges.
 */
void UI_DrawFrame(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    UI_DrawRect(x, y, w, 1, color);
    UI_DrawRect(x, (int16_t)(y + h - 1), w, 1, color);
    UI_DrawRect(x, y, 1, h, color);
    UI_DrawRect((int16_t)(x + w - 1), y, 1, h, color);
}

/*
 * Draw one character inside an 8x12 cell.
 *
 * Parameters:
 * x: Left coordinate.
 * y: Top coordinate.
 * ch: ASCII character.
 * color: RGB565 foreground color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws glyph pixels with ST7789 rectangles.
 */
static void UI_DrawChar(int16_t x, int16_t y, char ch, uint16_t color)
{
    const uint8_t *glyph;
    uint8_t column;
    uint8_t row;

    glyph = UI_FontGetGlyph(ch);
    for (column = 0U; column < 5U; column++)
    {
        for (row = 0U; row < 7U; row++)
        {
            if ((glyph[column] & (uint8_t)(1U << row)) != 0U)
            {
                UI_DrawRect(
                    (int16_t)(x + 1 + column),
                    (int16_t)(y + 2 + row),
                    1,
                    1,
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
 * x: Left coordinate.
 * y: Top coordinate.
 * text: Null-terminated ASCII text.
 * color: RGB565 text color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws text on the ST7789.
 */
void UI_DrawText(int16_t x, int16_t y, const char *text, uint16_t color)
{
    while ((text != 0) && (*text != '\0') && (x < (int16_t)UI_SCREEN_W))
    {
        if (((x + (int16_t)UI_FONT_WIDTH) > g_ui_draw_clip.x) &&
            (x < (g_ui_draw_clip.x + g_ui_draw_clip.w)) &&
            ((y + (int16_t)UI_FONT_HEIGHT) > g_ui_draw_clip.y) &&
            (y < (g_ui_draw_clip.y + g_ui_draw_clip.h)))
        {
            UI_DrawChar(x, y, *text, color);
        }
        x = (int16_t)(x + (int16_t)UI_FONT_WIDTH);
        text++;
    }
}

/*
 * Draw the standard top status bar.
 *
 * Parameters:
 * title: Page title.
 * accent_color: RGB565 bar color.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws the top status band.
 */
void UI_DrawStatusBar(const char *title, uint16_t accent_color)
{
    UI_DrawRect(0, 0, (int16_t)UI_SCREEN_W, (int16_t)UI_STATUS_H, accent_color);
    UI_DrawText((int16_t)UI_MARGIN, 6, title, UI_COLOR_BG);
    UI_DrawRect(218, 7, 10, 10, UI_COLOR_BG);
}

/*
 * Draw the standard footer hint band.
 *
 * Parameters:
 * hint: Compact key hint text.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws the bottom hint area.
 */
void UI_DrawFooter(const char *hint)
{
    UI_DrawRect(
        0,
        (int16_t)(UI_SCREEN_H - UI_FOOTER_H),
        (int16_t)UI_SCREEN_W,
        (int16_t)UI_FOOTER_H,
        UI_COLOR_SURFACE
    );
    UI_DrawText((int16_t)UI_MARGIN, 224, hint, UI_COLOR_MUTED);
}

/*
 * Draw a menu or setting row.
 *
 * Parameters:
 * x: Left coordinate.
 * y: Top coordinate.
 * w: Row width.
 * label: Left label text.
 * value: Optional right value text.
 * selected: Nonzero when focused.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws a complete row control.
 */
void UI_DrawMenuRow(
    int16_t x,
    int16_t y,
    int16_t w,
    const char *label,
    const char *value,
    uint8_t selected
)
{
    uint16_t fill;
    uint16_t text;

    fill = (selected != 0U) ? UI_COLOR_SELECTED : UI_COLOR_SURFACE;
    text = (selected != 0U) ? UI_COLOR_BG : UI_COLOR_TEXT;
    UI_DrawRect(x, y, w, (int16_t)UI_ROW_H, fill);
    UI_DrawRect(x, y, 4, (int16_t)UI_ROW_H, UI_COLOR_ACCENT);
    UI_DrawText((int16_t)(x + 14), (int16_t)(y + 12), label, text);
    if (value != 0)
    {
        UI_DrawText((int16_t)(x + w - 70), (int16_t)(y + 12), value, text);
    }
}

/*
 * Draw a horizontal progress bar.
 *
 * Parameters:
 * x: Left coordinate.
 * y: Top coordinate.
 * w: Width in pixels.
 * value: Current value.
 * max: Maximum value, must be nonzero.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws the progress track and filled region.
 */
void UI_DrawProgressBar(int16_t x, int16_t y, int16_t w, uint8_t value, uint8_t max)
{
    int16_t fill;

    if (max == 0U)
    {
        return;
    }

    fill = (int16_t)(((uint16_t)w * value) / max);
    UI_DrawRect(x, y, w, 10, UI_COLOR_SURFACE_2);
    UI_DrawRect(x, y, fill, 10, UI_COLOR_ACCENT);
    UI_DrawFrame(x, y, w, 10, UI_COLOR_MUTED);
}

/*
 * Draw a compact on/off toggle.
 *
 * Parameters:
 * x: Left coordinate.
 * y: Top coordinate.
 * enabled: Nonzero for ON.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Draws the toggle control.
 */
void UI_DrawToggle(int16_t x, int16_t y, uint8_t enabled)
{
    UI_DrawRect(x, y, 30, 14, (enabled != 0U) ? UI_COLOR_OK : UI_COLOR_SURFACE_2);
    UI_DrawRect(
        (enabled != 0U) ? (int16_t)(x + 17) : (int16_t)(x + 3),
        (int16_t)(y + 3),
        10,
        8,
        UI_COLOR_TEXT
    );
}
