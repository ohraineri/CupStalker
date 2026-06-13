#include "../config.h"

#include "api/api_client.h"
#include "api/api_parser.h"
#include "display/display.h"
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
    }
    state->match_count = 0;
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
    DisplayContext context = {
        .matches = state->matches,
        .count = state->match_count,
        .selected_index = state->selected_index,
        .show_details = state->show_details,
        .banner = (state->banner[0] != '\0') ? state->banner : NULL,
        .log_lines = (const char (*)[160])state->goal_log,
        .log_count = state->goal_log_count,
    };
    display_render(state->mode, &context);
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
        case INPUT_REFRESH:
            scheduler_trigger_now();
            break;
        case INPUT_NAV_UP:
            state->selected_index -= 1;
            app_clamp_selection(state);
            break;
        case INPUT_NAV_DOWN:
            state->selected_index += 1;
            app_clamp_selection(state);
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
    struct sigaction action = { 0 };
    action.sa_handler = request_quit;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
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
    terminal_restore();
    api_client_global_cleanup();
    return 0;
}
