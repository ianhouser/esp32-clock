---
name: writing-skills
description: >-
  Use when creating or updating agent skills so they stay consistent,
  testable, and easy to discover.
---

# Writing skills

Skills are **programs for agents**. Treat them like code: one clear purpose, imperative steps, verifiable behavior.

## Locations

| Context | Path |
|---------|------|
| **Starter / scaffold source** | `.scaffold/seed-skills/<skill-name>/SKILL.md` |
| **Installed project (Cursor)** | `.cursor/skills/<skill-name>/SKILL.md` |
| **Installed project (Claude Code)** | `.claude/skills/<skill-name>/SKILL.md` |

After editing under `.scaffold/seed-skills/`, run `scripts/sync-skills.sh` from the repo root to mirror into `.cursor/skills/` and `.claude/skills/` while authoring this starter.

## File contract

- One directory per skill: `<skill-name>/SKILL.md`.
- YAML frontmatter with `name` (slug) and `description` (what triggers the skill).

## TDD-style loop for new skills

1. **Intent** — one problem statement in a sentence.
2. **Draft** — imperative markdown; MUST / MUST NOT where appropriate.
3. **Dry run** — ask another agent session (or human) to execute only from the skill; watch where they drift.
4. **Tighten** — remove ambiguity; add examples; shorten prose.

## Checklist

- [ ] Frontmatter present and accurate.
- [ ] “When to use” and “When not to use” (if non-obvious).
- [ ] No host- or employer-specific secrets or credentials.
- [ ] Tested or reviewed on at least one realistic task.

## Maintenance

Archive or rewrite skills that are never used or are routinely ignored — stale skills train agents to ignore the tree.
