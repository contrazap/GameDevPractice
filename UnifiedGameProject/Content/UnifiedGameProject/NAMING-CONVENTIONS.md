# UnifiedGameProject — Content Naming Conventions

Short stub for asset naming inside `Content/UnifiedGameProject/`. Aligns with the
repo-level [ASSET-PIPELINE.md](../../../ASSET-PIPELINE.md) (§ Blender cleanup → Naming)
and is the naming reference for every feature that authors content here.

All game content lives under `Content/UnifiedGameProject/` (the project namespace) —
never at `Content/` root — so it never collides with engine or plugin content.

## Folder namespace

| Folder | Holds |
|---|---|
| `Characters/` | Player + enemy skeletal meshes, rigs, anims, character Blueprints |
| `Weapons/` | Weapon meshes, first-person viewmodels, weapon Blueprints |
| `Maps/` | Levels (`.umap`) and their `_BuiltData` (built data is gitignored) |
| `UI/` | UMG widgets, UI-only textures, fonts |
| `Environments/` | Modular kit pieces, props, environment meshes + materials |
| `Core/` | Shared base classes, gameplay-framework BPs, data assets, common materials/functions |
| `Titles/<Title>/` | Everything owned by one title — same type-folder layout inside (e.g. `Titles/GTFO/Weapons/`) |

## Title namespace & chunk discipline (one-app, titles-as-DLC)

The production model is one shipped app with titles delivered as DLC chunks (ROADMAP →
P5.5/P5.6). Foldering enforces chunkability from the first asset — retrofitting chunk boundaries
onto tangled content is the expensive path:

- **Base/shared content** lives in the type folders above (hub/test maps, session UI, core BPs,
  shared materials) → ships in the base install (chunk 0). `L_Sandbox` and the session-menu UI
  are base content.
- **Title content** lives under `Titles/<Title>/` with the same type-folder layout inside → becomes
  that title's DLC chunk(s) at P5.5. The first title folder (`Titles/GTFO/`) appears with P1.1's
  first asset.
- **Reference rules:** a title may hard-reference base content; base content **never** references
  title content; **titles never hard-reference each other**. Crossover content gets its own
  `Titles/<Crossover>/` folder and reaches foreign titles via soft references
  (`TSoftObjectPtr` / soft class refs) only, declaring chunk dependencies at P5.5.

## Asset prefixes

| Prefix | Asset type |
|---|---|
| `BP_` | Blueprint class |
| `WBP_` | Widget Blueprint (UMG) |
| `SM_` | Static Mesh |
| `SK_` | Skeletal Mesh |
| `A_` | Animation (AnimSequence / AnimMontage) |
| `T_` | Texture |
| `M_` | Material |
| `MI_` | Material Instance |
| `DA_` | Data Asset |
| `L_` | Level / map (`.umap`) |

## Texture suffixes (channel-packed, mobile path)

| Suffix | Map |
|---|---|
| `_D` | Base color / diffuse |
| `_N` | Normal |
| `_ORM` | Packed Occlusion (R) · Roughness (G) · Metallic (B) |

## Rules

- **PascalCase** after the prefix: `SM_StorageCrate`, `WBP_MainMenu`, `L_Sandbox`.
- No spaces, no double underscores except before a texture suffix: `T_Crate_ORM`.
- Variants get a numeric suffix: `SM_Crate_01`, `SM_Crate_02`.
- Collision meshes for precise props: `UCX_<meshname>` inside the FBX (see ASSET-PIPELINE §7).
- Budgets (tris/texture size/material slots) are enforced per CONVENTIONS.md **before** import.
