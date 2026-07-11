# Conventions

Shared rules for all plans. The plan generator reads this file — change it here, and every future plan follows.

## IDs, files, statuses

- Feature ID: `P<phase>.<n>` (e.g. `P1.3`). Sub-split features: `P1.3a`, `P1.3b`.
- Feature plan file: `features/P1.3-short-slug.md`. Knowledge note: `knowledge/KN-short-topic.md`.
- Status markers used in ROADMAP.md:
  - ⬜ not started · 📋 plan generated · 🔨 in progress · ✅ done · 🅿️ parked
  - ⭐ marks core-priority items (do these before non-⭐ items of the same phase if time is short).
- A **session** = one focused work block of ~2–4 hours. Estimates in plans are in sessions and assume Claude-assisted pace (like the 1-day EOS test), not solo-from-docs pace.

## Working rules

1. **Author to mobile budgets, always.** Every feature is designed within the budgets below even though it's built and tested on Windows first. Desktop scales *up* (resolution, shadows, post-processing) via scalability/device profiles; content is never authored desktop-only unless the plan explicitly justifies it. This is what makes the per-phase Android gate a formality instead of a rewrite.
2. **Replication-aware, always; device tests per phase.** Gameplay features are structured server-authoritative with replication designed in (even when it feels like overkill). Per feature: a **2-client PIE smoke test** (editor "Number of Players: 2", listen server — seconds to run). Per phase: real Android device packaging + full EOS cross-platform session test. Pure-local UI (minigames, menus) is exempt from replication.
3. **Verify against UE 5.8.** Agent training data may predate 5.8. Any engine API, renderer capability, or limit a plan relies on gets checked against the installed engine source / official 5.8 docs; unverified items are marked ⚠️ in the plan's Verify list.
4. **Learning split.** Steps in plans are tagged:
   - **[You]** — first-of-a-kind concepts and core architecture. This is where learning happens; plans must put the new ideas here.
   - **[Claude]** — boilerplate, repetition, second instances of a pattern, debugging assistance.
   - **[Pair]** — tricky integrations done together (you drive, Claude navigates — or the reverse with you reviewing every step).
5. **Knowledge notes are mandatory** per finished feature (template below) — but split by author so they're never busywork: **[Claude]** drafts the mechanics sections (*What was built*, *How it works*, *Links*) from the session's code at feature close; **[You]** write only *Decisions & why*, *Gotchas & fixes*, and *Would do differently* (~5 min — the parts only you know). Repetition retains skill; notes retain **decisions and gotchas**, calibrate future primers (the plan generator reads them to avoid re-teaching), and are the only reference allowed in variation challenges. Optional recall step at feature close: run the generator's `quiz` mode and append the results to the note.
6. **Implementation split: C++ / Blueprint / UMG.** Logic in **C++**: systems, gameplay, subsystems, base classes, save/load, anything reused or perf-sensitive — and **replication always**. **Blueprint** for content wiring and tuning: `BP_` subclasses of C++ bases holding asset references (mesh/sound/VFX) and designer-tunable values; level scripting; quick prototypes (promote to C++ if they survive). **UI:** logic in a C++ `UUserWidget` base, layout/animation in UMG (`WBP_`); raw Slate only for editor tooling or engine-level cases where UMG isn't available. **Data** in `DA_` Data Assets / DataTables, not hardcoded. One-liner: *logic in C++, content and tuning in Blueprint, layout in UMG.* Plans tag steps to match; deviations are justified in the plan.

## Mobile budgets (PROVISIONAL — validate and update in P0.5 device profiling)

Targets: 30 fps floor on a mid-range Android device, 60 fps desktop.

| Item | Budget |
|---|---|
| First-person weapon (viewmodel) | ≤ 20k tris, ≤ 2k textures |
| Enemy / character | 8–20k tris, ≤ 2k texture atlas, lean skeleton (≲ 75 bones ⚠️ verify 5.8 mobile limits), ≤ 4 skin influences per vertex |
| Hero prop | 2–8k tris · Standard prop: 0.5–3k · Modular environment piece: ≤ 2k |
| Textures | Props ≤ 1k, hero/characters ≤ 2k, channel-packed ORM, no 4k on the mobile path |
| Materials | ≤ ~200 base-pass instructions (mobile stats), ≤ 2 material slots per standard prop, atlas modular kits |
| Scene | ≤ ~500 draw calls and ≤ ~750k on-screen tris per view (provisional) |
| Lighting | ≤ 1 dynamic shadow-casting light (player flashlight) + a few non-shadowing points; static/baked wherever possible; renderer decision in P1.7 |

Deviations are allowed but must be stated and justified in the plan's Mobile & Performance section.

## Definition of Done — base checklist (every feature)

- [ ] Works in single-player PIE
- [ ] 2-client PIE smoke test passes: behavior replicates, no crashes/ensures (gameplay features only)
- [ ] Mobile budgets respected, or deviation documented
- [ ] Mobile Preview (Android Vulkan) render check for anything visual
- [ ] Compiles clean; no new warning/error spam in logs
- [ ] Knowledge note in `knowledge/` closed: Claude-drafted mechanics + your decisions/gotchas sections
- [ ] ROADMAP.md status updated

Plans add feature-specific checks on top. Variation challenge is recommended before the phase gate, not blocking per feature.

## Phase gate checklist (end of every phase)

- [ ] Package and install on a real Android device
- [ ] Full coop session test over EOS — at least two of: Win↔Win, Win↔Android, Android↔Android (include voice)
- [ ] **[You]** Device performance capture (stat unit / Unreal Insights) compared against budgets; update the budget table if reality disagrees. Profiling is a reps skill (P2.8) — never delegate this one
- [ ] Backlog triage: bugs and deferred items either scheduled or parked with a note
- [ ] Skim the phase's knowledge notes — fill gaps while memory is fresh

## Knowledge note template

```markdown
---
id: KN-<slug>
feature: <P1.3>
date: <YYYY-MM-DD>
---
# <Topic>
## What was built                           <!-- [Claude] drafts from the code -->
## How it works (key classes & flow)        <!-- [Claude] drafts from the code -->
## Decisions & why                          <!-- [You] -->
## Gotchas & fixes                          <!-- [You] -->
## Would do differently                     <!-- [You] -->
## Links (code paths, docs, related notes)  <!-- [Claude] drafts, you prune -->
## Quiz results (optional)                  <!-- appended by quiz mode -->

```
