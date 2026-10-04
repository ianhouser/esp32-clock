---
name: pr-copilot-review-remediation
description: Triage and remediate GitHub Copilot PR review threads with gh—implement blocking/should-fix items, defer or reply-and-resolve nits, resolve threads after push; re-request Copilot only when the user prompt asks (e.g. "with copilot review"). Use for Copilot feedback cleanup on a PR.
disable-model-invocation: false
---

# PR Copilot / bot review remediation

## Goal

Take a **PR number** (and optional head branch name), **triage** Copilot (and compatible bot) feedback, implement **high-signal** items, keep CI green, **resolve** threads once the outcome is on the branch (fix, reply, or explicit deferral), and **re-request Copilot** when—and only when—the user’s prompt includes an explicit trigger (e.g. `with copilot review`). **Do not** blindly implement every suggestion—follow **Triage** below.

## Triage (MUST do before writing code)

Classify each unresolved Copilot thread (or inline comment) into one of:

| Class | Examples | Default action |
|-------|----------|----------------|
| **Blocking** | Security, wrong score/money semantics, broken auth/session, data loss | **Implement** (or STOP per `AGENTS.md`) then resolve thread after push |
| **Should-fix** | Bug, regression, wrong deps, misleading UX, a11y break, type hole | **Implement** smallest fix; tests if warranted; resolve after push |
| **Nit** | Naming, micro-style, “consider extracting…”, hypothetical without evidence | **Defer** (follow-up issue or PR reply) **or** **reply + resolve** with one-sentence rationale; do **not** refactor by default |
| **Question** | Clarification only | **Reply** in thread; resolve when answered |

**MUST NOT**

- Treat “consider”, “might”, or architecture preferences as automatic work unless the PR’s stated scope includes that refactor.
- Stack refactors: if implementing a suggestion **creates** a new Copilot thread on the **same theme**, **revert or simplify** and document the tradeoff in a PR comment before adding more abstraction.
- Resolve a thread **before** the agreed outcome exists on the branch (fix, reply, or explicit won’t-fix).

**SHOULD**

- Batch low-impact nits into **one commit** and one summary where possible.
- Align with `AGENTS.md` (severity, noise control).

## Preconditions

- **`gh` authenticated** and repo remote matches GitHub (`gh auth status`, `gh repo view`).
- **`PATH`**: include Homebrew binaries, e.g. `export PATH=$PATH:/opt/homebrew/bin` (see `AGENTS.md`).
- Follow **`AGENTS.md`**: no AI attribution in commits; use **relative paths** in commit messages; do not close GitHub issues unless the user asked.

## Workflow (copy as checklist)

1. **Sync context**
   - `gh pr view <N> --json title,headRefName,baseRefName,url,state`
   - `git fetch origin && git checkout <headRefName> && git pull origin <headRefName>`
2. **If GitHub reports merge conflicts with base**: merge or rebase `origin/<base>` into the head branch, resolve (e.g. `src/config/settings.py` often needs **both** changelog entries when two branches add the same version—keep **newest `date` first**), commit, continue.
3. **List review feedback** (then **triage** each item per **Triage** above)
   - Latest Copilot review: `gh api repos/<owner>/<repo>/pulls/<N>/reviews --jq '.[] | select(.user.login=="copilot-pull-request-reviewer") | {id,body}' | tail -1`
   - Inline comments for that review:  
     `gh api repos/<owner>/<repo>/pulls/<N>/reviews/<reviewId>/comments`
   - Or enumerate **threads** (preferred for resolve): use GraphQL in step 5 **before** fixing to capture `threadId`s, or query after fixes to resolve all open Copilot threads. **Brace balance:** compact `reviewThreads{nodes{...comments{nodes{...}}}}` one-liners are easy to get wrong; generate the query with a short script or validate `{`/`}` counts before calling `gh api graphql`.
4. **For each comment thread (after triage)**
   - **Blocking / Should-fix:** implement the smallest correct fix; run targeted checks (`eslint`, `vitest` on touched tests, `npx tsc --noEmit`, or `npm run build` if warranted); **commit** with a clear message (e.g. `fix: address Copilot review on PR <N>`).
   - **Nit / out of scope:** prefer a **PR comment reply** (or follow-up issue) and **resolve** the thread after push if policy allows—or leave open if the repo expects human dismissal only.
   - Do **not** commit churn-only edits solely to silence low-value Copilot nits unless the user asked for a “clean sweep.”
5. **Resolve the conversation (required)**
   - **List threads** (set `N` to the PR number; owner/repo from `gh repo view --json nameWithOwner -q .nameWithOwner`):

```bash
N=531
gh api graphql -f query="query { repository(owner: \"ianhouser\", name: \"fbb-gh\") { pullRequest(number: $N) { reviewThreads(first: 50) { nodes { id isResolved comments(first: 10) { nodes { databaseId author { login } } } } } } } }"
```

   - For each **unresolved** thread whose comments include **`copilot-pull-request-reviewer`**, after triage and after the outcome is **pushed**:
     - Optional: reply on the comment via REST:  
       `gh api repos/OWNER/REPO/pulls/$N/comments -f body='…' -F in_reply_to=<commentDatabaseId> -f commit_id=<headSha> -f path=<file>`
     - **Resolve** the thread (use the `id` from the query, e.g. `PRRT_kwDOR7R5sM6CNm6l`):

```bash
gh api graphql -f query='mutation { resolveReviewThread(input: {threadId: "PRRT_..."}) { thread { isResolved } } }'
```

   - To resolve **several** threads in one request:

```bash
gh api graphql -f query='mutation { a: resolveReviewThread(input: {threadId: "PRRT_id1"}) { thread { isResolved } } b: resolveReviewThread(input: {threadId: "PRRT_id2"}) { thread { isResolved } } }'
```

   - Repeat until **all Copilot threads you committed to close** are resolved (fixed or answered). Threads intentionally deferred SHOULD be called out in the PR summary for the maintainer.

6. **Push & Verify** `git push origin <headRefName>` and confirm **`gh pr checks <N>`** (or CI on GitHub) passes.
7. **Re-request Copilot Review (only when the user asks)**
   - If the user's prompt includes a trigger such as `with copilot review`, `request review`, or similar, **MUST** re-request after push and thread resolution:
     ```bash
     gh pr edit <N> --add-reviewer copilot-pull-request-reviewer
     ```
   - If the user **did not** include that trigger, **do not** add Copilot as a reviewer (avoid an unsolicited nit wave).
   - Confirm in your final summary: what was implemented, what was deferred/replied, and whether Copilot was re-requested.

## Defaults for this repository

- **Remote owner/repo**: `ianhouser/sonicrelay` unless `gh repo view` says otherwise.
- **Common conflict**: `src/config/settings.py` — merge both entries for the same `version` when dates differ; sort **descending by `date`**.

## What not to do

- Do not “resolve” threads **before** the fix exists on the branch—resolve **after** push so the thread matches current code.
- Do not dismiss human reviewer threads unless the user explicitly asked.
- Do not edit the **original** GitHub issue description for status (use **comments** with the `AGENTS.md` agent prefix if posting updates).
- Do not implement **every** Copilot suggestion by default; triage first (**Triage**).
- Do not re-request Copilot unless the user’s prompt includes an explicit review trigger (`with copilot review`, etc.); unsolicited bot passes add noise.

## Quick reference: PR from number only

```bash
N=531
BR=$(gh pr view "$N" --json headRefName -q .headRefName)
git fetch origin && git checkout "$BR" && git pull origin "$BR"
```

Then steps 3–7 above.
