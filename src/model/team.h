#ifndef CUP_STALKER_TEAM_H
#define CUP_STALKER_TEAM_H

#define TEAM_NAME_CAPACITY 64
#define TEAM_CODE_CAPACITY 8
#define TEAM_FLAG_CAPACITY 16

typedef struct {
    char name[TEAM_NAME_CAPACITY];
    char code[TEAM_CODE_CAPACITY];
    char flag[TEAM_FLAG_CAPACITY];
} Team;

void team_init(Team *team, const char *name, const char *code, const char *flag);
const char *team_flag_for_code(const char *code);

#endif
