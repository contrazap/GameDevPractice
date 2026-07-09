# P0 — Foundations & Pipeline

Goal: a clean practice project where multiplayer, Android, saves, and the asset pipeline all demonstrably work end-to-end *before* gameplay features pile on. Everything later stands on this. Target: ~2 weeks.

## P0.1 — Clean project & source control ⭐

Fresh UE 5.8 C++ project (Blank or Third Person template as a base to strip). Set up git with LFS (`.gitignore`/`.gitattributes` for UE), folder structure and naming conventions (Content and Source layout), editor project settings baseline (target platforms Windows + Android from day one). Decide module layout: game module + room for the EOS plugin from P0.2. This project is the single umbrella practice game everything else lives in.
**Skills**: project anatomy, build targets, source control for binary-heavy projects. **Prereqs**: none. **Est**: 1. **Networked**: n/a. **Assets**: none.

## P0.2 — EOS plugin port ⭐

Extract the working multiplayer from the EOS test project (Epic login, host/join/leave session, voice chat mute/unmute) into a clean, reusable plugin or module in the new project — proper separation this time: no gameplay code in the plugin, a small clean API surface for the game to call. Re-verify Windows↔Android session + voice at the end. This plugin is intended to ship in the real September game.
**Skills**: plugins/modules, code organization, EOS Online Subsystem review (second exposure = retention). **Prereqs**: P0.1. **Est**: 2–3. **Networked**: yes (sessions layer). **Assets**: none.

## P0.3 — Input abstraction ⭐

Enhanced Input end-to-end: input actions and mapping contexts for on-foot / UI / focused-screen states; KB/M, gamepad, and touch (virtual sticks + buttons, basic gestures) driving the same actions. Simple rebinding proof (swap a key at runtime). Touch is designed now so mobile is never a bolt-on — this decision is what makes the GTFO-like game's mobile version viable later.
**Skills**: Enhanced Input, input contexts, platform input differences. **Prereqs**: P0.1. **Est**: 2. **Networked**: no. **Assets**: none.

## P0.4 — First-person character + replication fundamentals ⭐

The pawn for the GTFO-like slice: first-person camera and arms (placeholder), walk/sprint/crouch, interaction trace stub. Alongside it, the replication fundamentals primer *applied*: NetMode, authority/roles, replicated properties, RPCs, how CharacterMovement replication works, and the 2-client PIE workflow that every later feature's smoke test uses. This is deliberately the place to internalize the server-authoritative mental model.
**Skills**: Character/CharacterMovement, FP camera setup, core replication model, PIE multi-client testing. **Prereqs**: P0.3. **Est**: 2–3. **Networked**: yes. **Assets**: placeholder FP arms (engine/free).

## P0.5 — Android pipeline & performance baseline ⭐

Package and deploy to a real Android device from this project; set up device profiles and scalability so desktop scales up from mobile-authored content; learn the Mobile Preview (Android Vulkan) editor mode; capture a first performance baseline (stat unit, Unreal Insights) on a test scene at budget-level content. **Output: validate or correct the provisional budget table in CONVENTIONS.md.** Also record the renderer capabilities that matter for P1.7 (mobile forward vs mobile deferred, dynamic light costs — verify against 5.8, not memory).
**Skills**: Android packaging, device profiles, scalability, profiling. **Prereqs**: P0.1. **Est**: 2. **Networked**: no. **Assets**: none (test scene from engine content).

## P0.6 — Save architecture + cloud saves ⭐

Local SaveGame architecture (settings + player progression stub, save slots) and then EOS Player Data Storage sync on top: save on Windows, continue on Android and back — the crossplay-progression pillar proven in miniature. Design for the future: a version field and a plan for schema migration (deepened later in P2.7).
**Skills**: SaveGame system, EOS Player Data Storage, cross-device state. **Prereqs**: P0.2. **Est**: 2. **Networked**: cloud, not gameplay. **Assets**: none.

## P0.7 — Asset pipeline shakedown ⭐

One prop through the full route: Meshy generation → Blender cleanup per ASSET-PIPELINE.md (retopo decision, UVs, bake, ORM packing, collision, naming) → UE import with LODs → placed in the test scene within budget. Also import one free pack (Fab) and study its topology/materials as a quality reference. **Output: update ASSET-PIPELINE.md with the exact export preset and any workflow corrections; evaluate Meshy quality honestly.**
**Skills**: the entire asset pipeline, budget discipline. **Prereqs**: P0.1, P0.5 (budgets). **Est**: 2. **Networked**: no. **Assets**: this *is* the asset feature.
