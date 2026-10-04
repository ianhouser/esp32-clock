---
name: docs-indexer
description: >-
  Use when docs/INDEX.md must reflect the repo’s markdown files: add rows,
  refresh summaries and dates, without rewriting stable rows blindly.
---

# Docs indexer

Maintain `docs/INDEX.md` as a **card catalog**: paths, categories, last-updated dates, and **one-sentence** summaries. No long prose in the index.

## When to use

- After adding, renaming, or materially editing any `.md` under the repo that should appear in the catalog (usually everything except generated noise).
- When the user asks to “reindex docs” or refresh the catalog.

## Out of scope

- **Never** index or edit content inside `docs/_archive/` unless a human explicitly asks you to touch archived material.
- **Never** use the index to replace spokes — it only points to them.

## Input window (low temperature)

For each candidate `.md` file:

1. Read at most the **first 200 lines**.
2. Derive summary from the **first H1** and the **first paragraph** (first non-empty block after the H1) only.
3. **Summary rules:** one declarative sentence, no marketing tone, no second person.

## Updated column

Prefer **file mtime**; if unreliable (e.g. checkout quirks), use  
`git log -1 --format=%cs -- <path>` for the file’s last commit date.

## Idempotency

- If a row already exists for path `P` and the source file’s effective date is **unchanged** since the last index pass, **do not** rewrite the summary or category unless you are fixing a verified mistake.
- If `P` changed on disk, recompute summary and Updated.

## Determinism rule

The summary for path `P` must derive from ONLY:

1. The first H1 text (verbatim subject)
2. The first non-empty paragraph after H1 (extract one declarative verb phrase)

Format: "[Subject] — [verb phrase, one clause, no opinion]."

Two agents processing the same file using these rules MUST produce semantically identical summaries. Do not incorporate information from later sections, conversation context, or other files.

## Categories (fixed — pick exactly one)

`state` · `scope` · `architecture` · `decision` · `runbook` · `troubleshooting` · `developer` · `reference` · `archive` · `other`

| Hint |
|------|
| `docs/agent-handoff.md` | `state` |
| `docs/product-brief.md` | `scope` |
| `docs/adr/*` | `decision` |
| `docs/_reference/*` | `reference` |
| `docs/runbooks/*` | `runbook` |
| `docs/troubleshooting/**` | `troubleshooting` |

## Table shape

Preserve the existing markdown table header in `docs/INDEX.md`. Append or update rows; keep paths repo-root-relative (e.g. `docs/architecture.md`).

## Verify

- Reread `docs/INDEX.md` — no duplicate paths, no rows under `_archive/` unless explicitly requested.
