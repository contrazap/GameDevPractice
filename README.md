# Game Dev Practice Planning

Planning system for UE 5.8 practice building the full production skill base. Every future production game is coop and part of one episodic-DLC ecosystem (shared progression/collectibles across titles, Fortnite-style unified model). Production start is gated by this plan, not a date: the first game (GTFO-like or another) is green-lit at the P1 production gate (P1.12).

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
| UnifiedGameProject/ | The umbrella UE 5.8 C++ game project itself — created in P0.1 at this repo's root; every feature is built here. |

## The loop (per feature)

1. Pick the next feature from [ROADMAP.md](ROADMAP.md) (respect prereqs, prefer ⭐ core items).
2. Generate its plan — see the invocation block in [PLAN-GENERATOR.md](PLAN-GENERATOR.md). Use `primer` mode first for unfamiliar domains, `guided-build` when ready, `asset-build` per asset you choose to make yourself.
3. Build it in the practice game project, following the plan's [You] / [Claude] / [Pair] split.
4. Run the Definition of Done checklist (includes 2-client PIE smoke test and mobile budget check).
5. Recommended: do the plan's variation challenge — reimplement a variant yourself using only your notes — and/or run `quiz` mode for active recall.
6. Close the knowledge note in [knowledge/](knowledge/): Claude drafts the mechanics sections from the code; you write decisions, gotchas, would-do-differently.
7. Update the feature's status in [ROADMAP.md](ROADMAP.md).

At the end of each phase: **phase gate** — package to a real Android device, full cross-platform coop session test, device performance capture. Checklist in [CONVENTIONS.md](CONVENTIONS.md).

## How this relates to the game project

The UE project lives at `UnifiedGameProject/` under this repo's root (created in P0.1) — planning and implementation share one repo, so any agent session in this folder sees both the plans and the real code. One git repo covers everything: the UE-specific `.gitignore`/`.gitattributes` (LFS routing) live *inside* `UnifiedGameProject/` and apply to that subtree, while the planning files stay plain text at the root.
