#include "../config.h"

#include "api/api_client.h"
#include "api/api_parser.h"
#include "display/display.h"
#include "display/tab_recent_matches.h"
#include "display/terminal.h"
#include "input/input_handler.h"
#include "model/match.h"
#include "scheduler/scheduler.h"
#include "util/logger.h"
#include "util/string_utils.h"

#include <signal.h>
#include <stdio.h>
#include <time.h>

#define GOAL_LOG_CAPACITY 6

static volatile sig_atomic_t quit_requested = 0;

static void request_quit(int signal_number)
{
    (void)signal_number;
    quit_requested = 1;
}

typedef struct {
    Match         matches[MAX_MATCHES];
    int           match_count;

    MatchSnapshot previous[MAX_MATCHES];
    int           previous_count;
    bool          primed;

    DisplayMode   mode;
    int           selected_index;
    bool          show_details;

    AppTab        active_tab;
    Match         recent[MAX_MATCHES];
    int           recent_count;
    int           recent_scroll;
    bool          recent_loaded;
    bool          recent_pending;
    char          recent_status[160];

    char          goal_log[GOAL_LOG_CAPACITY][160];
    int           goal_log_count;

    char          banner[160];
    time_t        banner_expiry;
} AppState;

static void sleep_ms(long milliseconds)
{
    struct timespec span = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (milliseconds % 1000) * 1000000L,
    };
    nanosleep(&span, NULL);
}

static void app_release_matches(AppState *state)
{
    for (int i = 0; i < state->match_count; ++i) {
        match_release_events(&state->matches[i]);
        match_release_cards(&state->matches[i]);
    }
    state->match_count = 0;
}

static void app_release_recent(AppState *state)
{
    for (int i = 0; i < state->recent_count; ++i) {
        match_release_events(&state->recent[i]);
        match_release_cards(&state->recent[i]);
    }
    state->recent_count = 0;
}

static void app_clamp_recent(AppState *state)
{
    int max_scroll = state->recent_count - RECENT_VISIBLE_MATCHES;
    if (max_scroll < 0) {
        max_scroll = 0;
    }
    if (state->recent_scroll > max_scroll) {
        state->recent_scroll = max_scroll;
    }
    if (state->recent_scroll < 0) {
        state->recent_scroll = 0;
    }
}

static void app_enrich_timeline(Match *match)
{
    ApiResponse timeline = { 0 };
    Result fetched = api_client_fetch_timeline(match->id, &timeline);
    if (!fetched.success) {
        logger_debug("timeline fetch failed for %d: %s", match->id, fetched.message);
        return;
    }

    match_release_events(match);
    match_release_cards(match);
    api_parser_parse_timeline(timeline.data, match);
    api_response_free(&timeline);
}

static void app_load_recent(AppState *state)
{
    ApiResponse response = { 0 };
    Result fetched = api_client_fetch_recent_matches(&response);
    if (!fetched.success) {
        app_release_recent(state);
        snprintf(state->recent_status, sizeof state->recent_status,
                 "Fetch failed: %s", fetched.message);
        state->recent_loaded = true;
        logger_error("recent fetch failed: %s", fetched.message);
        return;
    }

    app_release_recent(state);
    Result parsed = api_parser_parse_matches(response.data, state->recent,
                                             MAX_MATCHES, &state->recent_count);
    api_response_free(&response);

    if (!parsed.success) {
        state->recent_count = 0;
        string_copy(state->recent_status, sizeof state->recent_status,
                    "Failed to parse match data (check API response)");
        state->recent_loaded = true;
        return;
    }

    int write = 0;
    for (int i = 0; i < state->recent_count; ++i) {
        bool keep = state->recent[i].status == MATCH_STATUS_FINISHED &&
                    write < RECENT_MATCHES_LIMIT;
        if (keep) {
            if (write != i) {
                state->recent[write] = state->recent[i];
            }
            write += 1;
        } else {
            match_release_events(&state->recent[i]);
            match_release_cards(&state->recent[i]);
        }
    }
    state->recent_count = write;

    for (int i = 0; i < state->recent_count; ++i) {
        app_enrich_timeline(&state->recent[i]);
    }

    if (state->recent_count == 0) {
        string_copy(state->recent_status, sizeof state->recent_status,
                    "No recent matches found");
    } else {
        state->recent_status[0] = '\0';
    }

    state->recent_scroll = 0;
    state->recent_loaded = true;
}

static void app_push_goal_log(AppState *state, const char *line)
{
    if (state->goal_log_count == GOAL_LOG_CAPACITY) {
        for (int i = 1; i < GOAL_LOG_CAPACITY; ++i) {
            string_copy(state->goal_log[i - 1], sizeof state->goal_log[i - 1],
                        state->goal_log[i]);
        }
        state->goal_log_count -= 1;
    }
    string_copy(state->goal_log[state->goal_log_count],
                sizeof state->goal_log[state->goal_log_count], line);
    state->goal_log_count += 1;
}

static const MatchEvent *latest_event(const Match *match)
{
    return (match->event_count > 0) ? &match->events[match->event_count - 1] : NULL;
}

static void app_announce_goal(AppState *state, const Match *match)
{
    const MatchEvent *event = latest_event(match);

    char line[160];
    display_format_goal_log(line, sizeof line, match, event);
    app_push_goal_log(state, line);

    display_format_goal_banner(state->banner, sizeof state->banner, match, event);
    state->banner_expiry = time(NULL) + GOAL_BANNER_SECONDS;

    logger_debug("goal detected: %s", line);
}

static void app_detect_changes(AppState *state)
{
    if (!state->primed) {
        return;
    }

    for (int i = 0; i < state->match_count; ++i) {
        const Match *current = &state->matches[i];
        const MatchSnapshot *before =
            match_snapshot_find(state->previous, state->previous_count, current->id);
        if (before == NULL) {
            continue;
        }

        int before_total = before->home_score + before->away_score;
        int after_total = current->home_score + current->away_score;
        if (after_total > before_total) {
            app_announce_goal(state, current);
        }
    }
}

static void app_capture_snapshots(AppState *state)
{
    state->previous_count = state->match_count;
    for (int i = 0; i < state->match_count; ++i) {
        state->previous[i] = match_snapshot(&state->matches[i]);
    }
    state->primed = true;
}

static void app_clamp_selection(AppState *state)
{
    if (state->selected_index >= state->match_count) {
        state->selected_index = state->match_count - 1;
    }
    if (state->selected_index < 0) {
        state->selected_index = 0;
    }
}

static Result app_refresh(AppState *state)
{
    ApiResponse response = { 0 };
    Result fetched = api_client_fetch_matches(&response);
    if (!fetched.success) {
        return fetched;
    }

    app_release_matches(state);
    Result parsed = api_parser_parse_matches(response.data, state->matches,
                                             MAX_MATCHES, &state->match_count);
    api_response_free(&response);
    if (!parsed.success) {
        return parsed;
    }

    for (int i = 0; i < state->match_count; ++i) {
        MatchStatus status = state->matches[i].status;
        if (status == MATCH_STATUS_LIVE || status == MATCH_STATUS_HALFTIME) {
            app_enrich_timeline(&state->matches[i]);
        }
    }

    app_detect_changes(state);
    app_capture_snapshots(state);
    app_clamp_selection(state);
    return RESULT_OK;
}

static void app_expire_banner(AppState *state)
{
    if (state->banner[0] != '\0' && time(NULL) >= state->banner_expiry) {
        state->banner[0] = '\0';
    }
}

static void app_render(const AppState *state)
{
    if (state->active_tab == APP_TAB_RECENT) {
        RecentMatchesContext recent = {
            .matches = state->recent,
            .count = state->recent_count,
            .scroll = state->recent_scroll,
            .status_message = (state->recent_status[0] != '\0') ? state->recent_status : NULL,
            .loaded = state->recent_loaded,
            .colorize = (state->mode == DISPLAY_MODE_VIVID),
            .active_tab = state->active_tab,
        };
        tab_recent_matches_render(&recent);
        return;
    }

    DisplayContext context = {
        .matches = state->matches,
        .count = state->match_count,
        .selected_index = state->selected_index,
        .show_details = state->show_details,
        .banner = (state->banner[0] != '\0') ? state->banner : NULL,
        .active_tab = state->active_tab,
        .log_lines = (const char (*)[160])state->goal_log,
        .log_count = state->goal_log_count,
    };
    display_render(state->mode, &context);
}

static void app_switch_tab(AppState *state, AppTab tab)
{
    state->active_tab = tab;
    if (tab == APP_TAB_RECENT && !state->recent_loaded) {
        string_copy(state->recent_status, sizeof state->recent_status, "");
        state->recent_pending = true;
    }
}

static bool app_handle_input(AppState *state, InputAction action)
{
    switch (action) {
        case INPUT_QUIT:
            return false;
        case INPUT_TOGGLE_MODE:
            state->mode = (state->mode == DISPLAY_MODE_STEALTH)
                              ? DISPLAY_MODE_VIVID : DISPLAY_MODE_STEALTH;
            break;
        case INPUT_NEXT_TAB:
            app_switch_tab(state, (AppTab)((state->active_tab + 1) % APP_TAB_COUNT));
            break;
        case INPUT_PREV_TAB:
            app_switch_tab(state,
                           (AppTab)((state->active_tab + APP_TAB_COUNT - 1) % APP_TAB_COUNT));
            break;
        case INPUT_REFRESH:
            if (state->active_tab == APP_TAB_RECENT) {
                state->recent_loaded = false;
                state->recent_pending = true;
            } else {
                scheduler_trigger_now();
            }
            break;
        case INPUT_NAV_UP:
            if (state->active_tab == APP_TAB_RECENT) {
                state->recent_scroll -= 1;
                app_clamp_recent(state);
            } else {
                state->selected_index -= 1;
                app_clamp_selection(state);
            }
            break;
        case INPUT_NAV_DOWN:
            if (state->active_tab == APP_TAB_RECENT) {
                state->recent_scroll += 1;
                app_clamp_recent(state);
            } else {
                state->selected_index += 1;
                app_clamp_selection(state);
            }
            break;
        case INPUT_TOGGLE_DETAILS:
            state->show_details = !state->show_details;
            break;
        case INPUT_NONE:
            break;
    }
    return true;
}

static void install_signal_handlers(void)
{
    signal(SIGINT, request_quit);
    signal(SIGTERM, request_quit);
}

static void run_loop(AppState *state)
{
    bool dirty = true;

    while (!quit_requested) {
        InputAction action = input_handler_poll();
        if (action != INPUT_NONE) {
            if (!app_handle_input(state, action)) {
                break;
            }
            dirty = true;
        }

        if (scheduler_poll()) {
            Result refresh = app_refresh(state);
            if (!refresh.success) {
                logger_error("refresh failed: %s", refresh.message);
            }
            dirty = true;
        }

        if (state->banner[0] != '\0') {
            dirty = true;
        }

        if (dirty) {
            app_expire_banner(state);
            app_render(state);
            dirty = false;
        }

        if (state->recent_pending) {
            app_load_recent(state);
            state->recent_pending = false;
            app_clamp_recent(state);
            dirty = true;
        }

        sleep_ms(MAIN_LOOP_SLEEP_MS);
    }
}

int main(void)
{
    AppState state = {
        .mode = DEFAULT_DISPLAY_MODE,
    };

    install_signal_handlers();

    Result curl_init = api_client_global_init();
    if (!curl_init.success) {
        fprintf(stderr, "fatal: %s\n", curl_init.message);
        return 1;
    }

    terminal_init();
    terminal_set_title(STEALTH_WINDOW_TITLE);

    Result first = app_refresh(&state);
    if (!first.success) {
        logger_error("initial fetch failed: %s", first.message);
    }
    app_render(&state);

    scheduler_start(REFRESH_INTERVAL_SECONDS);
    run_loop(&state);
    scheduler_stop();

    app_release_matches(&state);
    app_release_recent(&state);
    terminal_restore();
    api_client_global_cleanup();
    return 0;
}
