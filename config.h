#ifndef CUP_STALKER_CONFIG_H
#define CUP_STALKER_CONFIG_H

#define API_BASE_URL "https://www.thesportsdb.com/api/v1/json"

#ifndef API_KEY
#define API_KEY "3"
#endif

#define WC_LEAGUE_ID 4429

#define WC_SEASON "2026"

#define API_TIMEOUT_SECONDS 10

#define REFRESH_INTERVAL_SECONDS 30

#define MAX_MATCHES 32

#define MAX_EVENTS_PER_MATCH 32

#define RECENT_MATCHES_LIMIT 10

#define RECENT_VISIBLE_MATCHES 3

#define DEFAULT_DISPLAY_MODE DISPLAY_MODE_STEALTH

#define MAIN_LOOP_SLEEP_MS 30

#define GOAL_BANNER_SECONDS 3

#define STEALTH_WINDOW_TITLE "build-daemon"

#endif
