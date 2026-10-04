---
name: brainstorming
description: >-
  Use before creative work: new features, components, behavior changes, or
  non-trivial refactors. Clarifies intent and design before implementation.
---

# Brainstorming

Turn ideas into a reviewed design through dialogue before writing implementation code.

Start from project context, then ask questions **one at a time**. When the design is clear, get explicit approval before implementation.

## Hard gate

Do **not** invoke implementation skills, write production code, or scaffold a new subsystem until you have presented a design and the user has approved it. Trivial one-line fixes negotiated inline are exempt only if the user clearly treats them as such.

## Checklist (in order)

1. **Explore context** — hub, spokes, recent commits, open issues in the tracker.
2. **Clarify** — one question at a time; purpose, constraints, success criteria.
3. **Propose options** — two or three approaches with trade-offs and a recommendation.
4. **Present design** — in sections; pause for approval on each major section when useful.
5. **Write design doc** — `docs/specs/YYYY-MM-DD-<topic>-design.md` (create `docs/specs/` if needed).
6. **User reviews the file** — point them at the path; incorporate feedback.
7. **Plan implementation** — invoke `writing-plans` to produce a stepwise plan.

## Principles

- Prefer **one question at a time** over a questionnaire dump.
- Prefer **multiple choice** when it speeds decisions without hiding nuance.
- **YAGNI** — cut scope that does not serve the agreed success criteria.
- **Incremental approval** — large designs in digestible chunks.
