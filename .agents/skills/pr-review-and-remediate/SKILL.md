---
name: pr-review-and-remediate
description: Use when conducting an end-to-end pull request review and remediation flow. Evaluates PR against overarching milestone strategy and code quality, posts the review comment to GitHub, implements and tests code remediations, pushes fixes to the branch, posts a remediation summary comment, and defines next roadmap steps.
---

# PR Review and Remediate

Use this workflow to conduct a full-cycle pull request review, verification, remediation, and GitHub thread update.

---

## Philosophy & Core Principles

1. **Strategic Purpose Over Surface Polish**:
   - Good code with misaligned purpose or bad architectural strategy is tech debt.
   - Always evaluate the PR against the milestone goals, product roadmap, and hardware/software constraints before inspecting syntax.
2. **Close the Loop**:
   - Do not just leave critique; implement the agreed fixes directly on the branch, run test/build validations, and update the PR with verified results.
3. **Transparent Audit Trail**:
   - Post the initial strategic review comment on GitHub.
   - Follow up with a remediation summary detailing exact commits, verified test outputs, and merge readiness.
4. **Preserve Next-Milestone Focus**:
   - Distinguish between blocking issues for the current PR and deferred architectural improvements for upcoming milestones.

---

## Step-by-Step Workflow

### Step 1: Inspect PR & Overarching Intent
1. Fetch PR details using the GitHub CLI:
   ```bash
   gh pr view <PR_NUMBER> --json title,body,headRefName,baseRefName,commits,files
   ```
2. Determine:
   - What issue/milestone does this resolve?
   - Does the implementation advance the overarching architectural goal without taking on unmaintainable debt?
   - Are there duplicated sources of truth (e.g., hardware pin configurations, constants)?
   - Are there hardcoded assumptions (e.g., display dimensions, blocking delays)?

### Step 2: Post the Strategic Code Review Comment
Post a structured review comment on the PR via `gh pr comment <PR_NUMBER>` covering:
- **Strategic Alignment & Purpose**: Evaluation against the project milestone and requirements.
- **Architectural & Code Quality Findings**: Clear, actionable items categorized by severity and rationale.
- **Remediation Plan**: Explicit list of changes you will implement on the PR branch.

### Step 3: Implement & Verify Code Remediations
1. Check out the PR branch if not already active:
   ```bash
   git checkout <headRefName>
   ```
2. Apply code modifications:
   - Enforce single source of truth for all shared configurations.
   - Replace hardcoded metrics with responsive, dynamic lookups.
   - Ensure resilient, non-blocking operational paths.
3. Run verification tests / build commands:
   - Ensure the command outputs fewer than 50 lines (e.g., pipe to `tail -n 25`).
   - Confirm clean compilation and passing tests.

### Step 4: Commit & Push Changes
1. Stage and commit changes with a descriptive conventional commit message:
   ```bash
   git commit -am "refactor: address code review recommendations for <scope>"
   ```
2. Push to the appropriate remote:
   ```bash
   git push <remote> <headRefName>
   ```
   *(Note: If pushing to GitHub, prefer HTTPS remote or credential-managed remote to avoid interactive credential hangs).*

### Step 5: Post Remediation Summary & Merge Readiness
Post a follow-up comment on the PR via `gh pr comment <PR_NUMBER>` detailing:
- **Actions Taken**: Bulleted summary linking commits to review feedback.
- **Verification Evidence**: Build/test results confirming zero errors/warnings.
- **Readiness Recommendation**: Explicit recommendation on whether the PR is ready to merge.
- **Deferred Roadmap**: Concrete recommendations for the next milestone.
