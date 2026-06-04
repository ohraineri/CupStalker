#ifndef CUP_STALKER_MATCH_H
#define CUP_STALKER_MATCH_H

#include "team.h"

#include <stdbool.h>

#define MATCH_SCORER_CAPACITY 64
#define MATCH_PHASE_CAPACITY 32

typedef enum {
    MATCH_STATUS_SCHEDULED,
    MATCH_STATUS_LIVE,
    MATCH_STATUS_HALFTIME,
    MATCH_STATUS_FINISHED,
    MATCH_STATUS_POSTPONED
} MatchStatus;

typedef struct {
    char scorer_name[MATCH_SCORER_CAPACITY];
    int  minute;
    bool is_own_goal;
    bool is_penalty;
} MatchEvent;

typedef struct {
    int         id;
    Team        home;
    Team        away;
    int         home_score;
    int         away_score;
    int         minute;
    int         stoppage;
    MatchStatus status;
    char        phase[MATCH_PHASE_CAPACITY];

    MatchEvent *events;
    int         event_count;
    int         event_capacity;
} Match;

Match *match_create(void);
void match_destroy(Match **match);
void match_add_event(Match *match, const MatchEvent *event);
void match_release_events(Match *match);

typedef struct {
    int         id;
    int         home_score;
    int         away_score;
    MatchStatus status;
} MatchSnapshot;

MatchSnapshot match_snapshot(const Match *match);
const MatchSnapshot *match_snapshot_find(const MatchSnapshot *snapshots,
                                         int count, int id);

bool match_score_changed(const Match *before, const Match *after);
bool match_status_changed(const Match *before, const Match *after);
const char *match_status_label(MatchStatus status);
const Match *match_find_by_id(const Match *matches, int count, int id);

#endif
