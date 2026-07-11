# Roadmap

Status legend: ⬜ not started · 📋 plan generated · 🔨 in progress · ✅ done · 🅿️ parked · ⭐ core-priority
Estimates are in **sessions** (~2–4h, Claude-assisted pace). Details per feature live in [phases/](phases/).

## Sequencing & pace

- **The plan gates production — not a calendar date.** Production development starts when this practice plan says you're ready. Every production game will be coop + episodic DLC in one shared ecosystem (progression/collectibles cross titles), which is why the EOS plugin (P0.2), account-level saves (P0.6/P2.7), and DLC delivery (P5.5) are load-bearing beyond any single game.
- **Spine: P0 → P1, in dependency order.** P0+P1 ≈ 38–47 sessions; pace by sessions/week, not dates. If a phase drags, finish its ⭐ items first and park the rest.
- **Interleave P2 with P1 (standing recommendation, not time-permitting):** P2.1 animation fundamentals before P1.3/P1.6 (its upgrade pass comes after) · P2.2 materials/Niagara alongside P1.3/P1.7 · P2.3 audio before P1.8/P1.10 · minigames (P2.5/P2.6) as breather sessions anytime after prereqs.
- **P1.12 is the production gate.** Exit: the coop slice is fun for 15 minutes with a friend, on-device, cross-platform. Then decide — green-light the first production game (GTFO-like or another), or keep deepening with P2/P3+.
- **P3–P5 are the production menu.** No fixed order: whichever game gets green-lit pulls its phase forward (Snowrunner-like → P4.1–P4.3 jump the queue; survival → P3.3/P3.4). Generate plans on demand.

## P0 — Foundations & Pipeline — [phases/P0-foundations.md](phases/P0-foundations.md)

- ✅ ⭐ **P0.1** Clean UE 5.8 C++ project, git + LFS, project conventions (1)
- ⬜ ⭐ **P0.2** Port EOS test into a reusable plugin (sessions, login, voice) (2–3)
- ⬜ ⭐ **P0.3** Enhanced Input abstraction: KB/M + gamepad + touch, input contexts (2)
- ⬜ ⭐ **P0.4** First-person character + replication fundamentals (2–3)
- ⬜ ⭐ **P0.5** Android pipeline & performance baseline → validate budget table (2)
- ⬜ ⭐ **P0.6** Save architecture + EOS cloud saves round-trip (2)
- ⬜ ⭐ **P0.7** Asset pipeline shakedown: Meshy → Blender → UE (2)

## P1 — GTFO-like Vertical Slice (first slice & production gate) — [phases/P1-gtfo-slice.md](phases/P1-gtfo-slice.md)

- ⬜ ⭐ **P1.1** Interaction framework: doors, lockers, pickups (2)
- ⬜ **P1.2** Terminal system: camera-focus in-world screen + command UI (2–3)
- ⬜ ⭐ **P1.3** Weapons core: hitscan + projectile, ADS, ammo, replicated fire (3–4)
- ⬜ **P1.4** Melee weapon: charged swing, sneak-hit bonus (1–2)
- ⬜ ⭐ **P1.5** Health, downed state, revive, shared resources (2)
- ⬜ ⭐ **P1.6** Enemy AI: sleepers, perception, wake propagation, horde waves (4–5)
- ⬜ ⭐ **P1.7** Darkness & flashlight: mobile lighting strategy + renderer decision (2)
- ⬜ **P1.8** Objectives & alarms: bioscans, alarm-door defense, extraction (2–3)
- ⬜ ⭐ **P1.9** HUD & team feedback: ammo/health, hitmarkers, ping system (2)
- ⬜ **P1.10** Greybox rundown map: modular kit, zones, lighting, navmesh (2–3)
- ⬜ **P1.11** Session & lobby flow hardening: menus, friend/random join, leave/rejoin (2)
- ⬜ ⭐ **P1.12** Phase gate: Android package, cross-platform coop playtest, perf pass (1–2)

## P2 — Engine Breadth (interleave with P1; continues after) — [phases/P2-engine-breadth.md](phases/P2-engine-breadth.md)

- ⬜ **P2.1** Animation foundations: AnimBP, blendspaces, montages, retargeting (3)
- ⬜ **P2.2** Materials & Niagara basics within mobile budgets (2–3)
- ⬜ **P2.3** Audio foundations: MetaSounds, spatialization, surface-based footsteps (2)
- ⬜ **P2.4** UMG depth + minigame framework (in-world screens via P1.2) (2–3)
- ⬜ **P2.5** Minigame: Cookie Clicker — idle economy, offline progress (2)
- ⬜ **P2.6** Minigame: Flappy Bird — touch feel tuning (1)
- ⬜ ⭐ **P2.7** Save architecture depth: versioning, migration, cloud conflicts, cross-game/account-level schema (2)
- ⬜ **P2.8** Profiling & optimization practice: Insights, stat toolkit, find-the-planted-bottleneck reps (1–2)

## P3 — Genre Systems Library (production menu) — [phases/P3-genre-systems.md](phases/P3-genre-systems.md)

- ⬜ **P3.1** Souls melee core: lock-on, stamina, dodge, combos (4–5)
- ⬜ **P3.2** Traversal & platforming (PoP): ledges, wall moves, touch variant (3)
- ⬜ **P3.3** Inventory & equipment: items as data, replicated pickup/drop (3)
- ⬜ **P3.4** Survival loop: needs, harvesting, crafting, day/night (3–4)
- ⬜ **P3.5** Meta-progression: run upgrades, persistent unlocks (2)
- ⬜ **P3.6** Minigame: Chess + minimax AI (2–3)
- ⬜ **P3.7** Minigame: Word puzzle (1–2)
- ⬜ **P3.8** Minigame: HayDay-lite production timers (2)

## P4 — World, Vehicles & Flight (production menu) — [phases/P4-world-vehicles.md](phases/P4-world-vehicles.md)

- ⬜ **P4.1** Landscape & environment with landmark-driven layout (3)
- ⬜ **P4.2** World Partition & streaming for large maps (2–3)
- ⬜ **P4.3** Offroad vehicle (Snowrunner-lite): Chaos Vehicles, traction, winch, MP driving (4–5)
- ⬜ **P4.4** Flight & control-mode abstraction (SC-lite): simple/complex/assisted (3–4)
- ⬜ **P4.5** Procedural spawning: endless-runner segments, encounter director (3)

## P5 — Specialty & Scale (production menu) — [phases/P5-specialty.md](phases/P5-specialty.md)

- ⬜ **P5.1** Mass entities (Vampire Survivors at scale): pooling, instancing, mobile ceiling (3–4)
- ⬜ **P5.2** RTS slice (AoE): selection, pathfinding, fog of war (4–5)
- ⬜ **P5.3** Networked physics coop (Chained Together) — research-heavy (3–4)
- ⬜ **P5.4** Server architecture: dedicated servers, host-leaves problem, EAC, Epic app review for public matchmaking (2–3)
- ⬜ **P5.5** DLC & live-content pipeline: chunks/paks, patching, Play Asset Delivery, EOS entitlements, cross-game content (2–3)
