# Game Dev Practice Planning

Planning system for UE 5.8 practice leading into the real GTFO-like coop game (development starts September 2026, other episodes/DLC after its release).

Created 2026-07-09. Feature plans are generated **on demand** — this folder starts with structure + phase overviews only.

## Folder map

| File / folder | Purpose |
|---|---|
| [ROADMAP.md](ROADMAP.md) | Level 0 — all phases and features as one-liners with status. The menu + cut line. |
| [phases/](phases/) | Level 1 — one file per phase, a paragraph per feature (id, scope, skills, prereqs). |
| [features/](features/) | Level 2 — detailed feature plans, generated on demand via the generator. |
| [PLAN-GENERATOR.md](PLAN-GENERATOR.md) | The reusable instruction set that generates one feature plan per invocation. |
| [CONVENTIONS.md](CONVENTIONS.md) | IDs, statuses, working rules, budgets, definition-of-done checklists. |
| [ASSET-PIPELINE.md](ASSET-PIPELINE.md) | Meshy → Blender → UE workflow, free asset sources, cleanup checklists. |
| [SKILL-MATRIX.md](SKILL-MATRIX.md) | The 17 reference games → skill domains → where each is covered. |
| [knowledge/](knowledge/) | Your personal wiki — one note per finished feature (how it works + gotchas). |

## The loop (per feature)

1. Pick the next feature from [ROADMAP.md](ROADMAP.md) (respect prereqs, prefer ⭐ core items).
2. Generate its plan — see the invocation block in [PLAN-GENERATOR.md](PLAN-GENERATOR.md). Use `primer` mode first for unfamiliar domains, `guided-build` when ready.
3. Build it in the practice game project, following the plan's [You] / [Claude] / [Pair] split.
4. Run the Definition of Done checklist (includes 2-client PIE smoke test and mobile budget check).
5. Recommended: do the plan's variation challenge — reimplement a variant yourself using only your notes.
6. Write the knowledge note into [knowledge/](knowledge/).
7. Update the feature's status in [ROADMAP.md](ROADMAP.md).

At the end of each phase: **phase gate** — package to a real Android device, full cross-platform coop session test, device performance capture. Checklist in [CONVENTIONS.md](CONVENTIONS.md).

## How this relates to the game project

The UE project lives in its own repo (you create it in P0.1). Plans and knowledge notes live here. When generating or executing plans with an agent, give it access to both locations (or paste the relevant plan into the game repo session). Git for this folder is handled by you.
