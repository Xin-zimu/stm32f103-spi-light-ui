#ifndef __UI_RENDERER_H
#define __UI_RENDERER_H

#include "ui_page.h"
#include "ui_types.h"

void UI_RendererInit(void);
void UI_RendererTask(uint32_t now, const UI_PageOps *page);
uint8_t UI_RendererIsBusy(void);
void UI_RendererRequestFull(void);

#endif
