# Agent Instructions: Workout Library C++ Expert

## Project Overview
*   C++23 shared library (`workoutlib`) that reads and writes Garmin FIT, plan, erg and mrc training files.
*   All module sources live in the repo root; tests in `testing/`; CMake helpers in `CMake/`; FIT test fixtures in `testfiles/`.
*   Single CMake project (Ninja). `CMakePresets.json` defines `Debug` and `Release` presets (clang++, `-stdlib=libstdc++`, UBSan, module-map and std-module flags).

## Build & Test
*   Configure and build: `cmake -S . -B build --preset Debug && cmake --build build`
*   A plain `cmake -S . -B build -G Ninja` configure also works, but the presets carry the project's intended flags (module map, std-module JSON, UBSan) — prefer them.
*   Tests run via `ctest` in the build dir (single entry `WorkoutTests`), or directly with gtest for one test: `./build/Tests --gtest_filter=<pattern>`.
*   The FIT file tests shell out to `java -jar /usr/lib/garminfit/FitCSVTool.jar` — `java` and that jar must be installed or those tests fail.
*   Coverage: the test target depends on `coverage_target`; `bash build/coverage.sh` produces `lcov.info` in the build dir. For VS Code Coverage Gutters: base dir `build`, report file `lcov.info` (the README's `coverage.lcov` name is wrong).
*   Toolchain: Clang >= 22, CMake >= 4.4, Ninja, Garmin FIT SDK (headers in `/usr/include/GarminFit`), gtest, libmd. On Arch: install deps via pacman and the Garmin FIT SDK PKGBUILD (see README).

## Generated & External Files
*   `build/config.cppm` is generated at configure time via `file(GENERATE)` (holds the `TESTING` bool) — never edit it by hand.
*   `fit_profile.cppm` is generated from the SDK's `fit_profile.hpp` by `strip_macros.py` (run via `strip_macros.sh`). Regenerate it after a Garmin FIT SDK upgrade; do not hand-edit its generated `constexpr` blocks.
*   `fit.cppm` is the module interface that wraps the SDK headers; `#include` directives appear only inside this file.

## Subagents
*   Agressively use subagents for tasks like codebase exploration (`explore`) or independent parallel tasks (`general`). Their context is fresh, so keep the main agent's context lean; their intermediate output never enters it.
*   Subagent prompts must be self-contained (paths, background, what to return) since they share none of the conversation.
*   MCP tools (context7, playwright) are available to the `general` subagent but **not** to the `explore` subagent (fixed tool set). The `libdocs` subagent gathers up-to-date library documentation via Context7.

## Coding Standards (Mandatory)
All code generated or analyzed must strictly adhere to the following project conventions:
*   **Language Standard:** Target C++23.
*   **Modules:** Prefer C++20 modules over traditional include directives. Don't remove import statements. If you think you need an #include, ask the user for an alternative approach.
*   **Error Handling:** Use `std::expected<T, std::string>` for all operations that can fail. The result aliases are declared in `common_declarations.cppm` (`voidReturn`, `stringReturn`, `uintReturn`).
*   **Monadic Chaining:** When composing several fallible operations, prefer a single monadic `std::expected` chain over sequential checks with early returns. `FitDataTestContainer::getFileContent()` in `testing/file_tests.cpp` is the reference example:
    *   Use `and_then` for steps that can fail; use `transform` for infallible value mappings.
    *   Carry `void` (`voidReturn`) through the intermediate steps and let the final step produce the actual value — never fabricate a placeholder value just to seed a chain. Seed the chain with the first fallible check itself (e.g. an IIFE returning `voidReturn`).
    *   The error type must stay identical across all `and_then` steps; the value type may change (e.g. `void` → `std::string` in the final step).
    *   For `expected<void, E>` the `and_then`/`transform` callable is invoked with **no arguments**; for `expected<T, E>` it is invoked with `T&&` on rvalues and `T&` on lvalues, so prefer `const T&` parameters when the callable must bind to both.
*   **Lambda Captures:** Function-local `constexpr` variables that are odr-used inside a lambda must be captured explicitly by value (e.g. `[FitCSV]`).
*   **Constants:** Avoid "magic numbers." All hardcoded values, limits, or configuration constants must be defined as `(static) constexpr` variables.
*   **Naming:** Member variables are named like `m_var`; all names follow camel case. Names must be at least 3 characters.
*   **Performance & Safety:** Use `noexcept` and `constexpr` aggressively wherever possible.

## Gotchas
*   `PKGBUILD` embeds a git PAT in its source URL — never log or leak it; it is only used for packaging on Arch-style systems.
*   Formatting is clang-format via `.clang-format` (79-column limit, 2-space indent).

## Interaction Style
When responding to a request:
*   **Plan First:** Before implementing any changes or doing tool calls, always outline your idea first and ask for confirmation. If you need to make assumptions, confirm them first with the user before proceeding. Before implementing a new feature, check if that can be implemented using existing code.
*   **Avoid ambiguities:** Always ask for clarifications, especially before making any architectural or design choices.
