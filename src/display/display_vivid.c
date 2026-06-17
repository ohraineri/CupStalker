#include "ansi_colors.h"
#include "display.h"
#include "terminal.h"

#include "../../config.h"
#include "../util/string_utils.h"

#include <stdio.h>

#define VIVID_CONTENT_WIDTH 46

#define BOX_TOP    "╔"
#define BOX_BOTTOM "╚"
#define BOX_TR     "╗"
#define BOX_BR     "╝"
#define BOX_V      "║"
#define BOX_H      "═"
#define BOX_LSEP   "╠"
#define BOX_RSEP   "╣"

static void print_rule(const char *left, const char *right)
{
    fputs(left, stdout);
    for (int i = 0; i < VIVID_CONTENT_WIDTH + 2; ++i) {
        fputs(BOX_H, stdout);
    }
    printf("%s\n", right);
}

static void print_row(const char *content)
{
    size_t width = string_display_width(content);
    printf("%s %s", BOX_V, content);
    for (size_t i = width; i < (size_t)VIVID_CONTENT_WIDTH; ++i) {
        putchar(' ');
    }
    printf(" %s\n", BOX_V);
}

static const char *vivid_status_badge(MatchStatus status)
{
    switch (status) {
        case MATCH_STATUS_LIVE:      return "\U0001F534 AO VIVO";
        case MATCH_STATUS_FINISHED:  return "✅ ENCERRADO";
        case MATCH_STATUS_HALFTIME:  return "⏸ INTERVALO";
        case MATCH_STATUS_POSTPONED: return "⚠ ADIADO";
        case MATCH_STATUS_SCHEDULED: return "\U0001F550 EM BREVE";
    }
    return "";
}

static void print_score_row(const Match *match, bool selected)
{
    char row[256];
    const char *marker = selected ? ANSI_YELLOW "▶ " ANSI_RESET : "  ";

    snprintf(row, sizeof row,
             "%s%s  " ANSI_BOLD "%-10s" ANSI_RESET "  "
             ANSI_GREEN ANSI_BOLD "%d × %d" ANSI_RESET
             "  " ANSI_BOLD "%-10s" ANSI_RESET "  %s",
             marker, match->home.flag, match->home.name,
             match->home_score, match->away_score,
             match->away.name, match->away.flag);
    print_row(row);
}

static void print_goal_rows(const Match *match)
{
    for (int i = 0; i < match->event_count; ++i) {
        const MatchEvent *event = &match->events[i];
        char row[256];
        snprintf(row, sizeof row, "    ⚽ %s %d'%s%s",
                 event->scorer_name, event->minute,
                 event->is_penalty ? " (pen)" : "",
                 event->is_own_goal ? " (gc)" : "");
        print_row(row);
    }
}

static void print_meta_row(const Match *match)
{
    char row[128];
    const char *badge = vivid_status_badge(match->status);

    if (match->status == MATCH_STATUS_LIVE && match->minute >= 0) {
        snprintf(row, sizeof row, "    %s%s  |  \U0001F550 %d'",
                 match->phase, match->phase[0] ? "  |  " : "", match->minute);
    } else {
        snprintf(row, sizeof row, "    %s%s%s",
                 match->phase, match->phase[0] ? "  |  " : "", badge);
    }
    print_row(row);
}

static void print_match_panel(const Match *match, bool selected, bool show_details)
{
    print_score_row(match, selected);
    print_goal_rows(match);
    print_meta_row(match);

    if (show_details && selected && match->event_count == 0) {
        print_row("    (sem eventos registrados)");
    }
}

void display_render_vivid(const DisplayContext *context)
{
    terminal_clear();
    tabs_render_bar(context->active_tab, true);

    print_rule(BOX_TOP, BOX_TR);
    print_row(ANSI_BOLD ANSI_YELLOW "\U0001F3C6  CUP STALKER — AO VIVO \U0001F534" ANSI_RESET);

    if (context->banner != NULL && context->banner[0] != '\0') {
        print_rule(BOX_LSEP, BOX_RSEP);
        char banner[256];
        snprintf(banner, sizeof banner, ANSI_BG_GREEN ANSI_BOLD ANSI_BLINK "%s" ANSI_RESET,
                 context->banner);
        print_row(banner);
    }

    for (int i = 0; i < context->count; ++i) {
        print_rule(BOX_LSEP, BOX_RSEP);
        print_match_panel(&context->matches[i],
                          i == context->selected_index,
                          context->show_details);
    }

    print_rule(BOX_BOTTOM, BOX_BR);
    printf("  [s] stealth  [r] refresh  [↑↓] navegar  [d] detalhes  [q] sair\n");
    fflush(stdout);
}

void display_format_goal_banner(char *out, size_t size,
                                const Match *match, const MatchEvent *event)
{
    int minute = (event != NULL) ? event->minute : match->minute;
    const char *scorer = (event != NULL) ? event->scorer_name : "";

    if (scorer[0] != '\0') {
        snprintf(out, size, "⚽ GOL! %s %d × %d %s — %s %d'",
                 match->home.name, match->home_score, match->away_score,
                 match->away.name, scorer, minute);
    } else {
        snprintf(out, size, "⚽ GOL! %s %d × %d %s",
                 match->home.name, match->home_score,
                 match->away_score, match->away.name);
    }
}
