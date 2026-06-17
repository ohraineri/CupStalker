#ifndef CUP_STALKER_DISPLAY_H
#define CUP_STALKER_DISPLAY_H

#include "../model/match.h"
#include "tabs.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    DISPLAY_MODE_STEALTH,
    DISPLAY_MODE_VIVID
} DisplayMode;

typedef struct {
    const Match *matches;
    int          count;
    int          selected_index;
    bool         show_details;
    const char  *banner;
    AppTab       active_tab;

    const char (*log_lines)[160];
    int          log_count;
} DisplayContext;

void display_render(DisplayMode mode, const DisplayContext *context);
void display_render_stealth(const DisplayContext *context);
void display_render_vivid(const DisplayContext *context);
void display_format_goal_log(char *out, size_t size,
                             const Match *match, const MatchEvent *event);
void display_log_goal_stealth(const Match *match, const MatchEvent *event);
void display_format_goal_banner(char *out, size_t size,
                                const Match *match, const MatchEvent *event);

#endif
