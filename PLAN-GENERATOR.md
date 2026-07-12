# Plan Generator

Reusable instruction set. One invocation generates **one** feature plan into `features/`. Paste the invocation block below into a Claude Code session running in this folder (the game project lives at `UnifiedGameProject/` in this same repo, so real code is always in reach — real code beats assumptions).

**Format principles (revised 2026-07-12, twice — see rationale at bottom):**
1. **Teaching lives at the point of use.** One document per feature: orientation up front, then per-session **Teach → Steps**. Separate primers/variations/quizzes are retired — separated teaching decayed before use; separately-invoked practice never got run.
2. **Two-stage generation.** The plan is generated as a **container** (Why & what, Mental model, prerequisites, session *outline*, cross-cutting sections, DoD, Variation, KN prompts, Verify list). Full **session detail** (Teach + Steps) is generated **just-in-time at the start of the session that runs it**, against the repo as it actually exists then. Reason: steps written against a repo state that doesn't exist yet cannot be verified and read as authoritative anyway — that's how fabricated paths ship.
3. **The plan is also the progress record.** A `**Next action:**` pointer at the top and an append-only `## Session log` at the bottom make any fresh Claude session resumable with one read. Claude owns all updates to both (CONVENTIONS rule 10).

## Invocation (copy, fill, paste)

```
Read PLAN-GENERATOR.md in this folder and follow it exactly.
Feature: <ID, e.g. P1.3>
Mode: <guided-build | phase-primer | asset-build>
Asset: <asset-build mode only — the asset's name from the feature plan's Assets section>
Notes: <optional — time constraints, scope tweaks, what's already done, device on hand>
```

To expand the next session of an existing plan mid-feature, no invocation is needed — "read `features/<ID>-*.md` and continue" triggers session-detail generation (procedure 16).

## Modes

| Mode | When | Output |
|---|---|---|
| `guided-build` | Ready to build | The container plan (template below); session detail arrives JIT per session. |
| `phase-primer` | Rare, on explicit request only: an upcoming phase is dense with unfamiliar engine systems (e.g. rendering before P2) | A phase-level systems tour: concepts, how the systems relate, no build steps, ends with readiness questions. Never generated per-feature. |
| `asset-build` | An asset from a feature plan's Assets section will be self-made (or needs heavy Meshy cleanup) | A per-asset creation plan extending ASSET-PIPELINE.md: reference/concept step, DCC workflow (blockout → detail → retopo → UVs → bake → ORM → export preset → UE import + verification), the [You]/[Claude]/[Pair] split applied to Blender work, budget rows from CONVENTIONS.md, its own small DoD. File: `features/<featureID>-asset-<slug>.md`. Never blocks the feature — the placeholder ships first. |

**Retired modes:** per-feature `primer` (absorbed into Teach blocks), `variation` (spec ships inside the plan), `quiz` (retrieval happens through the knowledge-note conversation, CONVENTIONS rule 5).

## Procedure — container generation

1. **Read first**: `CONVENTIONS.md` → the feature's entry in its `phases/P<n>-*.md` file → its row/neighborhood in `ROADMAP.md` → relevant `SKILL-MATRIX.md` rows → `ASSET-PIPELINE.md` if assets are involved → related `knowledge/` notes, `reference/` snapshots, and prior `features/` plans. Plans cite **in-repo** sources for anything they depend on — external material gets snapshotted into `reference/` first; external projects are provenance, not dependencies.
2. **Pending-practice pickup**: scan earlier plans' `## Variation` sections for any whose `Do after:` trigger is at or before this feature and still unbuilt; list them under **Pending practice** in Why & what so they get scheduled, not silently dropped.
3. **Thin entries**: P2–P5 phase entries are brief. If too thin to plan from, propose an expanded 3–6 sentence feature description, get confirmation, write it back into the phase file, then generate.
4. **Clarify or proceed**: at most 3 questions before generating, only if the answer changes the plan's shape.
5. **Size it**: 2–6 sessions (per CONVENTIONS). Bigger → propose a split into `P<x>.<n>a/b/...` (update phase file + ROADMAP), then generate the first part only. There is **no line cap** — the session budget is the budget. (The old ≤400-line cap died 2026-07-12: correctly granular plans exceed it and the cap only pressured plans back toward vagueness.)
6. **Verify UE 5.8**: engine APIs/limits the container relies on get checked against the installed engine or official 5.8 docs. Unverifiable → ⚠️ Verify list. Never silently trust training data.
7. **Prerequisites declared**: the container opens with "Assumes you can: X, Y, Z" — each item either covered by a knowledge note (link it) or taught in a Teach block (say which session). No silent competence assumptions.
8. **Budgets come from CONVENTIONS.md** — reference, don't invent.
9. **Respect the learning split**: new concepts and core architecture → **[You]**; boilerplate, second instances, debug assistance → **[Claude]**; hairy integrations → **[Pair]**. A plan where [Claude] does the interesting parts is a failed plan.
10. **Sessions are outlined, not detailed**: per session, a short paragraph — goal, what exists when it's done, rough step scope, which skills are first encounters. Engine-stable facts already verified may be kept beneath the outline as a **Teach seed** (labelled `re-verify at session start`); never as numbered steps.
11. **Design rationale is part of learning**: the container explains *why* the reference game does it this way, not just how to code it.
12. **Write the file**: `features/<ID>-<slug>.md` with `**Next action:**` at top and empty `## Session log` at bottom; bump ROADMAP ⬜ → 📋. Touch nothing else in ROADMAP.

## Procedure — session detail (JIT, at session start)

13. **Read the container + session log first.** Then generate this session's `### Teach` + `### Steps` in place of its outline paragraph (outline text moves into the session header line).
14. **Static verification pass — every literal is a claim.** Before the detail ships, resolve every path, class name, module name, function signature, asset reference, and call site in it against the live repo/engine — Read/Grep/find/engine source/official docs. This is inspection, **not** execution: no builds, devices, or logins required. What genuinely can't be known until execution (device behavior, portal state, toolchain drift) goes to the ⚠️ Verify list instead of being asserted. A "mirror X" without an exact location, or a literal nobody checked, is a generation bug.
15. **Fading granularity.** Judge, from knowledge notes + prior plans + session logs, whether this session's [You] material is a **first encounter** (→ tutorial grade) or a **re-encounter** (→ how-to grade):
    - **Tutorial grade**: atomic steps — one edit site or one artifact per step, exact code/values, one path, no options, `**Done when:**` each. More than ~2 edits or >1 file → split into `Na/Nb/Nc`.
    - **How-to grade**: goal + constraints + reference to where it was learned; the user finds the path. Still ends with `**Done when:**`.
16. **Steps carry no explanation and no choices.** Concepts live in Teach; if a step needs explaining, Teach is missing something — fix Teach. Exception: a genuine **design decision** is its own step, options laid out with trade-offs, decision recorded in the session log.
17. **Teach blocks are just-in-time**: only what this session's steps use. Reference knowledge notes instead of re-teaching. First time a workflow operation appears (regenerate project files, packaging, adb…), its procedure appears inline; later features reference back instead of repeating.
18. **Variation rules** (container `## Variation`): include only if the feature's core skill recurs later (else `None — <reason>`). Default **in-project** enhancement; separate project only when portability itself is the skill. Must not implement a later roadmap feature (check ROADMAP; if the natural variation *is* one, write None and say so). Spaced: `**Do after:** <feature 1–2 ahead>`. Spec only: what to build, what may be consulted (own KN notes + engine docs; **not** the original implementation, not Claude), small DoD.
19. **Knowledge-note prompts** (container section): 3–6 questions answered **from memory first** at feature close via the KN conversation (CONVENTIONS rule 5); the note ends with `Had to look up:`.

## Output template (`guided-build` container; other modes use the subset that applies)

```markdown
---
id: P1.3
title: <name>
phase: P1
mode: guided-build
status: draft
skills: [<skill tags>]
maps_to: [<reference games>]
prereqs: [<feature IDs>]
est_sessions: <n>
networked: yes|no
mobile_sensitive: yes|no
assets_needed: yes|no
---

# <ID> — <Title>

**Next action:** <one line: exactly where work resumes — step ref or "generate Session N detail">

## Why & what
Goal, design rationale, locked decisions, scope (In / Out / Cut lines), assets,
Pending practice (or "none").
**Assumes you can:** <prereq skills, each linked to its KN note or naming the Teach that covers it>

## Mental model
The 3–5 concepts that organize the feature + an architecture sketch (ASCII ok).
Orientation only — mechanics belong in session Teach blocks. Ends with 3–5
readiness self-check questions.

## Session 1 — <name>   ← detail generated at session start (procedure 13–17)
<Outline paragraph: goal, end state, rough scope, first-encounter skills.
Optional Teach seed (labelled "re-verify at session start").>
When detailed, becomes:
### Teach
### Steps   (numbered; [You]/[Claude]/[Pair]; one-line why on [You]; **Done when:** each)

## Session 2..n — <name>
Same: outline until reached.

## Multiplayer design (mandatory for gameplay features)
Authority model, what replicates, client prediction concerns, what the smoke test shows.

## Mobile & performance (mandatory)
Applicable budget rows, expected costs, risks, fallbacks. Deviations justified here.

## Definition of Done
Base checklist from CONVENTIONS.md + feature-specific checks.

## Variation
Spec + `**Do after:** <feature>` + small DoD — or `None — <reason>`.

## Knowledge note prompts
3–6 questions for the KN conversation at feature close.

## ⚠️ Verify list
Only what can't be checked before execution, each with how/when to verify.

## Session log
Append-only. Dated entries: decisions made (+why), deviations from plan (+why),
traps hit (+fix) — raw material for the knowledge note. Claude writes it
per the trigger table (CONVENTIONS rule 10); steps get ✅ in place as they pass.
```

## Rationale trail (why these rules exist — don't relitigate without new evidence)

- **2026-07-12 (a):** primer/guided-build split + separate variation/quiz plans failed in practice (teaching decayed before use; separately-invoked practice never ran) → single teach-then-do doc, in-plan spaced variations, KN retrieval ritual. Grounding: worked-example fading, temporal contiguity, spacing effect, retrieval practice.
- **2026-07-12 (b):** P0.2 execution audit. Mega-steps ("API cleanup" = 6 changes, 3 files, 1 Done-when) stalled a beginner; switching to atomic confirm-verify steps produced zero rework across 6 changes. Pre-cooked answers in steps hid two generation errors (wrong Public/Private split; fabricated asset path) — hence: methods in Teach not answers in steps, static verification of every literal, fading granularity (Diátaxis: tutorial for the learner, how-to for the competent; our fade = the transition), two-stage generation (can't verify steps against a repo state that doesn't exist), Next action + Session log (multi-session continuity with Claude as bookkeeper).
