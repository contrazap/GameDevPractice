# reference/eos/ — EOS multiplayer snapshot (archival)

Snapshot taken 2026-07-11 from the EOS test project
(`C:\MyFiles\Projects\UE5Projects\MultiplayerTestEOS`) so that **P0.2 and later features are
executable from this repo alone**. The original project is provenance, not a dependency.

## Read this first — what's current vs superseded

- **Engine facts are trustworthy:** the verified-facts table in
  [EOS_MULTIPLAYER_SETUP.md](EOS_MULTIPLAYER_SETUP.md) §1 was checked line-by-line against the
  installed UE 5.8 source on 2026-07-07 (login types, bucket/lobby mechanics, `ClientEncryptionKey`
  rename, NetDriverEOS, UPL intent-filter injection). Re-verify only if the engine version changes.
- **Structure is superseded:** the guide builds EOS code *inside the game module* of a third-person
  template project (`MultiplayerCourse` / `GTFOClone` names, hardcoded maps and bucket). The current
  approach — a headless `OnlineCore` plugin with a clean API — lives in
  [features/P0.2-eos-plugin-port.md](../../features/P0.2-eos-plugin-port.md). When this snapshot and
  the P0.2 plan disagree on *where code goes or how UI is built*, the plan wins.
- **UI chapter (10) is a debug harness, not the pattern:** the Slate menu existed to get
  zero-content buttons on Android quickly. Production UI is UMG over a C++ widget base
  (CONVENTIONS.md working rule 6).
- **Portal values are superseded:** the guide references the GTFOClone portal product; the current
  product is **UnifiedGameProject** (same Epic organization, so member invites carry over).

## Contents

| File | What it is |
|---|---|
| `EOS_MULTIPLAYER_SETUP.md` | Single-file reference: decision rationale, **§1 verified 5.8 facts table**, portal walkthrough, config, code, test matrix, troubleshooting. |
| `EOS_Setup_Guide/00–10, 99` | Step-by-step chapters: portal (01), project config (02), session subsystem (03), UI/controller (04), replication cube (05), local/packaged/Android tests (06–08), voice (09), menu + voice HUD (10), troubleshooting (99). |
| `SECRETS_SETUP.md` | The secrets pattern: committed `DefaultEngine.ini` vs gitignored platform inis vs `*.template` files. Carried into UnifiedGameProject as-is. |
| `src-snapshot/` | The exact files P0.2 ports/adapts, frozen: `EOSSessionSubsystem.h/.cpp` (incl. the 5.8 voice receive-bug workaround), `MultiplayerCoursePlayerController.h/.cpp` (exec commands + Slate debug menu), `MultiplayerCourse.Build.cs`, `DefaultEngine.ini` (EOS blocks), both `*.ini.template` placeholder files. |
