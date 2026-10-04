---
name: archivist
description: >-
  Use when retiring active docs: move markdown under docs/_archive/ with dated
  paths, fix references, update INDEX — never silently delete knowledge.
---

# Archivist

Move **stale but valuable** docs out of the active agent path while preserving history and links.

## When to use

- A spoke is superseded (e.g. old runbook replaced) but should remain readable.
- A human asks to “archive” a doc or directory under `docs/`.

## Destination layout

```
docs/_archive/<YYYY-MM-DD>/<original-relative-path-from-docs>/
```

Example: retiring `docs/runbooks/v1.md` → `docs/_archive/2026-05-07/runbooks/v1.md`

Preserve directory shape under `docs/` so paths stay understandable.

## Process

1. **Confirm** — user explicitly wants archival (not delete). List what will move and what will replace it in the active tree.
2. **Move** — git mv preferred so history follows; otherwise copy + delete in one commit with a clear message.
3. **Fix references (exhaustive)** — Run a recursive search across the entire repo (`grep -r` or equivalent on the old path). Update every match in: `docs/**/*.md`, hub file, `AGENTS.md`, `.claude/skills/**/*.md`, `.cursor/skills/**/*.md`, `.claude/rules/**/*.md`, `.cursor/rules/**/*.md`, `README.md`, `scripts/`, and any other file containing the old path. Do not limit to `docs/` alone — skills and rules may also reference archived docs.
4. **INDEX** — invoke `docs-indexer`: active row removed or pointed to replacement; add `archive` category row for the archived path **only** if humans should discover it from the catalog; otherwise omit archived paths per indexer rules unless directed.
5. **Handoff** — one line under **What just happened** pointing to the archive location and replacement doc.

## Guardrails

- **Never** archive `docs/agent-handoff.md`, `docs/INDEX.md`, or the hub file — those stay live; rotate content instead.
- **Never** bulk-archive without per-path human confirmation when more than three files move.
- Agents default to **not** reading `_archive/` unless the user points there — state that in the handoff when relevant.

## Do not

- Delete archived markdown to “save space” without explicit approval.
- Rewrite ADRs into `_archive/` to hide them — ADRs stay under `docs/adr/` unless the project’s governance explicitly allows moving them (rare).
