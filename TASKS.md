# Remaining Tasks

The application is functional but several enhancements are still needed to reach a polished, production‑ready state.  The list below enumerates these items, grouped by area, with brief notes on the current status and a suggested next step.

## Core Functionality

| Item | Status | Next Step |
|------|--------|-----------|
| **Model discovery** | Done | Model list is now populated from a directory (default `./models` or `--model-dir`). |
| **Color configuration** | Pending | Map `foreground`, `background`, `selected`, and `highlight` values to notcurses color pairs and apply them in all screens. |
| **Error handling** | Done | Comprehensive error checks added (missing binary, command launch failures). |
| **Signal handling** | Done | SIGINT is trapped to cleanly exit the UI loop. |
| **Logging / verbosity** | Done | `--verbose` flag prints configuration details. |

## Testing

| Item | Status | Next Step |
|------|--------|-----------|
| **Unit tests** | One integration test | Write tests for `kvStringToIdx`, `applyFavorite`, and command string construction. |
| **UI tests** | None | Use a headless notcurses test harness or mock `notcurses_getch` to simulate key sequences. |

## Build System

| Item | Status | Next Step |
|------|--------|-----------|
| **Makefile improvements** | Basic build & clean | Add `make test`, `make check`, and optional `CFLAGS`/`LDFLAGS` overrides. |
| **Cross‑platform support** | Linux only | Detect OS and provide build instructions for macOS and Windows (via MinGW). |

## Documentation

| Item | Status | Next Step |
|------|--------|-----------|
| **README** | Basic | Add usage examples, screenshots, and FAQ. |
| **API docs** | None | Generate Doxygen or similar documentation for the public API. |

## Packaging & Distribution

| Item | Status | Next Step |
|------|--------|-----------|
| **Installer / package** | None | Create a simple installer (e.g., `deb`, `rpm`, or `Homebrew` tap). |
| **Versioning** | None | Implement semantic versioning and expose version via `--version`. |

## Future Enhancements

| Item | Status | Next Step |
|------|--------|-----------|
| **Plugin system** | Not planned | Allow third‑party UI plugins to extend screens. |
| **Persistent state** | None | Save last used settings to a local config file. |
| **Remote model listing** | None | Integrate `curl` to fetch model metadata from a remote API. |

---

Feel free to pick any of the above items to work on next.  Each entry can be split into smaller subtasks if needed.
