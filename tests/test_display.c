#include "test_util.h"

#include "../src/display/display.h"
#include "../src/model/match.h"
#include "../src/util/string_utils.h"

#include <string.h>

static Match make_sample_match(void)
{
    Match match = { .id = 1, .home_score = 2, .away_score = 1, .minute = 67,
                    .status = MATCH_STATUS_LIVE };
    team_init(&match.home, "Brazil", "BRA", "");
    team_init(&match.away, "Argentina", "ARG", "");
    return match;
}

static void test_goal_log_format(void)
{
    Match match = make_sample_match();
    MatchEvent event = { .minute = 67 };
    string_copy(event.scorer_name, sizeof event.scorer_name, "Rodrygo");

    char line[160];
    display_format_goal_log(line, sizeof line, &match, &event);

    CHECK(strstr(line, "GOAL:") != NULL, "log line marks a goal");
    CHECK(strstr(line, "BRA 2-1 ARG") != NULL, "log line shows codes and score");
    CHECK(strstr(line, "Rodrygo") != NULL, "log line names the scorer");
}

static void test_goal_banner_format(void)
{
    Match match = make_sample_match();
    MatchEvent event = { .minute = 67 };
    string_copy(event.scorer_name, sizeof event.scorer_name, "Rodrygo");

    char banner[160];
    display_format_goal_banner(banner, sizeof banner, &match, &event);

    CHECK(strstr(banner, "GOL!") != NULL, "banner celebrates the goal");
    CHECK(strstr(banner, "Brazil") != NULL, "banner uses full team name");
}

static void test_display_width_ignores_ansi(void)
{
    CHECK(string_display_width("\033[31mAB\033[0m") == 2, "ANSI escapes are zero-width");
    CHECK(string_display_width("BRA") == 3, "plain ASCII width");
}

void tests_display(void)
{
    test_goal_log_format();
    test_goal_banner_format();
    test_display_width_ignores_ansi();
}
