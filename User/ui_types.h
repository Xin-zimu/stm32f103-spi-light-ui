#ifndef __UI_TYPES_H
#define __UI_TYPES_H

#include "stm32f10x.h"

#define UI_SCREEN_W          240U     // ST7789 visible width.
#define UI_SCREEN_H          240U     // ST7789 visible height.
#define UI_STATUS_H           32U     // Top status bar height.
#define UI_FOOTER_H           28U     // Bottom hint area height.
#define UI_MARGIN             12U     // Standard outer margin.
#define UI_ROW_H              48U     // Standard row height.
#define UI_ROW_GAP             6U     // Gap between rows.

#define UI_COLOR_BG        0x0841U    // Dark neutral background.
#define UI_COLOR_SURFACE   0x2104U    // Primary surface color.
#define UI_COLOR_SURFACE_2 0x3186U    // Secondary surface color.
#define UI_COLOR_TEXT      0xFFFFU    // Primary text color.
#define UI_COLOR_MUTED     0xA514U    // Muted text and hint color.
#define UI_COLOR_DIM       0x632CU    // Low-emphasis divider color.
#define UI_COLOR_ACCENT    0x07FFU    // Cyan accent color.
#define UI_COLOR_SELECTED  0xFD20U    // Selected row color.
#define UI_COLOR_OK        0x07E0U    // Success state color.
#define UI_COLOR_WARN      0xFBE0U    // Warning state color.
#define UI_COLOR_DANGER    0xF800U    // Error or reset state color.

typedef struct
{
    int16_t x;                         // Left coordinate.
    int16_t y;                         // Top coordinate.
    int16_t w;                         // Width in pixels.
    int16_t h;                         // Height in pixels.
} UI_Rect;

typedef enum
{
    UI_PAGE_HOME = 0,                  // Main menu page.
    UI_PAGE_PLAYER,                    // Player placeholder page.
    UI_PAGE_SETTINGS,                  // Settings page.
    UI_PAGE_INFO,                      // System information page.
    UI_PAGE_COUNT                      // Number of registered pages.
} UI_PageId;

#endif
