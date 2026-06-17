#ifndef CUP_STALKER_INPUT_HANDLER_H
#define CUP_STALKER_INPUT_HANDLER_H

typedef enum {
    INPUT_NONE,
    INPUT_TOGGLE_MODE,
    INPUT_REFRESH,
    INPUT_QUIT,
    INPUT_NAV_UP,
    INPUT_NAV_DOWN,
    INPUT_TOGGLE_DETAILS,
    INPUT_NEXT_TAB,
    INPUT_PREV_TAB
} InputAction;

InputAction input_handler_poll(void);

#endif
