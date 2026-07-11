# Plan Generator

Reusable instruction set. One invocation generates **one** feature plan into `features/`. Paste the invocation block below into a Claude Code session running in this folder (the game project lives at `UnifiedGameProject/` in this same repo, so real code is always in reach — real code beats assumptions).

## Invocation (copy, fill, paste)

```
Read PLAN-GENERATOR.md in this folder and follow it exactly.
Feature: <ID, e.g. P1.3>
Mode: <primer | guided-build | variation | quiz | asset-build>
Asset: <asset-build mode only — the asset's name from the feature plan's Assets section>
Notes: <optional — time constraints, scope tweaks, what's already done, device on hand>
```

## Modes

| Mode | When | Output |
|---|---|---|
| `primer` | Before building something unfamiliar | Concepts, engine systems involved, architecture sketch, design rationale from the reference games. No step-by-step build. Ends with 3–5 readiness questions for the user to self-check. |
| `guided-build` | Ready to build | The full plan (all sections below). If a primer for this feature exists, don't repeat it — reference and extend it. |
| `variation` | Feature is ✅ done, user wants retention practice | A spec for a *variant* of the feature to be built by the user alone (no implementation guidance, no code). States what may be consulted: own knowledge notes + engine docs, not the original implementation or Claude. Includes its own small DoD. |
| `quiz` | After building, to test understanding | 6–10 questions about the actual implementation (reference real class/function names from `UnifiedGameProject/`), from "explain the flow" to "what breaks if X". Provide answers in a collapsed section at the end. May append results to the knowledge note instead of creating a plan file. |
| `asset-build` | An asset from a feature plan's Assets section will be self-made (or needs heavy Meshy cleanup) | A per-asset creation plan extending ASSET-PIPELINE.md: reference/concept step, DCC workflow (blockout → detail → retopo → UVs → bake → ORM → export preset → UE import + verification), the [You]/[Claude]/[Pair] learning split applied to Blender work, budget rows from CONVENTIONS.md, and its own small DoD. File: `features/<featureID>-asset-<slug>.md`. Never blocks the feature — the placeholder ships first. |

## Procedure

1. **Read first**: `CONVENTIONS.md` → the feature's entry in its `phases/P<n>-*.md` file → its row/neighborhood in `ROADMAP.md` (what's done, what's parked) → relevant `SKILL-MATRIX.md` rows → `ASSET-PIPELINE.md` if assets are involved → related `knowledge/` notes, `reference/` snapshots, and prior `features/` plans (build on what exists; never re-teach what a knowledge note says the user already knows). Plans must cite **in-repo** sources for anything they depend on — if a plan needs external material (another project's code or docs), snapshot it into `reference/` first; external projects are provenance, not dependencies.
2. **Thin entries**: P2–P5 phase entries are brief. If the entry is too thin to plan from, first propose an expanded 3–6 sentence feature description, get the user's confirmation (or adjustment), write it back into the phase file, then generate the plan.
3. **Clarify or proceed**: If something genuinely changes the plan's shape, ask at most 3 questions before generating. Otherwise proceed with stated assumptions.
4. **Size it**: A plan must fit **2–6 sessions** (per CONVENTIONS). If bigger, propose a split into `P<x>.<n>a/b/...` (update the phase file and ROADMAP with the split), then generate the plan for the first part only.
5. **Verify UE 5.8**: Check engine APIs/limits the plan relies on against the installed engine or official 5.8 docs. Anything unverifiable gets a ⚠️ entry in the Verify list — never silently trust training data.
6. **Budgets come from CONVENTIONS.md** — reference them, don't invent new numbers.
7. **Respect the learning split**: new concepts and core architecture go to **[You]**; boilerplate, second instances, and debug assistance to **[Claude]**; hairy integrations to **[Pair]**. A plan where [Claude] does all the interesting parts is a failed plan.
8. **Write the file**: `features/<ID>-<slug>.md`, ≤ ~250 lines, then update the feature's status in ROADMAP.md (⬜ → 📋). Touch nothing else in ROADMAP.
9. **Design rationale is part of learning**: the plan explains *why* the reference game does it this way (e.g. why GTFO's terminals slow the pacing), not just how to code it.

## Output template (`guided-build`; other modes use the subset that applies)

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

## Goal & design rationale
What we're building, which reference game(s) it draws from, and why it works in those games.

## Primer
Concepts, UE systems/classes involved, architecture sketch (ASCII ok). Sized to what the user doesn't already know (check knowledge notes).

## Scope
**In:** … **Out:** … **Cut lines:** what to drop first if the session estimate busts.

## Assets
Per asset: the recommended acquisition route per ASSET-PIPELINE.md **and** the self-build alternative, each with an estimated session cost so the choice is informed (self-build → generate an `asset-build` plan; it runs parallel and never blocks the feature — placeholder first). Budget targets per CONVENTIONS.md. Or "None".

## Steps
Grouped by session. Each step tagged [You] / [Claude] / [Pair] with a one-line why for [You] tags.

## Multiplayer design (mandatory for gameplay features)
Authority model, what replicates (properties/RPCs), client prediction concerns, what the 2-client PIE smoke must show.

## Mobile & performance (mandatory)
Which budget rows apply, expected costs, risks, fallbacks. Deviations justified here.

## Definition of Done
Base checklist from CONVENTIONS.md + feature-specific checks.

## Variation challenge
One self-build variant spec (what to change, what success looks like).

## Knowledge note prompts
3–6 questions the user's knowledge note should answer for this feature.

## ⚠️ Verify list
Engine specifics assumed but not verified against 5.8, each with how to verify.
```
