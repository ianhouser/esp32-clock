---
name: verification-before-completion
description: >-
  Use before claiming work is complete, fixed, or passing; before commits or
  merge requests. Demands fresh command output as evidence.
---

# Verification before completion

Claims without evidence are worse than silence. **Evidence before assertions**, always.

## Iron law

```
NO COMPLETION CLAIMS WITHOUT FRESH VERIFICATION EVIDENCE
```

## Gate (follow in order)

1. **Identify** — which command(s) or checks prove the claim for *this* repo? (tests, lint, build, manual script — cite `docs/developer-guide.md` when it exists.)
2. **Run** — execute the full command **now** in this session (not “I ran it earlier”).
3. **Read** — full output; capture non-zero exit codes and failing tests.
4. **Verify** — does output actually support the exact claim you intend to make?
5. **Report** — state the claim **with** command(s) and a one-line outcome summary.

## Handoff (when project docs exist)

If `docs/agent-handoff.md` is present and the task was non-trivial, update **Current focus**, **What just happened**, and **Next up** after verification succeeds. Follow the **`handoff-keeper`** skill for structure and limits.

## Red flags

- “Should work”, “probably fine”, “seems green”.
- Celebrating completion before reading command output.
- Relying on cached or stale CI from before your latest edits.

## Bottom line

Run the checks. Read the output. **Then** describe status.
