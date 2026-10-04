---
name: self-healing-docs
description: >-
  Use when reconciling evolving docs (handoff, INDEX, codebase-map) with ADRs
  and reality; flags conflicts, never silently rewrites immutable decisions.
---

# Self-healing docs

Reduce drift between **what we said we’d do** (ADRs, brief), **what we think we’re doing** (`docs/agent-handoff.md`), and **where things live** (`docs/codebase-map.md`, `docs/INDEX.md`).

## When to use

- Periodic hygiene before a release or milestone close.
- When the user reports “docs feel stale” or contradicting instructions.
- After large refactors that moved directories or renamed services.

## Read set (in order)

1. `docs/agent-handoff.md` — current claims.
2. `docs/adr/` — binding decisions (filenames + first sections; read full ADR only if a conflict is suspected).
3. `docs/codebase-map.md` — layout claims.
4. `docs/product-brief.md` — scope (optional but useful for scope creep).
5. `docs/INDEX.md` — catalog completeness (spot-check paths).

Optionally spot-check `docs/developer-guide.md` commands against the actual package scripts if the stack is Node/Python/etc.

## Automation and mirror drift (P6)

When any of these exist, include them in the **conflict report** if they disagree with docs:

| Signal | What to compare |
|--------|------------------|
| **Issue mirror** | `docs/issues/OPEN.md` / `index.json` vs tracker (spot-check one open issue); note if `./scripts/sync-issues.sh` is missing but `OPEN.md` claims automation. |
| **CI** | Last successful workflow or scheduled job vs age of `docs/issues/*` commits or header timestamp in `OPEN.md`. |
| **Pre-commit** | If hooks are configured, do not suggest disabling them to “fix” doc drift — fix content or hook config with human approval. |
| **Hub vs ADR** | Binding decisions in the hub still match ADR titles they claim to summarize. |

If `scripts/sync-issues.sh` exists but `docs/issues/index.json` is empty while the tracker clearly has open issues, flag **auth or script breakage** — do not fabricate issues in markdown.

## What you may change directly

- **`docs/agent-handoff.md`** — only to fix clear factual errors (wrong path, closed milestone still listed as active) or to reflect outcomes the user just confirmed. Prefer bullets over narrative.
- **`docs/INDEX.md`** — via **`docs-indexer`** rules only (do not freestyle a second index format).
- **`docs/codebase-map.md`** — propose concrete edits for stale trees or ownership; apply **after** user says yes if the change is non-obvious.

## What you must NOT change without explicit human instruction

- **`docs/adr/*.md` body** — never silently “fix” or modernize ADRs. If an ADR is wrong, produce a short **conflict report** recommending `ADR-00N-supersedes-…` or an amendment process; let humans edit ADRs.
- **Binding decisions in the hub** — same rule: report, don’t edit, unless the user asked to update the hub.

## Conflict report format

```markdown
## Doc reconciliation — [date]

| Topic | Source A | Source B | Recommendation |
|-------|----------|----------|----------------|
| … | … | … | Supersede via new ADR / update handoff only / update codebase-map |
```

## After reconciliation

1. Run **`docs-indexer`** if any indexed file moved or changed category.
2. Update **`handoff-keeper`** section **Open risks** if new drift remains unresolved.

## Passive detection (check without explicit invocation)

During normal work, if you observe any of these signals, run the conflict-report workflow above:

- A path in `docs/INDEX.md` returns file-not-found
- `docs/agent-handoff.md` references a closed milestone/issue as active
- A hub binding decision contradicts an ADR title or status
- `docs/codebase-map.md` lists a directory that does not exist

Do not wait for explicit invocation — these signals indicate drift that should be reconciled before continuing work.

## Tone

Diagnostic, not apologetic. Prefer tables over long prose.
