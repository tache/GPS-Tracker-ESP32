# AGENTS.md

This file defines how agents should work with FishDaddy across projects. It is intentionally limited to operating rules, collaboration norms, source-control expectations, security, debugging, coding style, documentation, and testing.

## Interaction

**Direct, respectful collaboration with evidence when it matters.**

- Always address FishDaddy as "FishDaddy".
- Treat them as a coworker and project owner.
- Say "I don't know" when that is the accurate answer.
- Prefer direct, practical communication over ceremonial status updates.
- Whenever possible, verify claims with citations, command output, source references, or file references.

## Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:

- State assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them; do not pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop, name what is confusing, and ask.
- Read the relevant code and documentation before editing.
- Ask before implementing mock mode. Prefer real data and real APIs unless a mock is necessary.

## Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:

- "Add validation" means write tests for invalid inputs, then make them pass.
- "Fix the bug" means reproduce the bug or capture evidence for it, then verify the fix.
- "Refactor X" means preserve behavior and ensure relevant tests pass.

For multi-step tasks, state a brief plan:

```text
1. [Step] -> verify: [check]
2. [Step] -> verify: [check]
3. [Step] -> verify: [check]
```

Strong success criteria allow independent execution. Weak criteria such as "make it work" require clarification.

## Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- Prefer simple, clean, maintainable solutions over clever or overly compact code.
- No features beyond what FishDaddy asked for or the specification requires.
- No abstractions for single-use code.
- No flexibility or configurability that was not requested.
- No error handling for impossible scenarios.
- Use modern language features only when they improve clarity and fit the surrounding code.
- If 200 lines could reasonably be 50, rewrite it.
- Ask: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:

- Make the smallest reasonable change that solves the assigned problem.
- Every changed line should trace directly to FishDaddy's request.
- Do not "improve" adjacent code, comments, or formatting.
- Do not refactor things that are not broken.
- Match existing style, even if you would do it differently.
- Do not rewrite or replace an existing feature or system from scratch without explicit permission.
- If you notice unrelated dead code or unrelated problems, mention them or create an issue; do not fix them in the current task.

When your changes create orphans:

- Remove imports, variables, and functions that your changes made unused.
- Remove files and tests only when your changes clearly orphaned them, or when FishDaddy approves.
- Do not remove pre-existing dead code unless asked.
- Never remove comments unless you can prove they are actively false.

## Security

**Protect credentials and private data before everything else.**

- Never read, print, transform, or process `.env` files.
- Stop immediately if you encounter API keys, passwords, certificates, tokens, or other credentials.
- Do not access files that are known or likely to contain credentials.
- Respect ignore files and project-specific security exclusions.
- Do not paste secrets into issues, logs, documentation, comments, or final responses.

## Shell And Filesystem

**Use the least surprising tools and keep working files where they belong.**

- Use non-login shells for command-line work unless the task explicitly requires FishDaddy's shell startup environment.
- Prefer `rg` and `rg --files` for searching.
- Never run `rm`, `rm -r`, or `rm -rf`. Do not ask for approval to run them.
- Never create, generate, or modify scripts that contain `rm`, `rm -r`, or `rm -rf`.
- Do not write or run ad hoc bash, Python, Ruby, or other scripts to inspect, transform, or automate repository work unless FishDaddy explicitly approves the script first.
- Use `apply_patch` for manual file edits.
- Do not create temporary working files in the repository root.
- Do not create working files in FishDaddy's home directory without explicit approval.
- Do not create working files in `/tmp`; use an approved project subdirectory or ask where the file should live.
- Organize durable notes, plans, diagnostics, and working documents in appropriate project subdirectories.

## Source Control

**FishDaddy controls history; agents explain options before git changes.**

- Do not run git commands unless FishDaddy explicitly asks for that specific git operation.
- Explain the intended git command and impact before FishDaddy runs it.
- Never use `--no-verify` when committing code.
- Never use destructive git commands such as `git reset --hard`, `git checkout --`, or force-push variants unless FishDaddy explicitly approves that exact operation.
- Do not revert or discard changes you did not make.
- If source-control history or working-tree state is confusing, stop and explain the current state before recommending action.

## GitHub And Issues

**Use the configured project workflow and leave a clear review trail.**

- Use the project's configured GitHub persona and issue workflow.
- Do not add generated-by footers unless FishDaddy explicitly requests them.
- When posting issue comments, summarize concrete changes, validation performed, and any remaining risk.
- If no code changes are needed, state that plainly.
- If code changes are made, leave issues open for FishDaddy review unless the project workflow says otherwise.

## Debugging

**Find the cause with evidence, then make the smallest verified fix.**

- Use systematic debugging: reproduce, collect evidence, isolate likely causes, make a minimal fix, then verify.
- Prefer logging and diagnostics over interactive breakpoints.
- When using Xcode, never create or modify breakpoints unless FishDaddy explicitly asks.
- Do not ignore logs, warnings, or test output. They often contain the actual failure.
- If a debugging tool or bridge appears disconnected, try the normal reconnect path before declaring the tool unavailable.

## Coding Style

**Make code read like it belongs here.**

- Avoid force unwraps and force casts in production code unless the invariant is proven locally and the safer form would materially reduce readability.
- In tests, prefer explicit requirements, typed helpers, or clear failures over force unwraps and force casts.
- When using Xcode, use standard Xcode-style file headers for Swift files.
- Source files such as Swift, Ruby, and Python files should keep a running change log near the top of the file, after the copyright, with one-line summaries for each edit using the form `// Codex Generated: version 1 - Short comment`.
- Comments should be evergreen. Do not write comments that depend on temporary context such as "new", "recently changed", or "during this refactor".
- Never name code as `new`, `improved`, `enhanced`, or similar temporal labels.

## Documentation

**Write durable notes that help the next engineer verify the work.**

- Documentation should be useful to both people and future agents.
- Name documents for their durable purpose, not temporary status.
- Keep project-specific implementation notes out of this file.
- When creating plans or specs, include enough context for another engineer to verify the work independently.
- Avoid root-level scratch files. Put working documents in the project's agreed documentation folder unless FishDaddy asks for a root-level file.

## Testing

**Prove behavior with clean output and explicit coverage.**

- Tests must cover the functionality being implemented.
- Unit tests, integration tests, and end-to-end tests are all expected unless FishDaddy says exactly: "I AUTHORIZE YOU TO SKIP WRITING TESTS THIS TIME".
- Never mark a test type as "not applicable" without that exact authorization.
- Test output must be pristine to pass.
- Do not ignore warnings, intermittent failures, simulator failures, or log noise.
- If logs are expected to contain errors, capture and test that expectation.
- If you cannot run a relevant test, explain why and state the residual risk.

## Reviews

**Lead with risks, then summarize context.**

- When FishDaddy asks for a review, default to a code-review stance.
- Lead with findings ordered by severity.
- Ground findings in file and line references.
- Include open questions or assumptions after findings.
- Put summaries after the issues, not before them.
- If no issues are found, say so directly and mention any remaining test gaps or residual risk.

## Project Details

**Project-specific commands, architecture, and workflows live elsewhere.**

See project specific details in **Project-Specific-Details.md**.
