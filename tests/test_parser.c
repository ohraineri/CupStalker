#include "test_util.h"

#include "../src/api/api_parser.h"
#include "../src/model/match.h"

#include <string.h>

static const char *SAMPLE_JSON =
    "{\"events\":["
    "  {\"idEvent\":\"2391741\",\"strEvent\":\"Austria vs Jordan\","
    "   \"strHomeTeam\":\"Austria\",\"strAwayTeam\":\"Jordan\","
    "   \"intHomeScore\":\"3\",\"intAwayScore\":\"1\","
    "   \"strVenue\":\"Levi's Stadium\",\"strCity\":\"Santa Clara\","
    "   \"strTimestamp\":\"2026-06-17T04:00:00\",\"strStatus\":\"FT\","
    "   \"strGroup\":\"Group A\"},"
    "  {\"idEvent\":\"2391742\",\"strEvent\":\"Brazil vs Morocco\","
    "   \"strHomeTeam\":\"Brazil\",\"strAwayTeam\":\"Morocco\","
    "   \"intHomeScore\":null,\"intAwayScore\":null,\"strStatus\":\"NS\"}"
    "]}";

static const char *TIMELINE_JSON =
    "{\"timeline\":["
    "  {\"strTimeline\":\"Goal\",\"strTimelineDetail\":\"Normal Goal\",\"strHome\":\"Yes\","
    "   \"strPlayer\":\"Romano Schmid\",\"intTime\":\"21\"},"
    "  {\"strTimeline\":\"Goal\",\"strTimelineDetail\":\"Penalty\",\"strHome\":\"No\","
    "   \"strPlayer\":\"Ali Olwan\",\"intTime\":\"50\"},"
    "  {\"strTimeline\":\"Card\",\"strTimelineDetail\":\"Yellow Card\",\"strHome\":\"Yes\","
    "   \"strPlayer\":\"Marko Arnautovic\",\"intTime\":\"30\"},"
    "  {\"strTimeline\":\"Card\",\"strTimelineDetail\":\"Red Card\",\"strHome\":\"No\","
    "   \"strPlayer\":\"Yazan Al-Arab\",\"intTime\":\"80\"},"
    "  {\"strTimeline\":\"subst\",\"strTimelineDetail\":\"Substitution 1\",\"strHome\":\"Yes\","
    "   \"strPlayer\":\"David Alaba\",\"intTime\":\"46\"}"
    "]}";

static void release_all(Match *matches, int count)
{
    for (int i = 0; i < count; ++i) {
        match_release_events(&matches[i]);
        match_release_cards(&matches[i]);
    }
}

static void test_parse_counts_and_fields(void)
{
    Match matches[8] = { 0 };
    int count = 0;
    Result result = api_parser_parse_matches(SAMPLE_JSON, matches, 8, &count);

    CHECK(result.success, "valid JSON parses successfully");
    CHECK(count == 2, "two events parsed");

    CHECK(matches[0].id == 2391741, "idEvent string mapped to int");
    CHECK(matches[0].status == MATCH_STATUS_FINISHED, "FT maps to FINISHED");
    CHECK(matches[0].home_score == 3 && matches[0].away_score == 1, "string scores mapped");
    CHECK(strcmp(matches[0].home.name, "Austria") == 0, "home name mapped");
    CHECK(strcmp(matches[0].home.code, "AUT") == 0, "home code resolved from name");
    CHECK(strcmp(matches[0].away.code, "JOR") == 0, "away code resolved from name");
    CHECK(strcmp(matches[0].venue, "Levi's Stadium") == 0, "venue mapped");
    CHECK(strcmp(matches[0].city, "Santa Clara") == 0, "city mapped");
    CHECK(strcmp(matches[0].date_utc, "2026-06-17T04:00:00") == 0, "timestamp mapped");
    CHECK(strcmp(matches[0].phase, "Group A") == 0, "group mapped to phase");

    CHECK(matches[1].status == MATCH_STATUS_SCHEDULED, "NS maps to SCHEDULED");
    CHECK(strcmp(matches[1].home.code, "BRA") == 0, "second match code resolved");

    release_all(matches, count);
}

static void test_parse_timeline(void)
{
    Match match = { 0 };
    team_init(&match.home, "Austria", "AUT", "");
    team_init(&match.away, "Jordan", "JOR", "");

    Result result = api_parser_parse_timeline(TIMELINE_JSON, &match);
    CHECK(result.success, "timeline parses successfully");

    CHECK(match.event_count == 2, "two goals parsed (subst ignored)");
    CHECK(strcmp(match.events[0].scorer_name, "Romano Schmid") == 0, "first scorer mapped");
    CHECK(match.events[0].minute == 21, "goal minute mapped");
    CHECK(strcmp(match.events[0].team_code, "AUT") == 0, "home goal resolves home code");
    CHECK(match.events[1].is_penalty, "penalty detail flagged");
    CHECK(strcmp(match.events[1].team_code, "JOR") == 0, "away goal resolves away code");

    CHECK(match.card_count == 2, "two cards parsed");
    CHECK(match.cards[0].type == CARD_YELLOW, "yellow card mapped");
    CHECK(match.cards[1].type == CARD_RED, "red card mapped");
    CHECK(strcmp(match.cards[1].team_code, "JOR") == 0, "away card resolves away code");

    match_release_events(&match);
    match_release_cards(&match);
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
    Result result = api_parser_parse_matches("{\"events\":null}", matches, 2, &count);
    CHECK(result.success, "null events treated as empty, not error");
    CHECK(count == 0, "null events yields zero matches");
}

void tests_parser(void)
{
    test_parse_counts_and_fields();
    test_parse_timeline();
    test_parse_rejects_garbage();
    test_parse_missing_array();
}
