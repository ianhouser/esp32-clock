# Contributing to KidClock

Welcome to the KidClock project! This is an ESP32-C3 nightstand clock for kids. To keep our codebase clean, history readable, and development organized, we strictly follow a GitHub Issue-driven Git workflow.

## Mandatory Git Workflow

Every code change must go through the following lifecycle. **AI agents contributing to this repository are explicitly forbidden from deviating from this flow or merging their own Pull Requests.**

### 1. Issue Creation & Validation
Before writing any code, there must be a tracked GitHub Issue describing the bug, feature, or task.
- Ensure the issue has clear requirements.
- The human user validates and approves the issue's scope.

### 2. Branching
Always create a new branch from `main` before starting work.
- **Naming Convention:** Branches **must** include the issue number and a descriptive slug.
- **Format:** `prefix/#-short-description`
- **Prefixes:** `feat`, `fix`, `chore`, `docs`, `refactor`, `test`
- **Example:** `feat/#3-clock-display` or `fix/#7-ntp-sync`

### 3. Implement & Commit
Write your code on the feature branch.
- Follow **Conventional Commits** for commit messages (e.g., `feat: add clock face rendering`, `fix: resolve NTP timezone offset`).
- Keep commits focused and atomic.

### 4. Push & Create PR
Once the work is complete and tested locally:
- Push the branch to the remote repository.
- Open a Pull Request against `main`.
- The PR description should link to the original issue (e.g., "Resolves #3").

### 5. Review & Remediate
The human user will review the Pull Request.
- If there is feedback or failing tests, push additional commits to the same branch.
- The PR updates automatically.

### 6. Human Merge
**Only the human user may merge Pull Requests.**
- AI assistants must stop at the PR creation or remediation phase and wait for the human user to click "Merge" on GitHub.
- Once merged, the branch can be deleted.

---

## Coding Standards

- **C++ (Firmware):** Use Arduino framework conventions. Header guards with `#pragma once`. Keep `.h` and `.cpp` paired in `src/` or `include/`.
- **Web UI:** Vanilla HTML/CSS/JS served from LittleFS. Mobile-first responsive design. No build tools for the web UI — keep it simple for the ESP32 to serve.
- **Pin Definitions:** All hardware pin assignments in `include/config.h`. Never hardcode GPIO numbers in source files.
- **Environment:** No secrets in the repo. WiFi credentials and location data are stored on-device in LittleFS JSON, excluded by `.gitignore`.

## Build & Flash

This project uses **PlatformIO**. See `docs/developer-guide.md` for:
- Installing PlatformIO
- Building firmware
- Flashing to ESP32-C3
- Uploading LittleFS filesystem
