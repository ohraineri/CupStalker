#ifndef CUP_STALKER_TERMINAL_H
#define CUP_STALKER_TERMINAL_H

void terminal_init(void);
void terminal_restore(void);
void terminal_clear(void);
void terminal_home(void);
void terminal_set_title(const char *title);

#endif
