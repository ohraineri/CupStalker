#include "tabs.h"

#include "ansi_colors.h"

#include <stdio.h>

const char *tab_label(AppTab tab)
{
    switch (tab) {
        case APP_TAB_LIVE:   return "Live";
        case APP_TAB_RECENT: return "Recent Matches";
        case APP_TAB_COUNT:  break;
    }
    return "";
}

void tabs_render_bar(AppTab active, bool colorize)
{
    for (int i = 0; i < APP_TAB_COUNT; ++i) {
        const char *label = tab_label((AppTab)i);
        bool is_active = ((AppTab)i == active);

        if (colorize && is_active) {
            printf(" " ANSI_BOLD ANSI_YELLOW "[%s]" ANSI_RESET, label);
        } else if (is_active) {
            printf(" [%s]", label);
        } else {
            printf("  %s ", label);
        }
    }

    if (colorize) {
        printf("    " ANSI_DIM "[Tab] switch" ANSI_RESET "\n");
    } else {
        printf("    [Tab] switch\n");
    }
}
