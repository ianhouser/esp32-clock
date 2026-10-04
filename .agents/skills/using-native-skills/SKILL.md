---
name: using-native-skills
description: >-
  Mandatory first step: check project skills under .cursor/skills/ and
  .claude/skills/ before complex work so process stays aligned with this repo.
---

# Using native skills

## Overview

This repository uses a **skills-first** methodology. Reusable workflows (planning, debugging, verification, review) live as skills with a single `SKILL.md` per skill.

Canonical copies for **new projects** ship under `.scaffold/seed-skills/` and are mirrored into `.cursor/skills/` and `.claude/skills/` during scaffold (or via `scripts/sync-skills.sh` while authoring the starter).

## The rule

**Before taking substantial action, you MUST:**

1. List **both** `.cursor/skills/` and `.claude/skills/` (when present).
2. Identify which skills apply to the current task.
3. Confirm **branch and commit conventions** with this repo’s contributing rules (`AGENTS.md` / `CLAUDE.md` / `.cursor/rules/core/contributing.mdc`) — do not assume a host-specific branching flow.
4. Read each applicable `SKILL.md`.
5. Follow those instructions exactly.

## When to use

- **Start of a meaningful task** — check for a matching skill.
- **Bugs and test failures** — `systematic-debugging`.
- **New features or behavior changes** — `brainstorming`, then `writing-plans`.
- **Claiming done or opening a review** — `verification-before-completion`, then `requesting-code-review`.
- **Repo hygiene** — `repo-audit` (after you know which tracker CLI applies).
- **Creating or structuring tracker issues** — **`issue-architect`** (seeded from starter; **trim** the CLI table in Phase D so only your tracker’s commands remain).
- **Card catalog** — `docs-indexer` after doc adds/renames.
- **Session / task state** — `handoff-keeper` when updating `docs/agent-handoff.md`.
- **Retiring docs** — `archivist` when moving files to `docs/_archive/`.
- **Doc drift** — `self-healing-docs` before milestones or when contradictions appear.
- **New deploy target** — `runbook-author` when no baked runbook exists; adapt **`web-launch`** when the target is Next.js + Studio Share Launch.

## Common mistakes

- **Assuming you remember the skill** — re-read `SKILL.md` when scope changes.
- **Mixing “how” (skill) with “what” (ticket text)** — skills define process; the hub and spokes define product truth.
