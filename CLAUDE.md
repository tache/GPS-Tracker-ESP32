# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

@AGENTS.md

For collaboration norms, source-control rules, security, debugging, coding style, and testing
expectations, see **AGENTS.md**. For the strict GitHub workflow policy (FishDaddy owns all git
operations) and documentation conventions, see **claude.local.md**.

## Project Type

This is **GPS-Tracker-ESP32** — an ESP32-based GPS tracker. The toolchain is **embedded C/C++**
(the `.gitignore` targets `*.o`, `*.elf`, `*.hex`, `*.dSYM`), not iOS/Swift. Expect PlatformIO,
ESP-IDF, or the Arduino-ESP32 framework once code lands.

> Note: The global `~/.claude/CLAUDE.md` and several sections of `AGENTS.md`/`claude.local.md`
> were authored for iOS/Swift/Xcode projects (SwiftLint, XcodeBuildMCP, `WheelOfWhat.xcodeproj`).
> Those Swift-specific conventions do **not** apply here. The *operating rules* in AGENTS.md
> (surgical changes, evidence-based debugging, real-data-over-mocks, test coverage, FishDaddy
> owns git) still apply. Confirm with FishDaddy before applying any Swift-only guidance.

## Current State

As of the initial commit, the repository contains only `LICENSE`, `.gitignore`, and these
instruction files — **no source code and no build configuration yet**.

- **Build / flash / monitor commands:** Not yet defined. Document them here once a
  `platformio.ini`, `CMakeLists.txt`/ESP-IDF setup, or Arduino sketch exists.
- **Lint / format:** Not yet defined.
- **Test commands (incl. running a single test):** Not yet defined.
- **Architecture:** Nothing to describe yet. Update this file once multiple source files exist
  and the big-picture structure spans more than one file.

When the first real code or build config is added, re-run `/init` (or update this file directly)
to capture the actual commands and architecture.

## Working Documents

Per `claude.local.md`, save all generated docs to `./claude-working-documents/` using the
`YYYYMMDD-Descriptive-Name-In-Title-Case.md` naming convention. Durable project-specific
implementation notes belong in `Project-Specific-Details.md` (currently empty).
