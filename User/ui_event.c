#include "ui_event.h"

typedef struct
{
    UI_Event items[UI_EVENT_QUEUE_SIZE];   // Static queue storage.
    uint8_t head;                          // Next item to pop.
    uint8_t tail;                          // Next slot to push.
    uint8_t count;                         // Current queued item count.
} UI_EventQueue;

static UI_EventQueue g_ui_event_queue;
volatile uint32_t ui_event_drop_count = 0U;

/*
 * Reset the fixed UI event queue.
 *
 * The queue is used by the cooperative key and UI tasks, so it has no dynamic
 * allocation and no blocking behavior. Existing queued input is intentionally
 * discarded during application initialization or when the UI requests a clear.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Clears all queued UI events.
 */
void UI_EventQueueInit(void)
{
    g_ui_event_queue.head = 0U;
    g_ui_event_queue.tail = 0U;
    g_ui_event_queue.count = 0U;
    ui_event_drop_count = 0U;
}

/*
 * Push one logical UI event into the ring queue.
 *
 * When the queue is full the new event is dropped and a counter is advanced.
 * The queue is intentionally small and the UI task drains it every loop, which
 * keeps this first-stage implementation compact enough for the 64 KB target.
 *
 * Parameters:
 * event: Logical event to enqueue.
 *
 * Return value:
 * 1: Event was queued.
 * 0: Event was rejected because the queue was full.
 *
 * Side effects:
 * Increments ui_event_drop_count when the queue is full.
 */
uint8_t UI_EventPush(const UI_Event *event)
{
    if ((event == 0) || (event->type == UI_EVENT_NONE))
    {
        return 0U;
    }

    if (g_ui_event_queue.count >= UI_EVENT_QUEUE_SIZE)
    {
        ui_event_drop_count++;
        return 0U;
    }

    g_ui_event_queue.items[g_ui_event_queue.tail] = *event;
    g_ui_event_queue.tail++;
    if (g_ui_event_queue.tail >= UI_EVENT_QUEUE_SIZE)
    {
        g_ui_event_queue.tail = 0U;
    }
    g_ui_event_queue.count++;

    return 1U;
}

/*
 * Pop one logical UI event from the ring queue.
 *
 * The UI task calls this function a bounded number of times per main-loop
 * pass, preventing a burst of key repeat events from starving display work.
 *
 * Parameters:
 * event: Receives the oldest queued event.
 *
 * Return value:
 * 1: An event was popped.
 * 0: The queue was empty or the output pointer was invalid.
 *
 * Side effects:
 * Removes one queued event when available.
 */
uint8_t UI_EventPop(UI_Event *event)
{
    if ((event == 0) || (g_ui_event_queue.count == 0U))
    {
        return 0U;
    }

    *event = g_ui_event_queue.items[g_ui_event_queue.head];
    g_ui_event_queue.head++;
    if (g_ui_event_queue.head >= UI_EVENT_QUEUE_SIZE)
    {
        g_ui_event_queue.head = 0U;
    }
    g_ui_event_queue.count--;

    return 1U;
}

/*
 * Convert one debounced physical key event into a logical UI event.
 *
 * Mapping is centralized here so page code never reads GPIO directly. Press
 * and repeat events drive normal navigation, a 600 ms RST long press becomes
 * HOME, and the two second RST hold becomes SYSTEM_RESET for the global UI
 * handler.
 *
 * Parameters:
 * event: Debounced physical key event from key_driver.c.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * May enqueue one logical UI event.
 */
void UI_EventPushFromKey(const KeyEvent *event)
{
    UI_Event ui_event;

    if (event == 0)
    {
        return;
    }

    ui_event.type = UI_EVENT_NONE;

    if (event->type == KEY_EVENT_SYSTEM_RESET)
    {
        ui_event.type = UI_EVENT_SYSTEM_RESET;
    }
    else if ((event->key == KEY_ID_RST) && (event->type == KEY_EVENT_LONG_PRESS))
    {
        ui_event.type = UI_EVENT_HOME;
    }
    else if ((event->type == KEY_EVENT_PRESS) || (event->type == KEY_EVENT_REPEAT))
    {
        switch (event->key)
        {
            case KEY_ID_UP:
                ui_event.type = UI_EVENT_UP;
                break;

            case KEY_ID_DOWN:
                ui_event.type = UI_EVENT_DOWN;
                break;

            case KEY_ID_LEFT:
                ui_event.type = UI_EVENT_LEFT;
                break;

            case KEY_ID_RIGHT:
                ui_event.type = UI_EVENT_RIGHT;
                break;

            case KEY_ID_MID:
                ui_event.type = UI_EVENT_OK;
                break;

            case KEY_ID_SET:
                ui_event.type = UI_EVENT_SETTINGS;
                break;

            case KEY_ID_RST:
                ui_event.type = UI_EVENT_BACK;
                break;

            default:
                break;
        }
    }

    if (ui_event.type != UI_EVENT_NONE)
    {
        (void)UI_EventPush(&ui_event);
    }
}
