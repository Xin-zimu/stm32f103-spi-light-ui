#ifndef __UI_FONT_H
#define __UI_FONT_H

#include "stm32f10x.h"

#define UI_FONT_WIDTH          8U      // Character cell width.
#define UI_FONT_HEIGHT        12U      // Character cell height.

const uint8_t *UI_FontGetGlyph(char ch);

#endif
