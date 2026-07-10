# P4 — World, Vehicles & Flight (production menu)

Goal: the large-world and vehicle skills behind the Snowrunner- and Star Citizen-flavored episodes. No dates — if a Snowrunner-like game is green-lit first, P4.1–P4.3 jump the queue. Entries are brief; the generator expands them on demand.

## P4.1 — Landscape & environment
Landscape sculpting/painting, foliage, mobile-viable environment lighting; layout driven by **recognizable landmarks** (the Star Citizen note: players navigate and describe places by memorable patterns — design for "meet me at the split rock"). **Maps to**: Snowrunner, Star Citizen. **Prereqs**: P0.5. **Est**: 3.

## P4.2 — World Partition & streaming
Large-map setup: World Partition, HLODs, data layers, streaming behavior in multiplayer (⚠️ verify 5.8 specifics), loading flow. What map size actually works on Android. **Prereqs**: P4.1. **Est**: 2–3.

## P4.3 — Offroad vehicle (Snowrunner-lite)
Chaos Vehicles: torque/differential/tire friction setup for *feel*, mud/traction zones, winch (physics constraint), cargo attachment, fuel; multiplayer driving and its physics-replication pain points (the honest preview of P5.3). **Maps to**: Snowrunner. **Prereqs**: P4.1. **Est**: 4–5.

## P4.4 — Flight & control-mode abstraction
Simple atmospheric/6DOF flight pawn with the Star Citizen control philosophy as the actual lesson: one flight model, three control layers (simple/arcade, complex/manual, assisted/automated) switchable at runtime — an input+autopilot abstraction exercise on top of P0.3. **Maps to**: Star Citizen. **Prereqs**: P0.3. **Est**: 3–4.

## P4.5 — Procedural spawning
Two flavors of procedural content: endless-runner track segments spawning ahead of the player (Subway Surfers — pooling, deterministic seeds), and a smarter encounter director (spawn pacing by tension curve — GTFO/VS deepening of P1.6). **Maps to**: Subway Surfers, Vampire Survivors. **Prereqs**: P1.6. **Est**: 3.
