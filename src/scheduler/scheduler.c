#include "scheduler.h"

#include <signal.h>
#include <sys/time.h>

static volatile sig_atomic_t tick_pending = 0;

static void handle_alarm(int signal_number)
{
    (void)signal_number;
    tick_pending = 1;
}

void scheduler_start(int interval_seconds)
{
    if (interval_seconds < 1) {
        interval_seconds = 1;
    }

    struct sigaction action = { 0 };
    action.sa_handler = handle_alarm;
    sigemptyset(&action.sa_mask);
    sigaction(SIGALRM, &action, NULL);

    struct itimerval timer;
    timer.it_value.tv_sec = interval_seconds;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = interval_seconds;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

void scheduler_stop(void)
{
    struct itimerval disarmed = { 0 };
    setitimer(ITIMER_REAL, &disarmed, NULL);
}

bool scheduler_poll(void)
{
    if (tick_pending) {
        tick_pending = 0;
        return true;
    }
    return false;
}

void scheduler_trigger_now(void)
{
    tick_pending = 1;
}
