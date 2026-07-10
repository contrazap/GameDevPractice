# P3 — Genre Systems Library (production menu)

Goal: the mechanics library for the episodic-DLC ecosystem — each feature is a self-contained system practiced in the umbrella project, in its own test level or bolted onto the P1 slice where it fits. No dates — a green-lit game pulls its systems forward (survival title → P3.3/P3.4 first). Entries are brief; the generator expands them on demand.

## P3.1 — Souls melee core
Lock-on targeting, stamina management, dodge with i-frames, light/heavy attack combos via montage chaining, hit reactions and a poise-lite stagger model. The animation-driven combat discipline (P2.1 prerequisite matters). Coop-compatible from the start. **Maps to**: Dark Souls 2. **Prereqs**: P2.1. **Est**: 4–5.

## P3.2 — Traversal & platforming
Ledge grab/climb, wall run/jump, timed platforming sequences; camera handling for acrobatics; a touch-friendly control variant (PoP mobile port instinct). **Maps to**: Prince of Persia WW. **Prereqs**: P0.4. **Est**: 3.

## P3.3 — Inventory & equipment
Items as DataAssets, slot/grid inventory, replicated pickup/drop/transfer, equipment slots feeding the P1.3 weapon system, container UI (lockers upgrade). Design the item schema with cross-game travel in mind — collectibles that move between games/DLCs are an ecosystem pillar (P2.7/P5.5). **Maps to**: PUBG, Zomboid. **Prereqs**: P1.1, P2.4. **Est**: 3.

## P3.4 — Survival loop
Needs (hunger/temperature), harvestable resources, crafting recipes, day/night cycle driving danger; coop-shared world state. **Maps to**: Zomboid, Don't Starve Together. **Prereqs**: P3.3. **Est**: 3–4.

## P3.5 — Meta-progression
Run-based upgrade choices (Vampire Survivors-style level-up picks) + persistent unlock tree between runs, using P2.5's economy math and P2.7's saves. **Maps to**: Vampire Survivors. **Prereqs**: P2.5, P2.7. **Est**: 2.

## P3.6 — Minigame: Chess
Full rules engine (castling, en passant, promotion), minimax + alpha-beta AI with difficulty levels, board UI on a terminal screen. Optional stretch: async turn sync between coop players. **Prereqs**: P2.4. **Est**: 2–3.

## P3.7 — Minigame: Word puzzle
Words-of-Wonders-style letter wheel: word-list data pipeline, drag-to-spell touch UX, hint economy. **Prereqs**: P2.4. **Est**: 1–2.

## P3.8 — Minigame: HayDay-lite timers
Production chains with real-time timers that progress offline (timestamp math from P2.5), queue management, soft currency. **Prereqs**: P2.5. **Est**: 2.
