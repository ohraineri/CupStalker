#include "terminal.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static struct termios original_termios;
static int termios_saved = 0;

void terminal_init(void)
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == 0) {
        termios_saved = 1;
    }

    struct termios raw = original_termios;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    fputs("\033[?25l", stdout);
    fflush(stdout);
}

void terminal_restore(void)
{
    fputs("\033[?25h", stdout);
    terminal_clear();

    if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    }
    fflush(stdout);
}

void terminal_clear(void)
{
    fputs("\033[2J\033[H", stdout);
}

void terminal_home(void)
{
    fputs("\033[H", stdout);
}

void terminal_set_title(const char *title)
{
    if (title == NULL) {
        return;
    }

    printf("\033]0;%s\007", title);
    fflush(stdout);
}
