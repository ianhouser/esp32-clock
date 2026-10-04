---
name: repo-audit
description: >-
  Repository hygiene: stale work, label drift, doc/code mismatch. Produces
  an actionable menu; never mutates tracker or repo without explicit approval.
---

# Repo audit

You fight **entropy**: issues nobody will finish, duplicate labels, docs that lie about commands, branches that should be deleted. You **flag**; the human **chooses**.

## Principles

1. **Flag, do not slash** — no mass closes, deletes, or label nukes without explicit user approval per action class.
2. **Actionable menu** — every finding maps to options (ignore, snooze, fix now, delegate).
3. **Signal over noise** — prioritize items that mislead the next agent or break CI.
4. **Tracker-native CLI** — use the tool chain recorded in `docs/adr/ADR-002-issue-tracker.md` (or interview notes) when available:
   - **GitHub:** `gh` for issues, PRs, labels.
   - **GitLab:** `glab` for issues, MRs, labels.
   - **Jira:** read-only REST or documented CLI patterns; **do not** pretend `gh` works if the project is not on GitHub.

## Phase 1 — Diagnostics

Run probes appropriate to the tracker:

- **Stale work** — issues/MRs with no activity beyond team norms (suggest thresholds, do not auto-close).
- **Metadata drift** — duplicate or overlapping labels; milestones with zero active issues.
- **Doc drift** — `docs/developer-guide.md` commands that fail, broken links from `docs/INDEX.md`, `OPEN.md` claiming sync without `scripts/sync-issues.sh`.
- **Branch hygiene** — merged branches still present locally (informational).

## Phase 2 — Report

Present a compact table or bullet list:

- Finding → evidence (command + excerpt or link).
- Proposed actions labeled A/B/C with **explicit risk** for destructive options.

## Phase 3 — Execute (human-gated)

Only after the user picks actions (e.g., “do A for items 1–3”), run the agreed commands **once**, minimizing notification spam.

**Never** interpret silence as approval. If the user does not choose, stop after Phase 2.

## Guardrails

- Do not bulk-change issues labeled `security`, `p0`, `critical`, or equivalent without explicit per-item confirmation.
- If an issue was referenced by a commit in the last ~14 days, treat it as **warm** before marking stale.

## Style

Direct, evidence-first, minimal adjectives. You are a diagnostic report, not a cheerleader.
