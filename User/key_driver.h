#ifndef __KEY_DRIVER_H
#define __KEY_DRIVER_H

#include "stm32f10x.h"

#define KEY_SCAN_INTERVAL_MS      10U      // Key scan period in milliseconds.
#define KEY_DEBOUNCE_MS           20U      // Stable time required before state changes.
#define KEY_LONG_PRESS_MS        600U      // Long press event threshold.
#define KEY_REPEAT_DELAY_MS      350U      // First repeat delay for direction keys.
#define KEY_REPEAT_INTERVAL_MS   100U      // Repeat interval after the first repeat.
#define KEY_RST_RESET_MS        2000U      // RST hold time before system reset event.

typedef enum
{
    KEY_ID_UP = 0,                         // Joystick UP on PA0.
    KEY_ID_DOWN,                           // Joystick DWN on PA1.
    KEY_ID_LEFT,                           // Joystick LFT on PA2.
    KEY_ID_RIGHT,                          // Joystick RHT on PA3.
    KEY_ID_MID,                            // Joystick MID on PA4.
    KEY_ID_SET,                            // Joystick SET on PA5.
    KEY_ID_RST,                            // Joystick RST on PA6.
    KEY_ID_COUNT                           // Number of physical keys.
} KeyId;

typedef enum
{
    KEY_EVENT_NONE = 0,                    // No key event.
    KEY_EVENT_PRESS,                       // Stable transition to pressed.
    KEY_EVENT_RELEASE,                     // Stable transition to released.
    KEY_EVENT_LONG_PRESS,                  // Pressed longer than KEY_LONG_PRESS_MS.
    KEY_EVENT_REPEAT,                      // Direction-key repeat while held.
    KEY_EVENT_SYSTEM_RESET                 // RST held long enough for software reset.
} KeyEventType;

typedef struct
{
    KeyId key;                             // Physical key that generated the event.
    KeyEventType type;                     // Event kind.
    uint32_t timestamp;                    // Timing_GetTick timestamp.
} KeyEvent;

void Key_Init(void);
void Key_Task(uint32_t now);

#endif
