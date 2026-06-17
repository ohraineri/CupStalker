#ifndef CUP_STALKER_API_PARSER_H
#define CUP_STALKER_API_PARSER_H

#include "../model/match.h"
#include "../util/result.h"

Result api_parser_parse_matches(const char *json,
                                Match *matches,
                                int capacity,
                                int *out_count);

Result api_parser_parse_timeline(const char *json, Match *match);

#endif
