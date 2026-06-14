#include "test_util.h"

#include "../src/model/match.h"

#include <string.h>

static void test_create_and_destroy(void)
{
    Match *match = match_create();
    CHECK(match != NULL, "match_create returns non-NULL");
    CHECK(match->status == MATCH_STATUS_SCHEDULED, "new match is scheduled");
    CHECK(match->minute == -1, "new match has no minute");
    CHECK(match->events == NULL, "new match has no events");

    match_destroy(&match);
    CHECK(match == NULL, "match_destroy nulls the pointer");
}

static void test_event_growth(void)
{
    Match *match = match_create();
    for (int i = 0; i < 10; ++i) {
        MatchEvent event = { .minute = i };
        match_add_event(match, &event);
    }
    CHECK(match->event_count == 10, "all events stored");
    CHECK(match->event_capacity >= 10, "capacity grew to fit");
    CHECK(match->events[9].minute == 9, "last event preserved");

    match_destroy(&match);
}

static void test_change_detection(void)
{
    Match before = { .home_score = 1, .away_score = 0, .status = MATCH_STATUS_LIVE };
    Match after = { .home_score = 2, .away_score = 0, .status = MATCH_STATUS_LIVE };

    CHECK(match_score_changed(&before, &after), "score change detected");
    CHECK(!match_status_changed(&before, &after), "status unchanged reported false");

    after.status = MATCH_STATUS_FINISHED;
    CHECK(match_status_changed(&before, &after), "status change detected");
}

static void test_snapshot_roundtrip(void)
{
    Match match = { .id = 7, .home_score = 3, .away_score = 1, .status = MATCH_STATUS_FINISHED };
    MatchSnapshot snapshot = match_snapshot(&match);

    CHECK(snapshot.id == 7, "snapshot keeps id");
    CHECK(snapshot.home_score == 3, "snapshot keeps home score");

    MatchSnapshot pool[] = { snapshot };
    CHECK(match_snapshot_find(pool, 1, 7) != NULL, "snapshot found by id");
    CHECK(match_snapshot_find(pool, 1, 99) == NULL, "missing id returns NULL");
}

static void test_status_label(void)
{
    CHECK(strcmp(match_status_label(MATCH_STATUS_LIVE), "LIVE") == 0, "LIVE label");
    CHECK(strcmp(match_status_label(MATCH_STATUS_FINISHED), "FT") == 0, "FT label");
}

void tests_match(void)
{
    test_create_and_destroy();
    test_event_growth();
    test_change_detection();
    test_snapshot_roundtrip();
    test_status_label();
}
