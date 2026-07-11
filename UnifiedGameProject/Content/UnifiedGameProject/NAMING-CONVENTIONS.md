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
