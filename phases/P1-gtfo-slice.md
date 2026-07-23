# P1 — GTFO-like Vertical Slice (first slice & production gate)

Goal: a small playable coop level with the GTFO core loop — sneak through darkness among sleeping enemies, manage scarce resources, trigger objectives that wake the nightmare, survive the wave, extract. This phase is your **first complete vertical slice**: the generic coop-action toolkit (interaction, weapons, health/revive, AI, HUD, sessions) every production candidate needs, and the proof you can ship a whole loop. If the GTFO-like game is green-lit at the gate, this code and its knowledge notes carry straight into production. Build order below is dependency order; ⭐ items form the minimum slice.

**Interleave P2 fundamentals (standing recommendation):** P2.1 animation fundamentals before P1.3/P1.6 (its upgrade pass comes after) · P2.2 materials/Niagara alongside P1.3/P1.7 · P2.3 audio before P1.8/P1.10. Pull each in when the P1 feature would otherwise lean on concepts you haven't practiced.

## P1.1 — Interaction framework ⭐

Trace-based interaction from the FP camera: interactable interface, on-screen prompt, instant and hold-to-interact, all replicated (server validates). First consumers: doors (openable), lockers/boxes (containers), floor pickups. Every later system (terminals, objectives, revives) plugs into this.
**Skills**: interfaces, replicated interactions, UI prompts. **Prereqs**: P0.4. **Est**: 2. **Networked**: yes. **Assets**: greybox meshes.

## P1.2 — Terminal system

GTFO's signature interaction: approach an in-world screen, camera focuses onto it (smooth blend), input context switches to UI, a command-line terminal responds (LIST/QUERY/PING-style commands against item/zone data). Design rationale: terminals create calm, heads-down tension and diegetic information flow. Doubles as the **minigame gateway** — P2.4 reuses this focus mechanic for minigame boards.
**Skills**: camera blending, input context switching, UMG/widget interaction on world objects, small command parser. **Prereqs**: P1.1, P0.3. **Est**: 2–3. **Networked**: partially (focus state visible to others). **Assets**: terminal prop (Meshy candidate).

## P1.3 — Weapons core ⭐

Hitscan and projectile weapons on a shared weapon architecture: equip/holster, fire/reload, ADS, spread/recoil, ammo pools, damage application; server-authoritative fire with client feedback (muzzle flash, tracers, impacts — placeholder VFX). The replication design here (RPC flow, effects for other clients) is the most transferable single lesson of the phase.
**Skills**: weapon architecture, replicated combat, traces/projectiles, anim hooks. **Prereqs**: P0.4. **Est**: 3–4. **Networked**: yes — the deep dive. **Assets**: 1–2 weapons (free pack or Meshy), placeholder VFX.

## P1.4 — Melee weapon

Charged-swing melee (GTFO hammer): hold to charge, release to swing, hit detection during swing arc, big damage multiplier on sleeping enemies (this is what makes stealth a strategy, not a mood). Reuses the weapon slot/equip architecture from P1.3.
**Skills**: melee traces, charge mechanics, anim montage timing. **Prereqs**: P1.3. **Est**: 1–2. **Networked**: yes. **Assets**: melee weapon prop.

## P1.5 — Health, downs, revive, shared resources ⭐

Health component, damage from P1.3/P1.6, downed state (not dead — crawl, wait for help), teammate revive via hold-interact (P1.1), team wipe → fail state. Consumable resources: medkit and ammo pack, usable on self or teammates. Coop's emotional core is here: scarcity + interdependence.
**Skills**: health/damage architecture, replicated states, team mechanics. **Prereqs**: P1.1, P1.3. **Est**: 2. **Networked**: yes. **Assets**: none beyond icons.

## P1.6 — Enemy AI ⭐

The phase's hardest feature. Sleeper enemies: dormant with periodic pulse-glow, wake on noise/light/proximity (perception tuning is the stealth game), wake propagation to nearby sleepers, chase + melee attack. Plus a wave director v1: spawn points, paced waves for alarm events (P1.8). Budget-aware: enemy count ceiling per the mobile scene budget — verify on-device in P1.12.
**Skills**: Behavior Trees/State Trees, AI Perception, navmesh, spawning architecture, MP-relevant AI (server-run brains, replicated anim/state). **Prereqs**: P1.3, P1.5. **Est**: 4–5 (split candidate: P1.6a sleepers/stealth, P1.6b horde director). **Networked**: yes. **Assets**: 1 creature (Meshy + rig evaluation per ASSET-PIPELINE) with sleep/walk/run/attack/death set; engine mannequin as fallback.

## P1.7 — Darkness & flashlight ⭐

GTFO's darkness on a mobile budget — a deliberately constraint-driven feature. Decide the renderer strategy (mobile forward vs deferred, verified against 5.8 from P0.5 findings), then build: oppressive-dark lighting approach, player flashlight (the one dynamic shadow-casting light), enemy glow pulses, emergency lighting states (alarm red). Light is also gameplay: it wakes sleepers (feeds P1.6 perception).
**Skills**: mobile rendering constraints, lighting design, light-as-gameplay. **Prereqs**: P0.5, ideally alongside P1.6. **Est**: 2. **Networked**: flashlight state replicates. **Assets**: none.

## P1.8 — Objectives & alarms

The loop closer: objective flow (reach zone → do thing → extract), bioscan team-standing circles (all players in the circle, progress bar — forces grouping right when it's most dangerous), alarmed doors that trigger wave defense (P1.6 director), extraction scan ends the level. Win/fail flow with restart.
**Skills**: game flow/state machines (GameMode/GameState), replicated objective state, encounter design. **Prereqs**: P1.5, P1.6. **Est**: 2–3. **Networked**: yes. **Assets**: scan VFX placeholder.

## P1.9 — HUD & team feedback ⭐

Minimal readable HUD: health/downed indicators for self and team, ammo, crosshair + hitmarkers, interaction prompts (P1.1 surface), middle-mouse/button ping system placing a world marker teammates see (crossplay communication when voice is off). Touch layout variant for Android.
**Skills**: UMG HUD architecture, replicated pings, mobile UI layout. **Prereqs**: P1.3, P1.5. **Est**: 2. **Networked**: pings. **Assets**: icons.

## P1.10 — Greybox rundown map

A small authored level using a modular greybox kit: 4–6 zones gated by doors, resource placement (lockers), terminal placement, sleeper placement, one alarm event, extraction point. Pacing pass (quiet → tense → loud → quiet), lighting pass (P1.7), navmesh validation. Level design *craft*, not tech.
**Skills**: blockout workflow, pacing, readability in darkness. **Prereqs**: P1.1–P1.8 mostly done. **Est**: 2–3. **Networked**: n/a. **Assets**: greybox kit (hand-model — simple modular pieces).

## P1.11 — Session & lobby flow hardening

From EOS-test plumbing to a game flow: main menu → host (friend-joinable) or quick-join (random session, PUBG-style) → simple lobby/loadout → travel into level → return to menu; handle mid-game leave gracefully; define the join-in-progress policy (allow into lobby only, or into active run?); voice UX polish (talking indicator). **Map/mode selection lives here** — the caller-owned listen-map URL from P0.2 gets its real caller — and with it the one-app session-discovery question: partition search per title/mode (a search attribute or per-title bucket) so a host in one title's map never surfaces in another's join list (evolves `OnlineCore`'s single `BucketId`; the P5.6 hub later consumes this same selection flow). Document the known limitation: listen-server host leaving ends the session — host migration vs dedicated servers is a P5.4 topic and a real-game decision.
**Skills**: map travel with sessions, game flow, matchmaking UX. **Prereqs**: P0.2, playable P1 content. **Est**: 2–3. **Networked**: yes. **Assets**: menu UI only.

## P1.12 — Phase gate ⭐

Per CONVENTIONS.md: Android device package, full cross-platform coop playthrough of P1.10's map (Win↔Android with voice), device perf capture vs budgets (especially P1.6 enemy counts + P1.7 lighting), triage, knowledge note review. Exit criteria: the slice is fun for 15 minutes with a friend — this is the **production gate**: decide here whether to green-light starting the production build (the real unified app, built fresh from these learnings, opening with the title this slice rehearsed — or another) or keep deepening with P2/P3+ first.
**Prereqs**: everything above (⭐ minimum: P1.1, P1.3, P1.5, P1.6, P1.7, P1.9). **Est**: 1–2.
