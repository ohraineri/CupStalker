#include "display.h"
#include "terminal.h"

#include "../../config.h"

#include <stdio.h>
#include <time.h>

static void format_minute(char *out, size_t size, const Match *match)
{
    if (match->status != MATCH_STATUS_LIVE || match->minute < 0) {
        out[0] = '\0';
        return;
    }

    if (match->stoppage > 0) {
        snprintf(out, size, "[%d+%d']", match->minute, match->stoppage);
    } else {
        snprintf(out, size, "[%d']", match->minute);
    }
}

static void print_stealth_header(void)
{
    char timestamp[32] = "";
    time_t now = time(NULL);
    struct tm utc;
    if (gmtime_r(&now, &utc) != NULL) {
        strftime(timestamp, sizeof timestamp, "%Y-%m-%d %H:%M:%S UTC", &utc);
    }

    printf("[cup-stalker] %s | refresh: %ds\n", timestamp, REFRESH_INTERVAL_SECONDS);
    printf("-----------------------------------------------------\n");
}

static void print_stealth_line(const Match *match)
{
    char minute[16];
    format_minute(minute, sizeof minute, match);

    printf("%-3s %d x %d %-3s  %s%s%s\n",
           match->home.code, match->home_score, match->away_score, match->away.code,
           match_status_label(match->status),
           minute[0] ? "  " : "", minute);
}

static void print_stealth_events(const Match *match)
{
    for (int i = 0; i < match->event_count; ++i) {
        const MatchEvent *event = &match->events[i];
        printf("    %2d' %s%s%s\n", event->minute, event->scorer_name,
               event->is_penalty ? " (pen)" : "",
               event->is_own_goal ? " (og)" : "");
    }
}

void display_render_stealth(const DisplayContext *context)
{
    terminal_clear();
    tabs_render_bar(context->active_tab, false);
    print_stealth_header();

    for (int i = 0; i < context->count; ++i) {
        print_stealth_line(&context->matches[i]);
        if (context->show_details && i == context->selected_index) {
            print_stealth_events(&context->matches[i]);
        }
    }

    if (context->log_count > 0) {
        printf("-----------------------------------------------------\n");
        for (int i = 0; i < context->log_count; ++i) {
            printf("%s\n", context->log_lines[i]);
        }
    }

    printf("-----------------------------------------------------\n");
    printf("[s] vivid  [r] refresh  [d] details  [q] quit\n");
    fflush(stdout);
}

void display_format_goal_log(char *out, size_t size,
                             const Match *match, const MatchEvent *event)
{
    int minute = (event != NULL) ? event->minute : match->minute;
    const char *scorer = (event != NULL) ? event->scorer_name : "Unknown";

    snprintf(out, size, "[%d'] GOAL: %s %d-%d %s (%s)",
             minute, match->home.code, match->home_score,
             match->away_score, match->away.code, scorer);
}

void display_log_goal_stealth(const Match *match, const MatchEvent *event)
{
    char line[160];
    display_format_goal_log(line, sizeof line, match, event);
    printf("%s\n", line);
    fflush(stdout);
}

void display_render(DisplayMode mode, const DisplayContext *context)
{
    if (mode == DISPLAY_MODE_VIVID) {
        display_render_vivid(context);
    } else {
        display_render_stealth(context);
    }
}
