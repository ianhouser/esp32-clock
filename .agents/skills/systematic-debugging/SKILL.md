---
name: systematic-debugging
description: >-
  Use on bugs, test failures, or unexpected behavior before proposing fixes.
  Forces root-cause work over guess-and-check.
---

# Systematic debugging

Random edits waste time and hide regressions. **Investigate before you fix.**

## Iron law

```
NO FIXES WITHOUT ROOT CAUSE INVESTIGATION FIRST
```

## Phase 0 — Documentation (when present)

If `docs/troubleshooting/INDEX.md` exists, **search it first** for matching symptoms or slugs linked from runbooks. Reuse a known fix when it applies; still confirm in this codebase.

## Phase 1 — Root cause investigation

1. Read error messages and stack traces completely (file, line, assertion).
2. Reproduce with the **smallest** reliable steps; note flaky vs deterministic.
3. Gather evidence at boundaries (logs, request/response shapes, config).
4. Pinpoint where reality diverges from expected behavior.

## Phase 2 — Pattern analysis

1. Find a **working** analogue in this repo or dependency docs.
2. List concrete differences (inputs, versions, flags, environment).

## Phase 3 — Hypothesis and test

1. State one hypothesis: “X is the cause because Y.”
2. Make the **minimum** experiment to falsify or support it.

## Phase 4 — Fix and prove

1. Prefer an automated test that failed before the fix.
2. Fix the cause, not a symptom.
3. Invoke `verification-before-completion` before claiming resolution.

## Red flags

- “Try changing X and see.”
- “Quick patch now, root cause later.”
- Proposing code before you can explain **why** the bug happened.
