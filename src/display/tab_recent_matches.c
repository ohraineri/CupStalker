#include "tab_recent_matches.h"

#include "ansi_colors.h"
#include "terminal.h"

#include "../../config.h"
#include "../util/string_utils.h"

#include <stdio.h>

#define RECENT_CONTENT_WIDTH 53

#define BOX_TOP    "┌"
#define BOX_BOTTOM "└"
#define BOX_TR     "┐"
#define BOX_BR     "┘"
#define BOX_V      "│"
#define BOX_H      "─"
#define BOX_LSEP   "├"
#define BOX_RSEP   "┤"

static void print_rule(const char *left, const char *right)
{
    fputs(left, stdout);
    for (int i = 0; i < RECENT_CONTENT_WIDTH + 2; ++i) {
        fputs(BOX_H, stdout);
    }
    printf("%s\n", right);
}

static void print_row(const char *content)
{
    size_t width = string_display_width(content);
    printf("%s %s", BOX_V, content);
    for (size_t i = width; i < (size_t)RECENT_CONTENT_WIDTH; ++i) {
        putchar(' ');
    }
    printf(" %s\n", BOX_V);
}

static void format_date(char *out, size_t size, const char *iso)
{
    if (iso == NULL || iso[0] == '\0') {
        string_copy(out, size, "TBD");
        return;
    }

    string_copy(out, size, iso);
    for (char *cursor = out; *cursor != '\0'; ++cursor) {
        if (*cursor == 'T') {
            *cursor = ' ';
        }
        if (*cursor == 'Z') {
            *cursor = ' ';
        }
    }
}

static void print_status_row(const Match *match, bool colorize)
{
    char date[32];
    format_date(date, sizeof date, match->date_utc);

    const char *label = match_status_label(match->status);
    char status[48];
    if (colorize && match->status == MATCH_STATUS_LIVE) {
        snprintf(status, sizeof status, ANSI_GREEN ANSI_BOLD "%s" ANSI_RESET, label);
    } else if (match->status == MATCH_STATUS_LIVE) {
        snprintf(status, sizeof status, "*%s*", label);
    } else {
        string_copy(status, sizeof status, label);
    }

    char row[128];
    snprintf(row, sizeof row, "%-18s  %-19s  %s",
             match->phase[0] ? match->phase : "—", date, status);
    print_row(row);
}

static void print_score_row(const Match *match, bool colorize)
{
    char row[256];
    if (colorize) {
        snprintf(row, sizeof row,
                 ANSI_BOLD "%-16s" ANSI_RESET "  " ANSI_GREEN ANSI_BOLD "%d : %d" ANSI_RESET
                 "  " ANSI_BOLD "%s" ANSI_RESET,
                 match->home.name, match->home_score, match->away_score, match->away.name);
    } else {
        snprintf(row, sizeof row, "%-16s  %d : %d  %s",
                 match->home.name, match->home_score, match->away_score, match->away.name);
    }
    print_row(row);
}

static void print_venue_row(const Match *match)
{
    char row[160];
    if (match->venue[0] && match->city[0]) {
        snprintf(row, sizeof row, "%s, %s", match->venue, match->city);
    } else if (match->venue[0]) {
        string_copy(row, sizeof row, match->venue);
    } else if (match->city[0]) {
        string_copy(row, sizeof row, match->city);
    } else {
        string_copy(row, sizeof row, "Venue unknown");
    }
    print_row(row);
}

static void print_goal_rows(const Match *match, bool colorize)
{
    if (match->event_count == 0) {
        print_row("  No goals recorded");
        return;
    }

    const char *icon = colorize ? "⚽" : "*";
    for (int i = 0; i < match->event_count; ++i) {
        const MatchEvent *event = &match->events[i];
        char row[160];
        snprintf(row, sizeof row, "  %s %d'  %s (%s)%s%s",
                 icon, event->minute, event->scorer_name,
                 event->team_code[0] ? event->team_code : "?",
                 event->is_penalty ? " [pen]" : "",
                 event->is_own_goal ? " [og]" : "");
        print_row(row);
    }
}

static void card_glyph(CardType type, bool colorize, char *out, size_t size)
{
    if (colorize) {
        switch (type) {
            case CARD_YELLOW:     string_copy(out, size, "🟨");   return;
            case CARD_RED:        string_copy(out, size, "🟥");   return;
            case CARD_YELLOW_RED: string_copy(out, size, "🟨🟥"); return;
        }
    } else {
        switch (type) {
            case CARD_YELLOW:     string_copy(out, size, "Y");   return;
            case CARD_RED:        string_copy(out, size, "R");   return;
            case CARD_YELLOW_RED: string_copy(out, size, "Y/R"); return;
        }
    }
    string_copy(out, size, "?");
}

static void print_card_rows(const Match *match, bool colorize)
{
    if (match->card_count == 0) {
        print_row("  No cards");
        return;
    }

    for (int i = 0; i < match->card_count; ++i) {
        const MatchCard *card = &match->cards[i];
        char glyph[16];
        card_glyph(card->type, colorize, glyph, sizeof glyph);

        char row[160];
        snprintf(row, sizeof row, "  %s %d'  %s (%s)",
                 glyph, card->minute, card->player_name,
                 card->team_code[0] ? card->team_code : "?");
        print_row(row);
    }
}

static void print_match_panel(const Match *match, bool colorize)
{
    print_rule(BOX_TOP, BOX_TR);
    print_status_row(match, colorize);
    print_score_row(match, colorize);
    print_venue_row(match);
    print_rule(BOX_LSEP, BOX_RSEP);
    print_row("GOALS");
    print_goal_rows(match, colorize);
    print_row("CARDS");
    print_card_rows(match, colorize);
    print_rule(BOX_BOTTOM, BOX_BR);
}

static void print_message_panel(const char *message)
{
    print_rule(BOX_TOP, BOX_TR);
    char row[160];
    snprintf(row, sizeof row, "  %s", message);
    print_row(row);
    print_rule(BOX_BOTTOM, BOX_BR);
}

void tab_recent_matches_render(const RecentMatchesContext *context)
{
    terminal_clear();
    tabs_render_bar(context->active_tab, context->colorize);

    if (!context->loaded) {
        print_message_panel("Loading…");
        printf("  [Tab] switch  [q] quit\n");
        fflush(stdout);
        return;
    }

    if (context->status_message != NULL) {
        print_message_panel(context->status_message);
    }

    if (context->count > 0) {
        int first = context->scroll;
        int last = first + RECENT_VISIBLE_MATCHES;
        if (last > context->count) {
            last = context->count;
        }

        for (int i = first; i < last; ++i) {
            print_match_panel(&context->matches[i], context->colorize);
        }

        printf("  showing %d-%d of %d  [↑↓] scroll  [r] refresh  [Tab] switch  [q] quit\n",
               first + 1, last, context->count);
    } else {
        printf("  [r] refresh  [Tab] switch  [q] quit\n");
    }

    fflush(stdout);
}
