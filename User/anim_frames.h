#ifndef __ANIM_FRAMES_H
#define __ANIM_FRAMES_H

#include "stm32f10x.h"

#define ANIM_FRAME_WIDTH          128U    // 动画帧宽度
#define ANIM_FRAME_HEIGHT         64U     // 动画帧高度
#define ANIM_FRAME_SIZE           1024U   // 每帧页格式字节数
#define ANIM_FRAME_COUNT          12U     // GIF 转换后的动画帧数
#define ANIM_FRAME_INTERVAL_MS    100U    // 动画帧间隔，单位 ms

extern const uint8_t anim_frames[ANIM_FRAME_COUNT][ANIM_FRAME_SIZE];

#endif
