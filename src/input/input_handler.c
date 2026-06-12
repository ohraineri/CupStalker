#include "input_handler.h"

#include <unistd.h>

static InputAction decode_escape_sequence(void)
{
    char bracket = 0;
    char code = 0;

    if (read(STDIN_FILENO, &bracket, 1) != 1 || bracket != '[') {
        return INPUT_NONE;
    }
    if (read(STDIN_FILENO, &code, 1) != 1) {
        return INPUT_NONE;
    }

    switch (code) {
        case 'A': return INPUT_NAV_UP;
        case 'B': return INPUT_NAV_DOWN;
        default:  return INPUT_NONE;
    }
}

InputAction input_handler_poll(void)
{
    char key = 0;
    if (read(STDIN_FILENO, &key, 1) != 1) {
        return INPUT_NONE;
    }

    switch (key) {
        case 's': return INPUT_TOGGLE_MODE;
        case 'r': return INPUT_REFRESH;
        case 'q': return INPUT_QUIT;
        case 'd': return INPUT_TOGGLE_DETAILS;
        case '\033': return decode_escape_sequence();
        default: return INPUT_NONE;
    }
}
