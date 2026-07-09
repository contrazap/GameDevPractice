# P2 — Engine Breadth

Goal: solid foundation on the commonplace UE systems every game needs — animation, materials/VFX, audio, UI, saves. Interleave with P0/P1 as breathers or run after September alongside real development. Entries are brief; the plan generator expands them on demand (see PLAN-GENERATOR.md rule 2).

## P2.1 — Animation foundations
AnimBP for the FP arms and enemies: blendspaces (locomotion), montages + notifies (fire/reload/melee/revive), state machines, basic IK, and the retargeting workflow (Manny ↔ Mixamo ↔ Meshy rigs) end-to-end. Directly upgrades P1's placeholder animation. **Prereqs**: P1.3. **Est**: 3.

## P2.2 — Materials & Niagara basics
PBR material fundamentals, master materials + instances, parameter-driven variation; Niagara starter set (muzzle flash, impact sparks, blood puff, dust) authored within mobile budgets and swapped into P1.3/P1.6 placeholders. **Prereqs**: P1.3. **Est**: 2–3.

## P2.3 — Audio foundations
MetaSounds basics, attenuation/spatialization (darkness makes audio load-bearing in a GTFO-like), sound classes + mix, surface-based footsteps, gun/creature sounds. Free SFX sources. **Prereqs**: P1 core. **Est**: 2.

## P2.4 — UMG depth + minigame framework
Widget architecture done properly (reusable components, view/data separation, gamepad+touch navigation — evaluate CommonUI ⚠️ verify 5.8 fit). The minigame gateway: reuse P1.2's camera-focus mechanic to run any minigame on an in-world screen, with a clean "minigame slot" API (start/stop, input capture, save hook). **Prereqs**: P1.2. **Est**: 2–3.

## P2.5 — Minigame: Cookie Clicker
Idle economy on a terminal screen: click income, generators, exponential cost curves, big-number formatting, offline progress via timestamps (P0.6 saves). Teaches economy math reused by P3.5/P3.8. **Prereqs**: P2.4. **Est**: 2.

## P2.6 — Minigame: Flappy Bird
Smallest-scope game: one-input physics feel, procedural obstacles, score. Touch-first; tune until it *feels* right — feel-tuning is the skill. **Prereqs**: P2.4. **Est**: 1.

## P2.7 — Save architecture depth
Versioning + migration of save schemas (add a field without breaking old saves), per-player vs per-world state split, cloud conflict strategy (two devices, divergent saves — last-write-wins vs merge), autosave cadence. Hardens P0.6 into what the real game ships with. **Prereqs**: P0.6. **Est**: 2.
