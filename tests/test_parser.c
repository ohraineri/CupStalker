#include "test_util.h"

#include "../src/api/api_parser.h"
#include "../src/model/match.h"

#include <string.h>

static const char *SAMPLE_JSON =
    "{\"matches\":["
    "  {\"id\":101,\"status\":\"IN_PLAY\",\"minute\":67,\"stage\":\"GROUP_STAGE\","
    "   \"group\":\"GROUP_B\","
    "   \"homeTeam\":{\"name\":\"Brazil\",\"tla\":\"BRA\"},"
    "   \"awayTeam\":{\"name\":\"Argentina\",\"tla\":\"ARG\"},"
    "   \"score\":{\"fullTime\":{\"home\":2,\"away\":1}},"
    "   \"goals\":[{\"minute\":34,\"type\":\"REGULAR\",\"scorer\":{\"name\":\"Vinicius Jr.\"}},"
    "             {\"minute\":61,\"type\":\"PENALTY\",\"scorer\":{\"name\":\"Rodrygo\"}}]},"
    "  {\"id\":102,\"status\":\"FINISHED\",\"stage\":\"FINAL\","
    "   \"homeTeam\":{\"name\":\"England\",\"tla\":\"ENG\"},"
    "   \"awayTeam\":{\"name\":\"United States\",\"tla\":\"USA\"},"
    "   \"score\":{\"fullTime\":{\"home\":3,\"away\":0}}}"
    "]}";

static void release_all(Match *matches, int count)
{
    for (int i = 0; i < count; ++i) {
        match_release_events(&matches[i]);
    }
}

static void test_parse_counts_and_fields(void)
{
    Match matches[8] = { 0 };
    int count = 0;
    Result result = api_parser_parse_matches(SAMPLE_JSON, matches, 8, &count);

    CHECK(result.success, "valid JSON parses successfully");
    CHECK(count == 2, "two matches parsed");

    CHECK(matches[0].id == 101, "first match id mapped");
    CHECK(matches[0].status == MATCH_STATUS_LIVE, "IN_PLAY maps to LIVE");
    CHECK(matches[0].home_score == 2 && matches[0].away_score == 1, "score mapped");
    CHECK(strcmp(matches[0].home.name, "Brazil") == 0, "home name mapped");
    CHECK(strcmp(matches[0].phase, "Group B") == 0, "group humanized");
    CHECK(matches[0].event_count == 2, "both goals parsed");
    CHECK(matches[0].events[1].is_penalty, "penalty flag set");

    CHECK(matches[1].status == MATCH_STATUS_FINISHED, "FINISHED mapped");
    CHECK(strcmp(matches[1].phase, "Final") == 0, "FINAL stage humanized");

    release_all(matches, count);
}

static void test_parse_rejects_garbage(void)
{
    Match matches[2] = { 0 };
    int count = 99;
    Result result = api_parser_parse_matches("not json", matches, 2, &count);
    CHECK(!result.success, "invalid JSON reported as error");
}

static void test_parse_missing_array(void)
{
    Match matches[2] = { 0 };
    int count = 99;
    Result result = api_parser_parse_matches("{\"foo\":1}", matches, 2, &count);
    CHECK(!result.success, "absent matches array reported as error");
}

void tests_parser(void)
{
    test_parse_counts_and_fields();
    test_parse_rejects_garbage();
    test_parse_missing_array();
}
