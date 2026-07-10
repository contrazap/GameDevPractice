# P5 — Specialty & Scale (production menu)

Goal: the hard, research-flavored problems plus the ecosystem's shipping tech. Each is optional until a green-lit game or the ecosystem needs it. Entries are brief; the generator expands them on demand.

## P5.1 — Mass entities
Vampire Survivors density in 3D: object pooling, instanced rendering (ISM/HISM), vertex-anim or lightweight-skeletal crowds, aggressive LODs, evaluate Mass/large-crowd frameworks (⚠️ verify 5.8 state); find the actual mobile ceiling on-device. **Maps to**: Vampire Survivors at scale. **Prereqs**: P1.6. **Est**: 3–4.

## P5.2 — RTS slice
Age-of-Empires vertical slice: marquee selection, command queue, group movement/formations, pathfinding at unit-count (NavMesh limits vs flow fields), fog of war, minimal gather-build economy. Top-down camera + input mode on P0.3. **Maps to**: AoE. **Prereqs**: P2.4. **Est**: 4–5.

## P5.3 — Networked physics coop
Chained-Together-style physics coupling between player characters (rope/chain constraint) — genuinely hard: physics replication strategies, authority over shared constraints, prediction limits, when to fake it. Research + prototype, expect iteration. **Maps to**: Chained Together. **Prereqs**: P0.4, ideally P4.3 experience. **Est**: 3–4.

## P5.4 — Server architecture & going public
The decisions the real game must eventually make, prototyped: dedicated server build (can EOS sessions front it? ⚠️ verify), the listen-server host-leaves problem and what host migration would actually cost, Easy Anti-Cheat overview, and the Epic application-review path required before random-player matchmaking goes public (external dependency — start paperwork early when the real game approaches release). **Prereqs**: P1.11. **Est**: 2–3.

## P5.5 — DLC & live-content pipeline
The ecosystem model's shipping tech: package content as chunks/paks (IoStore), a patch/update flow per platform (desktop patching; **Play Asset Delivery** for Android post-install content), content gated by **EOS entitlements**, and a cross-game round trip proven in miniature — a collectible earned in one "game" visible in another via the account-level saves from P0.6/P2.7. Research-heavy: verify UE 5.8 chunking/IoStore + PAD tooling and store policies for episodic updates against current docs (⚠️ verify all). **Prereqs**: P0.6, P2.7. **Est**: 2–3.
