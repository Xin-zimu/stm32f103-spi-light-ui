#ifndef __ANIM_FRAMES_H
#define __ANIM_FRAMES_H

#include "stm32f10x.h"

#define ANIM_FRAME_WIDTH          120U    // Indexed source width
#define ANIM_FRAME_HEIGHT         120U    // Indexed source height
#define ANIM_PIXEL_SCALE          2U      // Output scale to 240x240
#define ANIM_FRAME_SIZE           7200U   // Packed four-bit bytes per frame
#define ANIM_FRAME_COUNT          3U      // Exported GIF frame count
#define ANIM_FRAME_INTERVAL_MS    100U    // Playback interval in milliseconds

extern const uint16_t anim_palette[16];
extern const uint8_t anim_frames[ANIM_FRAME_COUNT][ANIM_FRAME_SIZE];

#endif
