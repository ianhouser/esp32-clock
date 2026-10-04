---
name: requesting-code-review
description: >-
  Use when finishing meaningful work or before merge: self-review, evidence,
  and tracker-appropriate review request.
---

# Requesting code review

Structured review catches integration issues before they land.

## When this applies

- Feature-sized change, risky refactor, or anything touching security, auth, persistence, or release paths.
- Before opening or updating a merge request / pull request.

## Process

1. **Summarize intent** — what changed, in plain language, linking issues or ADRs when relevant.
2. **Branch hygiene** — branch name and commit style follow this repo’s contributing rules (for Studio Tech Engineering starters: `{branch_type}-{ISSUE}-{description}` and Conventional Commits unless the hub overrides).
3. **Self-review** — walk your own diff; remove debug noise and dead paths.
4. **Evidence** — outputs from `verification-before-completion` (commands run, results).
5. **Open or refresh review** — use the tracker your project chose:
   - **GitHub:** `gh pr create` / `gh pr edit` as appropriate.
   - **GitLab:** `glab mr create` / `glab mr update` (or your documented CLI).
   - **Jira / other:** follow `docs/adr/ADR-002-issue-tracker.md` or team runbook; paste review links into the handoff file when used.

## Readiness statement

End with one of:

- **Ready for review** — checks green, risk called out, reviewers suggested.
- **Needs work** — known gaps listed with next steps.
- **Blocked** — external dependency; say what unblocks.

## Do not

- Add AI attribution lines to commits, MR titles, or descriptions (follow org contributing rules).
- Request review without listing what you **want** reviewers to scrutinize (risk areas, migrations, API edges).
