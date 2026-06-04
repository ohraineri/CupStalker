#include "match.h"

#include "../util/memory.h"

#include <string.h>

#define MATCH_EVENT_INITIAL_CAPACITY 4

Match *match_create(void)
{
    Match *match = memory_alloc(sizeof *match);
    match->minute = -1;
    match->status = MATCH_STATUS_SCHEDULED;
    return match;
}

void match_destroy(Match **match)
{
    if (match == NULL || *match == NULL) {
        return;
    }

    memory_free((void **)&(*match)->events);
    memory_free((void **)match);
}

void match_add_event(Match *match, const MatchEvent *event)
{
    if (match == NULL || event == NULL) {
        return;
    }

    if (match->event_count == match->event_capacity) {
        int next_capacity = (match->event_capacity == 0)
                                ? MATCH_EVENT_INITIAL_CAPACITY
                                : match->event_capacity * 2;
        match->events = memory_realloc(match->events,
                                       (size_t)next_capacity * sizeof *match->events);
        match->event_capacity = next_capacity;
    }

    match->events[match->event_count] = *event;
    match->event_count += 1;
}

void match_release_events(Match *match)
{
    if (match == NULL) {
        return;
    }

    memory_free((void **)&match->events);
    match->event_count = 0;
    match->event_capacity = 0;
}

MatchSnapshot match_snapshot(const Match *match)
{
    return (MatchSnapshot){
        .id = match->id,
        .home_score = match->home_score,
        .away_score = match->away_score,
        .status = match->status,
    };
}

const MatchSnapshot *match_snapshot_find(const MatchSnapshot *snapshots, int count, int id)
{
    if (snapshots == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; ++i) {
        if (snapshots[i].id == id) {
            return &snapshots[i];
        }
    }

    return NULL;
}

bool match_score_changed(const Match *before, const Match *after)
{
    if (before == NULL || after == NULL) {
        return false;
    }

    return before->home_score != after->home_score ||
           before->away_score != after->away_score;
}

bool match_status_changed(const Match *before, const Match *after)
{
    if (before == NULL || after == NULL) {
        return false;
    }

    return before->status != after->status;
}

const char *match_status_label(MatchStatus status)
{
    switch (status) {
        case MATCH_STATUS_SCHEDULED: return "SCHEDULED";
        case MATCH_STATUS_LIVE:      return "LIVE";
        case MATCH_STATUS_HALFTIME:  return "HT";
        case MATCH_STATUS_FINISHED:  return "FT";
        case MATCH_STATUS_POSTPONED: return "POSTPONED";
    }
    return "UNKNOWN";
}

const Match *match_find_by_id(const Match *matches, int count, int id)
{
    if (matches == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; ++i) {
        if (matches[i].id == id) {
            return &matches[i];
        }
    }

    return NULL;
}
