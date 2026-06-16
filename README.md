# Cup Stalker

Live World Cup 2026 scores in your terminal. No browser, no second monitor,
no tab your manager can see over your shoulder.

It has two modes. **Vivid** is the full thing: flags, colors, goal banners,
match panels. **Stealth** (the default) prints the scores as plain monochrome
lines that look like build output, so a glance from across the room reads as
"he's compiling something", not "he's watching Brazil vs Argentina". You can
flip between them with a single key.

```
[cup-stalker] 2026-06-15 21:47:02 UTC | refresh: 30s
-----------------------------------------------------
BRA 2 x 1 ARG  LIVE  [67']
GER 0 x 0 FRA  LIVE  [34']
ENG 3 x 0 USA  FT
-----------------------------------------------------
[s] vivid  [r] refresh  [d] details  [q] quit
```

## What you need

- Linux or macOS (it uses POSIX terminal stuff, so it won't build on Windows
  without WSL)
- `gcc` or `clang`, and `make`
- libcurl and cJSON

Install the libraries:

```sh
# Debian / Ubuntu
sudo apt install libcurl4-openssl-dev libcjson-dev

# macOS (Homebrew)
brew install curl cjson
```

There's no ncurses dependency. The vivid mode is just ANSI escape codes, so
the only things you have to install are libcurl and cJSON.

## Get an API key

Scores come from [football-data.org](https://www.football-data.org). Make a
free account and grab your API token from the dashboard.

Open `config.h` and paste it in:

```c
#define API_KEY "paste-your-token-here"
```

If you'd rather not edit the file, pass it at build time instead:

```sh
make release CFLAGS_EXTRA='-DAPI_KEY=\"your-token\"'
```

Heads up: the free tier is enough for live scores, status and the match clock,
but the per-goal scorer names usually only show up on the paid plans. The app
handles that gracefully (you just won't see the scorer lines).

## Build and run

```sh
make release
./cup-stalker
```

That's it. `make debug` gives you a build with logging and AddressSanitizer if
something misbehaves, and `make test` runs the unit tests.

## Controls

Keys work immediately, no Enter needed.

| Key   | Does                                   |
|-------|----------------------------------------|
| `s`   | switch between stealth and vivid       |
| `r`   | refresh right now                      |
| `d`   | show/hide goals for the selected match |
| `↑` `↓` | move between matches                 |
| `q`   | quit (puts your terminal back to normal) |

Ctrl-C also quits cleanly.

## Configuration

Everything lives in `config.h`. The ones you'll actually touch:

- `REFRESH_INTERVAL_SECONDS` — how often it polls. Default is 30. The free API
  tier is rate limited, so don't crank this too low.
- `DEFAULT_DISPLAY_MODE` — `DISPLAY_MODE_STEALTH` or `DISPLAY_MODE_VIVID`.
- `COMPETITION_ID` — the football-data.org competition id (set to the 2026
  World Cup).
- `MAX_MATCHES` — how many matches to keep on screen.

Change anything, then `make release` again.

## Notes

- Vivid mode draws flags with emoji and boxes with Unicode, so use a UTF-8
  terminal with a font that has them. If the alignment looks off, that's your
  terminal/font, not the scores.
- New goals are detected by comparing the latest poll against the previous one,
  so a goal shows up within one refresh interval of the API reporting it.
- It's a small personal project. If something breaks or you want a feature,
  open an issue.
