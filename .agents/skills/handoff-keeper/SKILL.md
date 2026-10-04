---
name: handoff-keeper
description: >-
  Use when updating docs/agent-handoff.md after meaningful work or at session
  end. Keeps current focus, decisions, and next steps tight and truthful.
---

# Handoff keeper

`docs/agent-handoff.md` is the **now** file: what we are doing, what just changed, what happens next. It wins over other docs for current execution state (see hub read order).

## When to use

- After completing a non-trivial task, before claiming done (pairs with `verification-before-completion`).
- When blockers, milestone, or active task change.
- At session end if you touched project state.

## Format contract

Keep the template sections from the scaffold:

- **Last updated** — ISO date and local time (or UTC if team prefers), plus actor.
- **Current focus** — milestone link or `none`, one task line (issue id + single sentence), blockers or `none`.
- **Last three decisions** — ADR id or one line + date; rotate oldest off when adding a fourth.
- **What just happened** — **≤ 7** bullets (commits, PRs, deploys, decisions).
- **Next up** — **≤ 5** bullets, concrete and ordered where order matters.
- **Open risks / watch list** — short; link troubleshooting or tracker.
- **Pointers** — INDEX, active runbook, tracker URL.

## Rules

1. **Brevity** — if a bullet wraps past one line, split into an ADR, issue, or spoke; don’t grow the handoff into an essay.
2. **Truth** — if unsure, write the unknown explicitly or remove the stale bullet.
3. **No secrets** — tracker URLs and issue ids are fine; no tokens or internal credentials.
4. **ADR boundary** — record *that* a decision happened in handoff; put the reasoning in `docs/adr/` (do not duplicate full ADR text here).

## After editing

If any `.md` path changed meaningfully, invoke `docs-indexer` so `docs/INDEX.md` stays aligned.
