#include "team.h"

#include "../util/string_utils.h"

#include <ctype.h>
#include <string.h>

void team_init(Team *team, const char *name, const char *code, const char *flag)
{
    if (team == NULL) {
        return;
    }

    string_copy(team->name, sizeof team->name, name);
    string_copy(team->code, sizeof team->code, code);
    string_copy(team->flag, sizeof team->flag, flag);
}

typedef struct {
    const char *code;
    const char *flag;
} FlagEntry;

static const FlagEntry FLAG_TABLE[] = {
    { "BRA", "\U0001F1E7\U0001F1F7" },
    { "ARG", "\U0001F1E6\U0001F1F7" },
    { "FRA", "\U0001F1EB\U0001F1F7" },
    { "GER", "\U0001F1E9\U0001F1EA" },
    { "ESP", "\U0001F1EA\U0001F1F8" },
    { "ENG", "\U0001F3F4\U000E0067\U000E0062\U000E0065\U000E006E\U000E0067\U000E007F" },
    { "POR", "\U0001F1F5\U0001F1F9" },
    { "NED", "\U0001F1F3\U0001F1F1" },
    { "ITA", "\U0001F1EE\U0001F1F9" },
    { "BEL", "\U0001F1E7\U0001F1EA" },
    { "CRO", "\U0001F1ED\U0001F1F7" },
    { "URU", "\U0001F1FA\U0001F1FE" },
    { "MEX", "\U0001F1F2\U0001F1FD" },
    { "USA", "\U0001F1FA\U0001F1F8" },
    { "CAN", "\U0001F1E8\U0001F1E6" },
    { "JPN", "\U0001F1EF\U0001F1F5" },
    { "KOR", "\U0001F1F0\U0001F1F7" },
    { "MAR", "\U0001F1F2\U0001F1E6" },
    { "SEN", "\U0001F1F8\U0001F1F3" },
    { "GHA", "\U0001F1EC\U0001F1ED" },
    { "NGA", "\U0001F1F3\U0001F1EC" },
    { "COL", "\U0001F1E8\U0001F1F4" },
    { "ECU", "\U0001F1EA\U0001F1E8" },
    { "SUI", "\U0001F1E8\U0001F1ED" },
    { "DEN", "\U0001F1E9\U0001F1F0" },
    { "AUS", "\U0001F1E6\U0001F1FA" },
};

static const size_t FLAG_TABLE_SIZE = sizeof FLAG_TABLE / sizeof FLAG_TABLE[0];

static const char *const FLAG_UNKNOWN = "\U0001F3F3";

typedef struct {
    const char *name;
    const char *code;
} CodeEntry;

static const CodeEntry CODE_TABLE[] = {
    { "Brazil", "BRA" },        { "Argentina", "ARG" },     { "France", "FRA" },
    { "Germany", "GER" },       { "Spain", "ESP" },         { "England", "ENG" },
    { "Portugal", "POR" },      { "Netherlands", "NED" },   { "Italy", "ITA" },
    { "Belgium", "BEL" },       { "Croatia", "CRO" },       { "Uruguay", "URU" },
    { "Mexico", "MEX" },        { "United States", "USA" }, { "USA", "USA" },
    { "Canada", "CAN" },        { "Japan", "JPN" },         { "South Korea", "KOR" },
    { "Morocco", "MAR" },       { "Senegal", "SEN" },       { "Ghana", "GHA" },
    { "Nigeria", "NGA" },       { "Colombia", "COL" },      { "Ecuador", "ECU" },
    { "Switzerland", "SUI" },   { "Denmark", "DEN" },       { "Australia", "AUS" },
    { "South Africa", "RSA" },  { "Czech Republic", "CZE" },{ "Paraguay", "PAR" },
    { "Bosnia-Herzegovina", "BIH" }, { "Jordan", "JOR" },   { "Austria", "AUT" },
    { "Poland", "POL" },        { "Serbia", "SRB" },        { "Wales", "WAL" },
    { "Iran", "IRN" },          { "Saudi Arabia", "KSA" },  { "Qatar", "QAT" },
    { "Cameroon", "CMR" },      { "Tunisia", "TUN" },       { "Costa Rica", "CRC" },
};

static const size_t CODE_TABLE_SIZE = sizeof CODE_TABLE / sizeof CODE_TABLE[0];

void team_code_for_name(char *out, size_t size, const char *name)
{
    if (out == NULL || size == 0) {
        return;
    }
    if (name == NULL || name[0] == '\0') {
        out[0] = '\0';
        return;
    }

    for (size_t i = 0; i < CODE_TABLE_SIZE; ++i) {
        if (strcmp(name, CODE_TABLE[i].name) == 0) {
            string_copy(out, size, CODE_TABLE[i].code);
            return;
        }
    }

    size_t written = 0;
    for (const char *cursor = name; *cursor && written < 3 && written + 1 < size; ++cursor) {
        if (isalpha((unsigned char)*cursor)) {
            out[written++] = (char)toupper((unsigned char)*cursor);
        }
    }
    out[written] = '\0';
}

const char *team_flag_for_code(const char *code)
{
    if (code == NULL || code[0] == '\0') {
        return FLAG_UNKNOWN;
    }

    char upper[TEAM_CODE_CAPACITY];
    string_copy(upper, sizeof upper, code);
    for (char *cursor = upper; *cursor; ++cursor) {
        *cursor = (char)toupper((unsigned char)*cursor);
    }

    for (size_t i = 0; i < FLAG_TABLE_SIZE; ++i) {
        if (strcmp(upper, FLAG_TABLE[i].code) == 0) {
            return FLAG_TABLE[i].flag;
        }
    }

    return FLAG_UNKNOWN;
}
