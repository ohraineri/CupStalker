#include "api_parser.h"

#include "../util/logger.h"
#include "../util/string_utils.h"

#include <cjson/cJSON.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const char *json_string_or(const cJSON *object, const char *key, const char *fallback)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);
    return (cJSON_IsString(item) && item->valuestring != NULL) ? item->valuestring : fallback;
}

static int json_int_or(const cJSON *object, const char *key, int fallback)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);
    return cJSON_IsNumber(item) ? item->valueint : fallback;
}

static MatchStatus map_status(const char *status)
{
    if (string_equals(status, "IN_PLAY"))  return MATCH_STATUS_LIVE;
    if (string_equals(status, "PAUSED"))   return MATCH_STATUS_HALFTIME;
    if (string_equals(status, "FINISHED")) return MATCH_STATUS_FINISHED;
    if (string_equals(status, "POSTPONED") ||
        string_equals(status, "SUSPENDED") ||
        string_equals(status, "CANCELLED")) return MATCH_STATUS_POSTPONED;
    return MATCH_STATUS_SCHEDULED;
}

static void humanize_phase(char *destination, size_t size,
                           const char *group, const char *stage)
{
    if (group != NULL && strncmp(group, "GROUP_", 6) == 0) {
        snprintf(destination, size, "Group %s", group + 6);
        return;
    }

    if (string_equals(stage, "LAST_16"))        { string_copy(destination, size, "Round of 16"); return; }
    if (string_equals(stage, "QUARTER_FINALS")) { string_copy(destination, size, "Quarterfinal"); return; }
    if (string_equals(stage, "SEMI_FINALS"))    { string_copy(destination, size, "Semifinal"); return; }
    if (string_equals(stage, "THIRD_PLACE"))    { string_copy(destination, size, "Third place"); return; }
    if (string_equals(stage, "FINAL"))          { string_copy(destination, size, "Final"); return; }

    string_copy(destination, size, stage != NULL ? stage : "");
}

static void parse_team(const cJSON *team_json, Team *team)
{
    const char *name = json_string_or(team_json, "name", "TBD");
    const char *code = json_string_or(team_json, "tla", "");
    team_init(team, name, code, team_flag_for_code(code));
}

static void parse_score(const cJSON *match_json, Match *match)
{
    const cJSON *score = cJSON_GetObjectItemCaseSensitive(match_json, "score");
    const cJSON *full_time = cJSON_GetObjectItemCaseSensitive(score, "fullTime");
    match->home_score = json_int_or(full_time, "home", 0);
    match->away_score = json_int_or(full_time, "away", 0);
}

static void parse_goals(const cJSON *match_json, Match *match)
{
    const cJSON *goals = cJSON_GetObjectItemCaseSensitive(match_json, "goals");
    if (!cJSON_IsArray(goals)) {
        return;
    }

    const cJSON *goal = NULL;
    cJSON_ArrayForEach(goal, goals) {
        const cJSON *scorer = cJSON_GetObjectItemCaseSensitive(goal, "scorer");
        const char *type = json_string_or(goal, "type", "REGULAR");

        MatchEvent event = { 0 };
        string_copy(event.scorer_name, sizeof event.scorer_name,
                    json_string_or(scorer, "name", "Unknown"));
        event.minute = json_int_or(goal, "minute", 0);
        event.is_own_goal = string_equals(type, "OWN");
        event.is_penalty = string_equals(type, "PENALTY");

        match_add_event(match, &event);
    }
}

static void parse_match(const cJSON *match_json, Match *match)
{
    match->id = json_int_or(match_json, "id", 0);
    match->status = map_status(json_string_or(match_json, "status", ""));
    match->minute = json_int_or(match_json, "minute", -1);
    match->stoppage = json_int_or(match_json, "injuryTime", 0);

    humanize_phase(match->phase, sizeof match->phase,
                   json_string_or(match_json, "group", NULL),
                   json_string_or(match_json, "stage", ""));

    parse_team(cJSON_GetObjectItemCaseSensitive(match_json, "homeTeam"), &match->home);
    parse_team(cJSON_GetObjectItemCaseSensitive(match_json, "awayTeam"), &match->away);
    parse_score(match_json, match);
    parse_goals(match_json, match);
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

    const cJSON *match_array = cJSON_GetObjectItemCaseSensitive(root, "matches");
    if (!cJSON_IsArray(match_array)) {
        cJSON_Delete(root);
        return RESULT_ERROR("JSON has no \"matches\" array");
    }

    int count = 0;
    const cJSON *match_json = NULL;
    cJSON_ArrayForEach(match_json, match_array) {
        if (count >= capacity) {
            logger_debug("match list truncated at capacity %d", capacity);
            break;
        }

        matches[count] = (Match){ .minute = -1, .status = MATCH_STATUS_SCHEDULED };
        parse_match(match_json, &matches[count]);
        count += 1;
    }

    *out_count = count;
    cJSON_Delete(root);
    return RESULT_OK;
}
