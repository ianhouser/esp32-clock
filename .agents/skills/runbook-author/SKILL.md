---
name: runbook-author
description: >-
  Use when the interview target has no baked runbook, or when extending a
  baked runbook for a new variant. Applies research → validate → author per
  docs/_reference/reference-runbook-protocol.md.
---

# Runbook author

Produce or extend **`docs/runbooks/<platform>.md`** so hello-world is repeatable and failures have a home under **`docs/troubleshooting/`**.

## When to use

- Interview Round 2 named a target **without** a matching baked runbook in this repo.
- The user asks to add or refresh a deployment / local-dev runbook.
- A baked runbook exists (e.g. **web-launch**) but the stack variant differs (monorepo app path, different DB adapter) and needs a documented delta.

## When not to use

- The user only wants a one-line command — answer inline; do not spawn a runbook.
- You cannot access required docs or networks — stop and record the blocker in `docs/agent-handoff.md` instead of inventing steps.

## Process (must follow order)

1. **Read** `docs/_reference/reference-runbook-protocol.md` (or `.scaffold/reference-runbook-protocol.md` during starter work).
2. **Check for baked** runbook under `docs/runbooks/` or `.scaffold/spoke-templates/runbooks/` — if `web-launch.md` fits (Next.js + Studio Share Launch), start from that template and document **only deltas** in the project runbook or in linked troubleshooting entries.
3. **Research** per protocol Phase 1 — version-pin sources; no secrets in notes.
4. **Validate** per protocol Phase 2 — human confirms hello-world criteria and prerequisites.
5. **Author** per protocol Phase 3 — use `_template.md` structure; cross-link troubleshooting.
6. **Record** per protocol Phase 4 — ADR alignment.
7. **Hello-world + INDEX + handoff** per protocol Phase 5 — invoke **`docs-indexer`** and **`handoff-keeper`** when those files exist.

## Outputs (checklist)

- [ ] `docs/runbooks/<platform>.md` exists and has Preconditions, Steps, Verify, When things fail, References.
- [ ] New or updated troubleshooting entries under `docs/troubleshooting/` with links from the runbook.
- [ ] `docs/INDEX.md` updated via **`docs-indexer`**.
- [ ] `docs/agent-handoff.md` pointers mention the runbook in play.

## Quality bar

- Prefer **links** to long platform docs over copying them.
- Prefer **tables** for env vars (name + purpose; values only in `.env.example` placeholders).
- If a step is “ask your platform admin”, say so explicitly instead of guessing URLs or IDs.
