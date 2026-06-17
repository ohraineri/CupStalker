#include "api_parser.h"

#include "../util/logger.h"
#include "../util/string_utils.h"

#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>

static const char *json_string_or(const cJSON *object, const char *key, const char *fallback)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);
    return (cJSON_IsString(item) && item->valuestring != NULL) ? item->valuestring : fallback;
}

static int json_int_string_or(const cJSON *object, const char *key, int fallback)
{
    const char *text = json_string_or(object, key, NULL);
    return (text != NULL && text[0] != '\0') ? atoi(text) : fallback;
}

static MatchStatus map_status(const char *status)
{
    if (string_equals(status, "FT") || string_equals(status, "AET") ||
        string_equals(status, "PEN") || string_equals(status, "Match Finished")) {
        return MATCH_STATUS_FINISHED;
    }
    if (string_equals(status, "HT") || string_equals(status, "Half Time")) {
        return MATCH_STATUS_HALFTIME;
    }
    if (string_equals(status, "1H") || string_equals(status, "2H") ||
        string_equals(status, "ET") || string_equals(status, "Live") ||
        string_equals(status, "In Play")) {
        return MATCH_STATUS_LIVE;
    }
    if (string_equals(status, "PST") || string_equals(status, "Postponed") ||
        string_equals(status, "Canc.") || string_equals(status, "Cancelled")) {
        return MATCH_STATUS_POSTPONED;
    }
    return MATCH_STATUS_SCHEDULED;
}

static void parse_team(const char *name, Team *team)
{
    char code[TEAM_CODE_CAPACITY];
    team_code_for_name(code, sizeof code, name);
    team_init(team, (name != NULL) ? name : "TBD", code, team_flag_for_code(code));
}

static void parse_event(const cJSON *event_json, Match *match)
{
    match->id = json_int_string_or(event_json, "idEvent", 0);
    match->status = map_status(json_string_or(event_json, "strStatus", ""));
    match->home_score = json_int_string_or(event_json, "intHomeScore", 0);
    match->away_score = json_int_string_or(event_json, "intAwayScore", 0);

    string_copy(match->phase, sizeof match->phase,
                json_string_or(event_json, "strGroup", ""));
    string_copy(match->venue, sizeof match->venue,
                json_string_or(event_json, "strVenue", ""));
    string_copy(match->city, sizeof match->city,
                json_string_or(event_json, "strCity", ""));
    string_copy(match->date_utc, sizeof match->date_utc,
                json_string_or(event_json, "strTimestamp", ""));

    parse_team(json_string_or(event_json, "strHomeTeam", "TBD"), &match->home);
    parse_team(json_string_or(event_json, "strAwayTeam", "TBD"), &match->away);
}

Result api_parser_parse_matches(const char *json, Match *matches, int capacity, int *out_count)
{
    if (json == NULL || matches == NULL || out_count == NULL) {
        return RESULT_ERROR("api_parser_parse_matches: NULL argument");
    }

    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        return RESULT_ERROR("failed to parse JSON response");
    }

    const cJSON *events = cJSON_GetObjectItemCaseSensitive(root, "events");
    if (!cJSON_IsArray(events)) {
        *out_count = 0;
        cJSON_Delete(root);
        return RESULT_OK;
    }

    int count = 0;
    const cJSON *event_json = NULL;
    cJSON_ArrayForEach(event_json, events) {
        if (count >= capacity) {
            logger_debug("event list truncated at capacity %d", capacity);
            break;
        }

        matches[count] = (Match){ .minute = -1, .status = MATCH_STATUS_SCHEDULED };
        parse_event(event_json, &matches[count]);
        count += 1;
    }

    *out_count = count;
    cJSON_Delete(root);
    return RESULT_OK;
}

static CardType map_card(const char *detail)
{
    if (detail == NULL) {
        return CARD_YELLOW;
    }
    if (strstr(detail, "Second") != NULL) {
        return CARD_YELLOW_RED;
    }
    if (strstr(detail, "Red") != NULL) {
        return CARD_RED;
    }
    return CARD_YELLOW;
}

Result api_parser_parse_timeline(const char *json, Match *match)
{
    if (json == NULL || match == NULL) {
        return RESULT_ERROR("api_parser_parse_timeline: NULL argument");
    }

    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        return RESULT_ERROR("failed to parse timeline JSON");
    }

    const cJSON *timeline = cJSON_GetObjectItemCaseSensitive(root, "timeline");
    if (!cJSON_IsArray(timeline)) {
        cJSON_Delete(root);
        return RESULT_OK;
    }

    const cJSON *entry = NULL;
    cJSON_ArrayForEach(entry, timeline) {
        const char *kind = json_string_or(entry, "strTimeline", "");
        const char *detail = json_string_or(entry, "strTimelineDetail", "");
        bool is_home = string_equals(json_string_or(entry, "strHome", "Yes"), "Yes");
        const char *code = is_home ? match->home.code : match->away.code;
        int minute = json_int_string_or(entry, "intTime", 0);

        if (string_equals(kind, "Goal")) {
            MatchEvent event = { 0 };
            string_copy(event.scorer_name, sizeof event.scorer_name,
                        json_string_or(entry, "strPlayer", "Unknown"));
            string_copy(event.team_code, sizeof event.team_code, code);
            event.minute = minute;
            event.is_penalty = (strstr(detail, "Penalty") != NULL);
            event.is_own_goal = (strstr(detail, "Own") != NULL);
            match_add_event(match, &event);
        } else if (string_equals(kind, "Card")) {
            MatchCard card = { 0 };
            string_copy(card.player_name, sizeof card.player_name,
                        json_string_or(entry, "strPlayer", "Unknown"));
            string_copy(card.team_code, sizeof card.team_code, code);
            card.minute = minute;
            card.type = map_card(detail);
            match_add_card(match, &card);
        }
    }

    cJSON_Delete(root);
    return RESULT_OK;
}
