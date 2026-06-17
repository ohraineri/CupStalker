#ifndef CUP_STALKER_TAB_RECENT_MATCHES_H
#define CUP_STALKER_TAB_RECENT_MATCHES_H

#include "../model/match.h"
#include "tabs.h"

#include <stdbool.h>

typedef struct {
    const Match *matches;
    int          count;
    int          scroll;
    const char  *status_message;
    bool         loaded;
    bool         colorize;
    AppTab       active_tab;
} RecentMatchesContext;

void tab_recent_matches_render(const RecentMatchesContext *context);

#endif
