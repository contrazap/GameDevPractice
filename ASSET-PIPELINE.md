# Asset Pipeline

How assets get into the game: acquisition decision tree, Meshy AI workflow, Blender cleanup, UE import conventions. Feature plans reference this file in their Assets section. Refined continuously — P0.7 is the shakedown run; update this file with what you learn there.

## Decision tree (per asset)

1. **Engine/placeholder first.** Greybox with engine primitives, Manny/Quinn, and engine content while the feature is being built. Real assets come after the mechanic works.
2. **Free/purchased pack exists?** Use it (see Sources below). Fastest path to game-ready topology, LODs, and sane materials — and studying a well-made pack is itself learning.
3. **Meshy generation** — best for props, hero objects, and creatures where packs don't fit the game's look. Expect cleanup (below).
4. **Hand-model in Blender** — simple hard-surface items, modular kit pieces, and anything where cleanup would take longer than modeling (you're intermediate; simple props are often faster by hand).

## Sources (free / already owned)

- **Fab** — free monthly assets (claim routinely), free section, previously claimed Quixel Megascans.
- **Mixamo** — free humanoid animations (retarget to UE5 skeleton).
- **Kenney.nl, PolyHaven, ambientCG** — CC0 props, textures, HDRIs.
- **Sketchfab** — filter CC0/CC-BY (check license per asset).
- Epic's free sample projects (Lyra, City Sample, etc.) — assets usable in UE projects; also reference implementations.

## Meshy workflow

Prompting (text→3D):
- One object per generation. Name the object class + style + material hints: `"industrial metal storage crate, sci-fi horror, worn painted steel, game asset, PBR"`.
- For a consistent art direction across assets, keep a reusable style suffix (define it once the game's look settles) and reuse it in every prompt.
- Characters/creatures: generate in **neutral A/T-pose**, symmetrical, limbs separated from body — anything else fights rigging.
- Image→3D gives more control: generate/pick a concept image first, then convert. Prefer it for hero assets.
- Use Meshy's target-polycount / quad-topology export options where available, but treat the result as a **high-poly source**, not a final asset.

Export: FBX or GLB with PBR maps. Always inspect in Blender before UE.

## Blender cleanup checklist (Meshy or any external asset)

1. **Scale & orientation**: real-world size, apply all transforms; UE is Z-up, centimeters — validate via a known-size reference cube. Export FBX with standard UE-compatible settings (document the exact preset here after P0.7).
2. **Retopology** (only if the mesh will deform, is over budget, or has broken shading):
   - Blender built-in **QuadriFlow remesh** — free, decent for organic shapes.
   - **Instant Meshes** — free standalone, quick auto-retopo.
   - **Quad Remesher** (Exoside) — paid addon, best-in-class auto quads.
   - `<add the free retopo tool you recently found here after evaluating it>`
   - Simple **Decimate** is fine for static props that just need fewer tris.
3. **UVs**: re-unwrap after retopo (Smart UV for props, seam-based for hero assets). One UV set + lightmap UVs generated in UE if needed.
4. **Bake** high→low in Blender: normal + AO (+ curvature if useful). This is how Meshy detail survives retopo.
5. **Materials**: consolidate to 1–2 slots per asset; channel-pack ORM (Occlusion/Roughness/Metallic); atlas modular kit pieces (draw calls are a mobile budget line).
6. **Budgets**: hit the CONVENTIONS.md table *before* export, not in UE.
7. **Collision**: simple props — UE auto-convex; anything precise — `UCX_<meshname>` collision meshes in the FBX.
8. **Naming**: UE style — `SM_` static mesh, `SK_` skeletal, `T_` texture (`_D/_N/_ORM` suffixes), `M_`/`MI_` materials.
9. **LODs**: UE auto-LOD is fine for props; hand-check LOD1/2 on hero assets and enemies.

## Characters & animation

- **Humanoids**: rig to the UE5 skeleton (Manny/Quinn) so every Epic/Fab/Mixamo animation retargets cleanly (UE IK Retargeter). Rigging routes: Blender addon (e.g. Game Rig Tools — free; Auto-Rig Pro — paid) or Mixamo auto-rig → retarget in UE.
- **Creatures/enemies** (the GTFO-like game's big lift): custom lean skeleton (mobile budget: ≲75 bones ⚠️ verify 5.8, ≤4 influences/vertex). Minimum viable anim set: idle/sleep, walk/run, attack, hit-react, death. Sources: Meshy auto-rig+anim (evaluate quality per asset — expect to reject some), hand-keying in Blender (you can), or retargeted humanoid anims for humanoid-ish creatures.
- Evaluate **Meshy auto-rigging** honestly during P0.7/P1.6: if weights are consistently bad, the pipeline is Meshy mesh → own rig in Blender → own/retargeted anims.

## What a feature plan's Assets section must specify

Per asset: name/purpose → acquisition route (tree above) → budget row it falls under → required anim set (if skeletal) → any Meshy prompt suggestion → cleanup steps that apply. Plus a placeholder strategy so the feature is never blocked on assets.
