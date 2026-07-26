#ifndef __UI_DIRTY_H
#define __UI_DIRTY_H

#include "ui_types.h"

#define UI_DIRTY_MAX_RECTS       8U       // Maximum tracked local refresh areas.

void UI_DirtyInit(void);
void UI_DirtyAdd(const UI_Rect *rect);
void UI_DirtyAddIsolated(const UI_Rect *rect);
void UI_DirtyAddXYWH(int16_t x, int16_t y, int16_t w, int16_t h);
void UI_DirtyAddUnion(const UI_Rect *a, const UI_Rect *b);
void UI_DirtyFullScreen(void);
uint8_t UI_DirtyPop(UI_Rect *rect);
void UI_DirtyClear(void);

#endif
