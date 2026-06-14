# CUP STALKER — World Cup 2026 live terminal scoreboard

A small, dependency-light terminal scoreboard for the 2026 FIFA World Cup,
written in C17. It polls a sports-data API and renders live scores in one of
two modes:

- **STEALTH** (default) — minimal, monochrome, log-like output that blends into
  a normal terminal. Work-safe.
- **VIVID** — flags, ANSI colors, ASCII-art panels, goal banners, and per-match
  event detail.

```
[cup-stalker] 2026-06-15 21:47:02 UTC | refresh: 30s
-----------------------------------------------------
BRA 2 x 1 ARG  LIVE  [67']
GER 0 x 0 FRA  LIVE  [34']
ENG 3 x 0 USA  FT
-----------------------------------------------------
[s] vivid  [r] refresh  [d] details  [q] quit
```

## Dependencies

| Library    | Purpose                       | Install (Debian/Ubuntu)            | Install (macOS / Homebrew) |
|------------|-------------------------------|------------------------------------|----------------------------|
| `libcurl`  | HTTP requests to the API      | `apt install libcurl4-openssl-dev` | `brew install curl`        |
| `cJSON`    | JSON parsing                  | `apt install libcjson-dev`         | `brew install cjson`       |

No `ncurses` dependency: the VIVID mode is implemented with raw ANSI escape
sequences, keeping the dependency surface to `libcurl` and `cJSON` only.

The VIVID mode and flag emoji require a UTF-8 terminal with emoji support.

## Configuration

All configuration lives in [`config.h`](config.h) — there is no runtime config
file, by design. Key constants:

| Constant                   | Meaning                                  |
|----------------------------|------------------------------------------|
| `API_BASE_URL`             | Provider base URL (football-data.org v4) |
| `API_KEY`                  | Auth token sent as `X-Auth-Token`        |
| `COMPETITION_ID`           | Numeric id of the World Cup 2026         |
| `REFRESH_INTERVAL_SECONDS` | Polling cadence (default 30s)            |
| `DEFAULT_DISPLAY_MODE`     | `DISPLAY_MODE_STEALTH` or `_VIVID`       |
| `MAX_MATCHES`              | Max matches held/rendered                |

### API key

This project targets [football-data.org](https://www.football-data.org)'s free
v4 API. Register for a token, then either edit `API_KEY` in `config.h` or pass
it at build time without touching the file:

```sh
make release CFLAGS_EXTRA='-DAPI_KEY=\"your-token-here\"'
```

> Note: the free football-data.org tier exposes goal scorers and minute data
> only on paid plans; on the free tier the scoreboard still shows live scores,
> status, and minute, but per-goal scorer rows may be empty.

## Build

```sh
make release      # optimized build  -> ./cup-stalker
make debug        # -g -DDEBUG + AddressSanitizer, with stderr logging
make test         # build and run the unit tests
make clean        # remove objects and binaries
```

Builds cleanly with `-Wall -Wextra -Wpedantic`.

## Run

```sh
./cup-stalker
```

### Keys

| Key   | Action                              |
|-------|-------------------------------------|
| `s`   | Toggle STEALTH / VIVID              |
| `r`   | Force an immediate refresh          |
| `d`   | Toggle event detail for selection   |
| `↑ ↓` | Navigate between matches            |
| `q`   | Quit (restores the terminal)        |

Input is non-blocking and raw — no Enter required. `Ctrl-C` also exits cleanly.

## Goal detection

On each refresh the new scores are diffed against a snapshot of the previous
state. When a match's aggregate score increases:

- **VIVID** shows a blinking green `⚽ GOL!` banner for a few seconds
  (`GOAL_BANNER_SECONDS`), then returns to the panel.
- **STEALTH** appends a discreet log line, e.g.
  `[67'] GOAL: BRA 2-1 ARG (Rodrygo)`, retained in a small on-screen ring.

## Project layout

```
cup-stalker/
├── config.h              # compile-time configuration
├── Makefile
├── src/
│   ├── main.c            # orchestrator + main loop
│   ├── api/              # libcurl client + cJSON parser
│   ├── model/            # Team, Match, MatchEvent, snapshots
│   ├── display/          # terminal control + STEALTH/VIVID renderers
│   ├── input/            # non-blocking keyboard handling
│   ├── scheduler/        # SIGALRM polling timer
│   └── util/             # Result, checked memory, strings, logging
└── tests/                # unit tests (parser, model, display)
```

### Design notes

- **Error handling:** I/O, allocation, and network functions return a `Result`
  (`{ bool success; char message[256]; }`); callers check `.success`.
- **Memory:** allocations go through checked wrappers (`memory_alloc` aborts on
  OOM); `memory_free` nulls the pointer to prevent double-free. The build is
  intended to be Valgrind-clean.
- **Global state:** the only mutable globals are the two
  `volatile sig_atomic_t` signal flags (timer tick and quit request).
- **Portability:** POSIX (`termios`, `SIGALRM`, `nanosleep`); compiles on Linux
  and macOS. The Makefile sets the feature-test macros needed under `-std=c17`.

## Testing

`make test` compiles the suites in `tests/` against the library sources
(excluding `main.c`) and runs them, printing a `N checks, M failed` summary and
exiting non-zero on any failure.
