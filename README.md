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

Scores come from [TheSportsDB](https://www.thesportsdb.com). It ships a free
shared test key (`"3"`), which is what `config.h` uses out of the box — no
account needed to try it.

```c
#define API_KEY "3"
```

The shared key is rate limited and exposes only a subset of fixtures. If you
back TheSportsDB on Patreon you get a private key with full coverage; drop it in
`config.h` or pass it at build time:

```sh
make release CFLAGS_EXTRA='-DAPI_KEY=\"your-key\"'
```

Match list (scores, teams, venue, status) comes from the season endpoint;
goal scorers and cards are pulled per match from the event timeline endpoint,
so the GOALS and CARDS sections fill in for finished and live matches.

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
| `Tab` | switch tabs (Live ↔ Recent Matches)    |
| `s`   | switch between stealth and vivid       |
| `r`   | refresh right now (re-fetches the current tab) |
| `d`   | show/hide goals for the selected match (Live tab) |
| `↑` `↓` | move between matches / scroll Recent Matches |
| `q`   | quit (puts your terminal back to normal) |

Ctrl-C also quits cleanly.

## Configuration

Everything lives in `config.h`. The ones you'll actually touch:

- `REFRESH_INTERVAL_SECONDS` — how often it polls. Default is 30. The free API
  key is rate limited, so don't crank this too low.
- `DEFAULT_DISPLAY_MODE` — `DISPLAY_MODE_STEALTH` or `DISPLAY_MODE_VIVID`.
- `WC_LEAGUE_ID` / `WC_SEASON` — TheSportsDB league id (4429 = FIFA World Cup)
  and season (`"2026"`).
- `RECENT_MATCHES_LIMIT` / `RECENT_VISIBLE_MATCHES` — how many finished matches
  the Recent Matches tab keeps, and how many show per screen before scrolling.
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
