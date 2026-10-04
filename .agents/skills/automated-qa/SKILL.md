---
name: automated-qa
description: Use this skill to solve a bug report or implement a feature from a GitHub Issue autonomously, including browser verification of the fix.
---

# Automated QA & Bug Fix Skill
This skill is designed to take an incoming bug report or feature request (via GitHub Issue), perform deep technical analysis, implement the fix, and verify it using automated browser testing and log analysis.

## 🎯 Goal
Resolve project issues autonomously with minimal user intervention, ensuring high code quality and visual excellence.

## 🛠️ Prerequisites
- [ ] `gh` CLI authenticated (`/opt/homebrew/bin/gh`).
- [ ] Browser subagent capability.

## 📋 Workflow
1. **[Phase 1: Deep Analysis]**:
   - [ ] Read the issue description using `gh issue view`.
   - [ ] Search the codebase for relevant file patterns mentioned or implied.
   - [ ] Attempt to reproduce the issue locally and capture terminal output.
   - [ ] **Autonomous Decision**: If the requirements are ambiguous, ask ONE targeted question for clarification. Otherwise, proceed to drafting a plan.

2. **[Phase 2: Implementation]**:
   - [ ] Create a feature branch using `gh issue develop`.
   - [ ] Implement the fix/feature following the project's design standards (Tailwind, responsive, no inline styles).
   - [ ] **Conditional**: If modifying the web app, bump the version in `web/package.json`.
   - [ ] Document changes in an implementation plan (if significant) or proceed directly if minor.

3. **[Phase 3: Automated Verification]**:
   - [ ] **Infrastructure Check**: Use the local dev server (e.g., `http://localhost:5173`) for verification.
   - [ ] **Browser Testing**: Use `browser_subagent` to navigate to the affected page.
   - [ ] **Visual Check**: Verify that the UI matches the "Premium Design" guidelines (no layout shifts, correct colors, responsive behavior).
   - [ ] **Functional Check**: Perform the actions described in the bug report to confirm the fix.
   - [ ] **Log Check**: Check terminal output for any new warnings/errors induced by the fix.

4. **[Phase 4: Closure Recommendation]**:
   - [ ] Comment on the issue using `gh issue comment` with the agent signature 🤖.
   - [ ] Provide a link to the walkthrough artifact.
   - [ ] Explicitly state: "Recommendation: This issue can be CLOSED."

## 🚦 Autonomous Decision Rules
- **Proceed if**: You can reproduce the issue or locate the offending code and have a clear fix path.
- **Stop and Ask if**: 
  - DB migrations are required on production-level data.
  - Secrets/Authentication flows need modification.
  - You encounter 5+ build failures on the same file.

## ⛔ Hard Boundaries
- **Single-Issue Scope**: ONLY work on the specific issue ID provided by the user. NEVER create, pick up, or work on issues that were not explicitly requested.
- **No Issue Creation**: NEVER create new GitHub issues, even if you identify related problems. Report findings to the user instead.
- **No Issue Closure**: NEVER close issues. Recommend closure in a comment, but leave the action to the user.
- **No Chaining**: NEVER invoke other skills (e.g., github-issue-architect) to generate follow-up work. Your job ends when the single requested issue is verified.

## 🧪 Success Criteria
- [ ] Issue is resolved and verified in a live browser session.
- [ ] No regression errors in logs.
- [ ] UI is responsive and follows premium aesthetics.

## 🔗 Resources
- [AGENTS.md](../../../AGENTS.md)
- [Project Structure](../../../docs/codebase-map.md)
