---
id: KN-project-source-control
feature: P0.1
date: 2026-07-11
---
# Project & Source Control (UnifiedGameProject)

Standing up the umbrella practice project — UE 5.8 Blank C++ at the repo root — with
LFS-backed source control, a scoped ignore/attributes setup, the Content namespace tree,
and a Windows + Android baseline. This is the spine every later feature builds on.

## What was built

- **Project:** `UnifiedGameProject` — UE 5.8 **Blank C++**, engine association `5.8`, created at the
  GameDevPractice repo root (no nested git repo — it joins the existing shared repo).
- **Source control:**
  - `UnifiedGameProject/.gitignore` — current UE 5.8 ignore set (engine-generated dirs, IDE/solution files
    incl. the new `.slnx`, compiled binaries, built lighting data), plus a **secrets** block that ignores the
    real platform `Config/Windows/WindowsEngine.ini` + `Config/Android/AndroidEngine.ini` (EOS keys land there in P0.2).
  - `UnifiedGameProject/.gitattributes` — **Git LFS routing** for 38 binary asset patterns
    (`*.uasset *.umap`, meshes, textures, audio, video, fonts, archives). File-locking (`lockable`) documented but
    deferred (solo repo).
  - Both files live **inside the project folder** so UE rules stay scoped to that subtree of the shared repo.
- **Content namespace:** `Content/UnifiedGameProject/{Characters,Weapons,Maps,UI,Environments,Core}/`, each with a
  `.gitkeep`, plus `Content/UnifiedGameProject/NAMING-CONVENTIONS.md` (prefixes `BP_ WBP_ SM_ SK_ A_ T_ M_ MI_ DA_ L_`,
  texture suffixes `_D/_N/_ORM`), aligned to ASSET-PIPELINE.md.
- **First real asset:** `L_Sandbox.umap` (Basic template) in `Maps/`, set as both Editor Startup + Game Default map,
  verified working in PIE.
- **Project settings baseline:** Windows + Android supported platforms; project name/description/company
  (`ContrazapGames`); single Runtime module confirmed.

## How it works (key classes & flow)

**Target vs Module (the core learning).** Targets (`*.Target.cs`) declare *what executable* gets built and which
modules it pulls in; Modules (`*.Build.cs` + code) are the compilation units with their own dependencies.

- `Source/UnifiedGameProject.Target.cs` → `UnifiedGameProjectTarget : TargetRules`, `Type = TargetType.Game` —
  the **standalone game** build (what ships). Sets `BuildSettingsVersion.V7`, `IncludeOrderVersion = Unreal5_8`,
  and adds the game module via `ExtraModuleNames`.
- `Source/UnifiedGameProjectEditor.Target.cs` → `...EditorTarget`, `Type = TargetType.Editor` — the **editor** build
  (what you launch to open the project). Same settings; also includes the game module.
- `Source/UnifiedGameProject/UnifiedGameProject.Build.cs` → `UnifiedGameProject : ModuleRules` — per-module rules:
  `PCHUsage`, and `PublicDependencyModuleNames = { Core, CoreUObject, Engine, InputCore, EnhancedInput }`.
  Note UE 5.8's Blank C++ already wires **EnhancedInput**. Commented hints exist for `Slate`/`OnlineSubsystem` — the
  online one is a reminder, **not** where EOS goes (that's a plugin, see seam below).
- `Source/UnifiedGameProject/UnifiedGameProject.cpp` → `IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl,
  UnifiedGameProject, "UnifiedGameProject")` — registers the **primary game module** with the default impl (no custom
  startup/shutdown logic).
- `UnifiedGameProject.h` → just `#include "CoreMinimal.h"` — empty shell.

So: **one project → two targets (Game, Editor) → one shared game module.**

**LFS flow (existing shared repo — the real path used):**
```bash
git lfs install                       # one-time, installs the clean/smudge filters (done once)
# drafted .gitignore + .gitattributes (LFS routing) inside UnifiedGameProject/
git lfs track                         # lists the patterns from .gitattributes to confirm they register
git add .gitattributes .gitignore Content/... && git commit && git push
# --- verify (never assume) ---
git lfs ls-files                      # lists committed LFS pointers (e.g. L_Sandbox.umap)
git show HEAD:UnifiedGameProject/Content/UnifiedGameProject/Maps/L_Sandbox.umap | head   # prints the pointer, not bytes
git lfs status                        # "objects to be pushed" empty = uploaded to remote
```
Pre-commit verification helpers used before ever committing: `git check-attr filter diff merge text -- <file>`
(confirms a file routes to LFS) and `git check-ignore -v <file>` (confirms an ignore rule and which line matched).

**From-scratch flow (for the variation challenge):** `git init` → `git lfs install` → add the two files (or
`git lfs track "*.uasset" "*.umap" ...`) → `git add . && git commit` → `git remote add origin … && git push -u origin main`.

**What LFS added over the EOS test (`MultiplayerCourse`).** That project had a solid `.gitignore` but **no
`.gitattributes` and no LFS** — every `.uasset`/`.umap` was committed as a **raw binary blob** (repo bloat, slow
clones, no dedup). Here, `.gitattributes` routes all binary asset types through LFS as lightweight pointers while the
bytes live in the LFS store. The ignore set was also modernized: added `.slnx`, `.idea/`, `*.pdb/*.ipdb/*.iobj`, and
scoped plugin `Binaries/Intermediate`.

**Android "from day one".** Set `"TargetPlatforms": ["Android","Windows"]` in the `.uproject` — Android is available
because the engine's Android platform files are installed. This **declares** the target only. Still deferred to **P0.5**:
the SDK/NDK/JDK toolchain (`SetupAndroid.bat`), Android project settings (package name, min/target SDK, signing), and
actual packaging/device deployment. **Target Hardware was deliberately left `Desktop / Maximum`** — flipping it to
Mobile/Scalable changes a batch of renderer defaults, and the renderer decision is formally deferred to **P1.7**.

**Module-vs-plugin seam (for P0.2).** The project is a **single primary Runtime module**. EOS will be a **separate
plugin** in P0.2 — not welded into the game module (the mistake the EOS test made). The clean seam is exactly this
single-module layout: a plugin drops in via the `.uproject` `Plugins` array + its own `Source/`, with **no gameplay
code in the plugin**. The commented `OnlineSubsystem` hint in `Build.cs` is not where EOS belongs.

## Decisions & why

- **Blank C++ over a template** — keeps P0.4's first-person-character build as real learning; avoids template cruft
  (the EOS test dragged in `Variant_*` folders).
- **Scoped ignore/attributes inside the project folder** — the GameDevPractice repo already exists and holds planning
  docs too; nesting the UE rules under `UnifiedGameProject/` keeps them applying only to the project subtree, with **no
  nested git repo**.
- **Secrets = ignore real `*Engine.ini`, commit `*.template`** — real EOS keys never touch git; the pattern is in place
  now and activates in P0.2 when those files actually exist.
- **File locking deferred** — solo repo; `lockable` is documented in `.gitattributes` but off (plan cut line 1).
- **`.obj` intentionally NOT routed to LFS** — it collides with C++ compiled object files (already gitignored).
- **`L_Sandbox` from the Basic template** — a single self-contained `.umap` (cleanest LFS pointer proof) and
  lightweight, vs Open World's World-Partition One-File-Per-Actor sprawl.

## Gotchas & fixes

- **`.gitignore`/`.gitattributes` don't support trailing inline comments.** `#` only starts a comment at the *start* of
  a line, so `*.slnx   # note` would make the whole string (spaces included) the pattern. Fix: comments on their own line.
- **`git lfs ls-files` was empty after the first commit — and that was correct.** A Blank C++ project ships an **empty
  `Content/`**, so there were no assets to catch. Proven by staging a throwaway `.png` that immediately showed as an LFS
  object; real pointers appeared only once `L_Sandbox.umap` existed. Lesson: LFS can't list what doesn't exist yet.
- **Supported Platforms writes to the `.uproject` (`TargetPlatforms`), not `Config/*.ini`.** So `git diff Config/` was
  empty after setting platforms. For reference: project name/description/company → `DefaultGame.ini`; default/startup
  maps → `DefaultEngine.ini`.
- **`git lfs status` false alarm.** It listed the 0-byte `DefaultEditor.ini` (empty-blob SHA `e3b0c442…`) under
  "objects to be pushed," which looked like a config file being LFS-tracked. It wasn't — confirmed via empty
  `git lfs ls-files`, `check-attr filter: unspecified`, and a 0-byte plain blob.
- **Git doesn't track empty folders** — the Content namespace subfolders needed `.gitkeep` placeholders to survive the
  commit. UE ignores non-asset files, so they don't clutter the Content Browser.

## Would do differently

- **Create one Content asset earlier** to verify LFS end-to-end before the first commit — would have skipped the
  "empty `ls-files`, did it break?" detour.
- **Know upfront that Supported Platforms → `.uproject`, not `Config/`** — avoids the empty-`git diff Config/` confusion.

## Links (code paths, docs, related notes)

- Plan: [features/P0.1-clean-project-source-control.md](../features/P0.1-clean-project-source-control.md) · primer:
  [features/P0.1-primer.md](../features/P0.1-primer.md)
- `UnifiedGameProject/.gitignore`, `UnifiedGameProject/.gitattributes`
- `UnifiedGameProject/Content/UnifiedGameProject/NAMING-CONVENTIONS.md`
- Source: `Source/UnifiedGameProject.Target.cs`, `Source/UnifiedGameProjectEditor.Target.cs`,
  `Source/UnifiedGameProject/UnifiedGameProject.Build.cs`, `…/UnifiedGameProject.{h,cpp}`
- Repo docs: [ASSET-PIPELINE.md](../ASSET-PIPELINE.md), [CONVENTIONS.md](../CONVENTIONS.md), [ROADMAP.md](../ROADMAP.md)
- Next: **P0.2** — port the EOS test into a reusable plugin (the seam documented above).

## Quiz results (optional)
<!-- appended by quiz mode -->
