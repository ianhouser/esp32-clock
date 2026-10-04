---
name: writing-plans
description: >-
  Use when you have an agreed spec or requirements for a multi-step change,
  before writing implementation code.
---

# Writing plans

Write implementation plans for an engineer who may have **no** prior context on this repo. Name concrete files, commands, and verification steps.

**Save plans to:** `docs/plans/YYYY-MM-DD-<feature-name>.md` (create `docs/plans/` if needed).

## Task granularity

Each step should be one focused action (order-of minutes, not days), for example:

- Write or adjust a failing test.
- Run the test and confirm it fails for the expected reason.
- Implement the smallest change that can pass.
- Run **project** test, lint, or typecheck commands (whatever this repo documents).
- Commit with a Conventional Commit message.

## No vague placeholders

Do **not** use:

- “TBD”, “TODO”, “fill in later” for substantive work.
- “Add appropriate error handling” without specifying behavior and boundaries.
- “Write tests for the above” without concrete cases or file paths.

## Task shape (example)

```markdown
### Task N: [Component or concern]
**Files:**
- Create: `path/to/new.ts`
- Modify: `path/to/existing.ts`

- [ ] Step 1: …
- [ ] Step 2: …
```

## Remember

- **Exact paths** for every touched file.
- **Explicit verify commands** from `docs/developer-guide.md` or the hub.
- Prefer **TDD** when the stack already practices it.
