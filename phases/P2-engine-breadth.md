# P2 — Engine Breadth

Goal: solid foundation on the commonplace UE systems every game needs — animation, materials/VFX, audio, UI, saves, profiling. Interleave with P1 (the roadmap's Sequencing section names the recommended pull-in points) or run after the production gate alongside real development. Entries are brief; the plan generator expands them on demand (see PLAN-GENERATOR.md rule 2).

## P2.1 — Animation foundations
AnimBP for the FP arms and enemies: blendspaces (locomotion), montages + notifies (fire/reload/melee/revive), state machines, basic IK, and the retargeting workflow (Manny ↔ Mixamo ↔ Meshy rigs) end-to-end. Directly upgrades P1's placeholder animation. **Interleave**: run the fundamentals session (AnimBP/blendspace/montage basics on the mannequin) *before* P1.3/P1.6; the upgrade + retargeting pass after P1.3. **Prereqs**: P0.4 (fundamentals session), P1.3 (upgrade pass). **Est**: 3.

## P2.2 — Materials & Niagara basics
PBR material fundamentals, master materials + instances, parameter-driven variation; Niagara starter set (muzzle flash, impact sparks, blood puff, dust) authored within mobile budgets and swapped into P1.3/P1.6 placeholders. **Interleave**: alongside P1.3/P1.7. **Prereqs**: P1.3. **Est**: 2–3.

## P2.3 — Audio foundations
MetaSounds basics, attenuation/spatialization (darkness makes audio load-bearing in a GTFO-like), sound classes + mix, surface-based footsteps, gun/creature sounds. Free SFX sources. **Interleave**: before P1.8/P1.10 (alarms and level pacing want real audio). **Prereqs**: P1 core. **Est**: 2.

## P2.4 — UMG depth + minigame framework
Widget architecture done properly (reusable components, view/data separation, gamepad+touch navigation — evaluate CommonUI ⚠️ verify 5.8 fit). The minigame gateway: reuse P1.2's camera-focus mechanic to run any minigame on an in-world screen, with a clean "minigame slot" API (start/stop, input capture, save hook). **Prereqs**: P1.2. **Est**: 2–3.

## P2.5 — Minigame: Cookie Clicker
Idle economy on a terminal screen: click income, generators, exponential cost curves, big-number formatting, offline progress via timestamps (P0.6 saves). Teaches economy math reused by P3.5/P3.8. **Prereqs**: P2.4. **Est**: 2.

## P2.6 — Minigame: Flappy Bird
Smallest-scope game: one-input physics feel, procedural obstacles, score. Touch-first; tune until it *feels* right — feel-tuning is the skill. **Prereqs**: P2.4. **Est**: 1.

## P2.7 — Save architecture depth ⭐
Versioning + migration of save schemas (add a field without breaking old saves), per-player vs per-world state split, cloud conflict strategy (two devices, divergent saves — last-write-wins vs merge), autosave cadence. Hardens P0.6 into what production ships with. **Core under the ecosystem model**: progression and collectibles cross games/DLCs, so schemas must be account-level, versioned, and migration-ready — this system outlives any single title (pairs with P5.5 entitlements). **Prereqs**: P0.6. **Est**: 2.

## P2.8 — Profiling & optimization practice
Deliberate reps on the tools that keep every game on budget: `stat unit` / `stat gpu` / `stat scenerendering`, Unreal Insights traces (CPU/GPU frames, hitches), memory reports, Android-side capture. Format: **find the planted bottleneck** — Claude sabotages a copy of the test scene (unbatched draws, oversized textures, one shadow-casting light too many, blocking loads…), you diagnose and fix it with the tools; re-runnable with fresh sabotage as variation challenges. Builds on P0.5's baseline and feeds every phase gate's perf capture (always a [You] task). **Prereqs**: P0.5. **Est**: 1–2.
