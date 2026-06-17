#ifndef CUP_STALKER_TABS_H
#define CUP_STALKER_TABS_H

#include <stdbool.h>

typedef enum {
    APP_TAB_LIVE,
    APP_TAB_RECENT,
    APP_TAB_COUNT
} AppTab;

const char *tab_label(AppTab tab);
void tabs_render_bar(AppTab active, bool colorize);

#endif
